#include "autoupdater.h"
#include "utils.h"
#include "birdtrayapp.h"

#if defined(_WIN32) && !defined(_WIN64)
#  include <windows.h>
#endif

#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <version.h>
#include <QtWidgets/QMessageBox>
#include <QDir>
#include <QApplication>
#include <QtCore/QProcess>
#include <QtGui/QDesktopServices>

#define LATEST_VERSION_INFO_URL "https://api.github.com/repos/gyunaev/birdtray/releases/latest"
#define GENERIC_DOWNLOAD_URL "https://github.com/gyunaev/birdtray/releases/latest"
#define VERSION_TAG_REGEX "^(RELEASE_)?(?<major>\\d+)\\.(?<minor>\\d+)(\\.(?<patch>\\d+))?$"

AutoUpdater::AutoUpdater(QObject* parent) :
        networkAccessManager(new QNetworkAccessManager(this)),
        installerFile(QDir::temp().filePath("birdtrayInstaller.exe")),
        versionRe(VERSION_TAG_REGEX, QRegularExpression::CaseInsensitiveOption), QObject(parent) {
    connect(networkAccessManager, &QNetworkAccessManager::finished,
            this, &AutoUpdater::onRequestFinished);
    connect(&updateDialog, &UpdateDialog::onDownloadButtonClicked,
            this, &AutoUpdater::startDownload);
}

AutoUpdater::~AutoUpdater() {
    networkAccessManager->deleteLater();
    updateDialog.deleteLater();
    if (downloadProcessDialog != nullptr) {
        downloadProcessDialog->deleteLater();
        downloadProcessDialog = nullptr;
    }
    if (installerFile.isOpen()) {
        // We quit in the middle of downloading
        installerFile.remove();
    }
}

void AutoUpdater::checkForUpdates() {
    QNetworkReply* result = networkAccessManager->get(
            QNetworkRequest(QUrl(LATEST_VERSION_INFO_URL)));
    if (result != nullptr && result->sslConfiguration().isNull()) {
        emit onCheckUpdateFinished(tr("No ssl configuration!\nOpenSSL might not be installed."));
    }
}

void AutoUpdater::onRequestFinished(QNetworkReply* result) {
    bool isVersionInfoResult = result->url() == QUrl(LATEST_VERSION_INFO_URL);
    if (result->error()) {
        if (isVersionInfoResult) {
            emit onCheckUpdateFinished(result->errorString());
        } else {
            QString errorMessage;
            if (installerFile.error() != QFileDevice::NoError) {
                errorMessage = tr("Failed to save the Birdtray installer:\n")
                               + installerFile.errorString();
            } else {
                errorMessage = tr("Failed to download the Birdtray installer:\n")
                               + result->errorString();
            }
            installerFile.remove();
            bool wasCanceled = false;
            if (downloadProcessDialog != nullptr) {
                wasCanceled = downloadProcessDialog->wasCanceled();
                downloadProcessDialog->close();
                downloadProcessDialog->deleteLater();
                downloadProcessDialog = nullptr;
            }
            if (!wasCanceled && QMessageBox::critical(
                    nullptr, tr("Installer download failed"), errorMessage,
                    QMessageBox::StandardButton::Retry | QMessageBox::StandardButton::Cancel)
                                == QMessageBox::StandardButton::Retry) {
                startDownload();
            }
        }
    } else if (isVersionInfoResult) {
        onReleaseInfoRequestFinished(result);
    } else {
        onInstallerDownloadFinished(result);
    }
    result->deleteLater();
}

bool AutoUpdater::parseReleaseTag(int (&version)[3], const QString &releaseTag) {
    const QRegularExpressionMatch &match = versionRe.match(releaseTag);
    if (!match.hasMatch()) {
        return false;
    }
    const QString versionParts[] = {"major", "minor", "patch"};
    for (int i = 0; i < 3; i++) {
        QString partStr = match.captured(versionParts[i]);
        int versionPart;
        if (partStr.isNull()) {
            if (i != 2) {
                return false;
            }
            versionPart = 0; // Allow missing patch version
        } else {
            bool parseOk = true;
            versionPart = partStr.toInt(&parseOk);
            if (!parseOk) {
                return false;
            }
        }
        version[i] = versionPart;
    }
    return true;
}

void AutoUpdater::startDownload() {
    if (!downloadUrl.isValid()) {
        return;
    }
    if (haveActualInstallerDownloadUrl) {
        installerFile.unsetError();
        if (installerFile.isOpen()) {
            installerFile.reset();
        } else {
            while (!installerFile.open(QFile::WriteOnly)) {
                if (QMessageBox::critical(
                        nullptr, tr("Installer download failed"),
                        tr("Failed to save the Birdtray installer:\n")
                        + installerFile.errorString(),
                        QMessageBox::StandardButton::Retry | QMessageBox::StandardButton::Cancel)
                    != QMessageBox::StandardButton::Retry) {
                    return;
                }
            }
        }
        if (downloadProcessDialog == nullptr) {
            downloadProcessDialog = new UpdateDownloadDialog();
        } else {
            downloadProcessDialog->reset();
        }
        downloadProcessDialog->show();
        QNetworkReply* reply = networkAccessManager->get(QNetworkRequest(downloadUrl));
        connect(reply, &QNetworkReply::downloadProgress, this,
                [=](qint64 bytesReceived, qint64 bytesTotal) {
                    AutoUpdater::onDownloadProgress(reply, bytesReceived, bytesTotal);
                });
        return;
    }
    QDesktopServices::openUrl(downloadUrl);
}

