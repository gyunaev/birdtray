#ifndef AUTO_UPDATER_H
#define AUTO_UPDATER_H

#include <QNetworkAccessManager>
#include <QRegularExpression>
#include <QtWidgets/QProgressDialog>
#include <QUrl>
#include <QFile>
#include "updatedialog.h"
#include "updatedownloaddialog.h"


/**
 * Allows to check, download (and, if the platform is supported, install) new updates for Birdtray.
 */
class AutoUpdater : public QObject {
Q_OBJECT
public:
    explicit AutoUpdater(QObject* parent = nullptr);
    
    ~AutoUpdater() override;
    
    /**
     * Check if there is a newer version of Birdtray available.
     * Once the check finished, onCheckUpdateFinished() will be signaled.
     */
    void checkForUpdates();

Q_SIGNALS:
    
    /**
     * Indicates that an update check finished.
     *
     * @param hasUpdate true if a new update was found.
     * @param errorString An error String of the request failed, else a null string.
     */
    void onCheckUpdateFinished(bool hasUpdate, const QString &errorString);

public slots:
    
    void onRequestFinished(QNetworkReply* result);
    
    void onDownloadProgress(QNetworkReply* result, qint64 bytesReceived, qint64 bytesTotal);
    
    /**
     * Start the download of the installer.
     * On supported systems, this will download the installer file and execute it.
     * On all other systems, the download url will be opened in the standard browser.
     */
    void startDownload();

private:
    /**
     * The network manager used to make network requests.
     */
    QNetworkAccessManager* networkAccessManager;
    
    /**
     * Parse a release tag and try to extract the major, minor and patch version from it.
     *
     * @param version A place to store the parsed version.
     * @param releaseTag The release tag to parse.
     * @return true, if a valid version has been parsed from the tag.
     */
    bool parseReleaseTag(int (&version)[3], const QString &releaseTag);
    
    /**
     * Parse the download url from the json assets array
     * received from the Github api for the latest release.
     *
     * @param assets The assets list.
     * @param defaultUrl A default download url to use when
     *                   no asset for the current platform is found.
     * @return The estimated download size, 0 if the size is unknown,
     *         or -1 of no url to an actual installer file could be found.
     */
    qulonglong parseDownloadUrl(const QJsonArray &assets, const QString &defaultUrl);
    
    /**
     * Handle the result from a release info request.
     *
     * @param result The network result from the release info request.
     */
    void onReleaseInfoRequestFinished(QNetworkReply* result);
    
    /**
     * Handle the completion of the download of the Birdtray installer.
     *
     * @param result The network result from the download.
     */
    void onInstallerDownloadFinished(QNetworkReply* result);
    
    /**
     * Resolve a redirect url while downloading the installer.
     * Blocks redirect loops.
     *
     * @param redirectUrl The new redirect url.
     * @return The resolved url to redirect to or an invalid url, if the redirect is not valid.
     */
    QUrl resolveRedirectDownloadUrl(const QUrl &redirectUrl) const;
    
    /**
     * Check if the first version is greater than the second version
     * according to semantic versioning.
     *
     * @param version The first version.
     * @param other The second version to compare with the first version.
     * @return true, if the first version is greater than the second version.
     */
    static bool versionGrater(const int (&version)[3], const int (&other)[3]);
    
    /**
     * The download url to the Birdtray installer or a download site.
     */
    QUrl downloadUrl;
    
    /**
     * Indicates whether or not the current ::downloadUrl
     * points to the actual installer file or just a download page.
     */
    bool haveActualInstallerDownloadUrl = false;
    
    /**
     * The destination to save the downloaded installer to.
     */
    QFile installerFile;
    
    /**
     * A regular expression to parse a version from a release tag.
     */
    QRegularExpression versionRe;
    
    /**
     * A dialog that notifies the user about a new Birdtray version.
     */
    UpdateDialog updateDialog;
    
    /**
     * A dialog that lets the user see the process during download of the installer.
     */
    UpdateDownloadDialog* downloadProcessDialog = nullptr;
};

#endif /* AUTO_UPDATER_H */