qulonglong AutoUpdater::parseDownloadUrl(const QJsonArray &assets, const QString &defaultUrl) {
#ifdef Q_OS_WIN
#ifdef _WIN64
    const QString fileEnding = "x64.exe";
#elif defined(_WIN32)
    BOOL is64Bit = FALSE;
    is64Bit = IsWow64Process(GetCurrentProcess(), &is64Bit) && is64Bit;
    const QString fileEnding = is64Bit ? "x64.exe" : "x86.exe";
#endif
    for (auto assetInfo : assets) {
        QString name = assetInfo["name"].toString();
        if (!name.isNull() && name.endsWith(fileEnding)) {
            downloadUrl = assetInfo["browser_download_url"].toString();
            if (downloadUrl.isValid()) {
                return assetInfo["size"].toVariant().toULongLong();
            }
        }
    }
#endif /* Q_OS_WIN */
    if (defaultUrl.isNull()) {
        downloadUrl = GENERIC_DOWNLOAD_URL;  // Last resort
    } else {
        downloadUrl = defaultUrl;
    }
    return (qulonglong) -1;
}

void AutoUpdater::onReleaseInfoRequestFinished(QNetworkReply* result) {
    haveActualInstallerDownloadUrl = false;
    QByteArray buffer = result->readAll();
    QJsonDocument responseDocument = QJsonDocument::fromJson(buffer);
    QJsonObject response = responseDocument.object();
    QString releaseTag = response["tag_name"].toString();
    int version[3];
    if (!updateDialog.isVisible() && parseReleaseTag(version, releaseTag) &&
        versionGrater(version, {VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH})) {
        qulonglong downloadSize = parseDownloadUrl(
                response["assets"].toArray(), response["html_url"].toString());
        if (downloadUrl.isValid()) {
            QString versionString = QString("%1.%2.%3")
                    .arg(version[0]).arg(version[1]).arg(version[2]);
            if (versionString != BirdtrayApp::get()->getSettings()->mIgnoreUpdateVersion) {
                haveActualInstallerDownloadUrl = downloadSize != (qulonglong) -1;
                updateDialog.show(versionString, response["body"].toString(), downloadSize);
            }
        }
    }
    emit onCheckUpdateFinished(QString());
}

void AutoUpdater::onInstallerDownloadFinished(QNetworkReply* result) {
    QVariant redirectionTarget = result->attribute(QNetworkRequest::RedirectionTargetAttribute);
    if (!redirectionTarget.isNull()) {
        QUrl newUrl = resolveRedirectDownloadUrl(redirectionTarget.toUrl());
        if (newUrl.isValid()) {
            downloadUrl = newUrl;
            startDownload();
            return;
        } else {
            QMessageBox::critical(
                    nullptr, tr("Installer download failed"),
                    tr("Failed to download the Birdtray installer:\n") + tr("Invalid redirect: ")
                    + redirectionTarget.toString(), QMessageBox::StandardButton::Abort);
        }
    } else if (installerFile.write(result->readAll()) == -1) {
        QString errorMessage(tr("Failed to save the Birdtray installer:\n")
                             + installerFile.errorString());
        installerFile.remove();
        if (QMessageBox::critical(
                nullptr, tr("Installer download failed"), errorMessage,
                QMessageBox::StandardButton::Retry | QMessageBox::StandardButton::Cancel)
            == QMessageBox::StandardButton::Retry) {
            startDownload();
            return;
        }
    } else {
        installerFile.close();
        if (downloadProcessDialog != nullptr && !downloadProcessDialog->wasCanceled()) {
            downloadProcessDialog->onDownloadComplete();
            if (downloadProcessDialog->exec() == QDialog::Rejected
                || downloadProcessDialog->wasCanceled()) {
                installerFile.remove();
            } else if (!QProcess::startDetached(installerFile.fileName())) {
                QMessageBox::critical(
                        nullptr, tr("Update failed"),
                        tr("Failed to start the Birdtray installer."),
                        QMessageBox::StandardButton::Abort);
                installerFile.remove();
            } else {
                QApplication::quit();
            }
        }
    }
    if (downloadProcessDialog != nullptr) {
        downloadProcessDialog->close();
        downloadProcessDialog->deleteLater();
        downloadProcessDialog = nullptr;
    }
}

void AutoUpdater::onDownloadProgress(QNetworkReply* result, qint64 bytesReceived,
                                     qint64 bytesTotal) {
    if (downloadProcessDialog != nullptr) {
        if (downloadProcessDialog->wasCanceled() || installerFile.write(result->readAll()) == -1) {
            result->close();
        } else {
            downloadProcessDialog->onDownloadProgress(bytesReceived, bytesTotal);
        }
    }
}

QUrl AutoUpdater::resolveRedirectDownloadUrl(const QUrl &redirectUrl) const {
    QUrl newUrl;
    if (redirectUrl.isRelative()) {
        newUrl = downloadUrl.resolved(redirectUrl);
    } else {
        newUrl = redirectUrl;
    }
    if (!newUrl.isEmpty() && newUrl != downloadUrl) {
        return newUrl;
    }
    return QUrl();
}

bool AutoUpdater::versionGrater(const int (&version)[3], const int (&other)[3]) {
    return version[0] > other[0] || (version[0] == other[0] && (
            version[1] > other[1] || (version[1] == other[1] && version[2] > other[2])));
}
