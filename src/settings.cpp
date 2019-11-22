#include <QBuffer>
#include <QDir>
#include <QtCore/QStandardPaths>
#include <QCoreApplication>
#include <QtWidgets/QMessageBox>
#include <QtCore/QUrl>
#include <QDirIterator>

#include "settings.h"
#include "utils.h"
#include "version.h"

#define BORDER_COLOR_KEY "common/bordercolor"
#define BORDER_WIDTH_KEY "common/borderwidth"
#define UPDATE_ON_STARTUP_KEY "advanced/updateOnStartup"
#define READ_INSTALL_CONFIG_KEY "hasReadInstallConfig"

Settings::Settings(bool verboseOutput)
{
#ifdef Q_OS_WIN
    QFileInfo applicationFilePath(qApp->applicationFilePath());
    QFileInfo fileInfo(QDir(qApp->applicationDirPath()), QString("%1.ini").arg(applicationFilePath.baseName()));

    if (QFileInfo::exists(fileInfo.absoluteFilePath()))
        // Portable
        mSettings = new QSettings(fileInfo.absoluteFilePath(), QSettings::IniFormat);
    else
        // Registry
        mSettings = new QSettings;
#else
    mSettings = new QSettings;
#endif

    mVerboseOutput = verboseOutput;
    mIconSize = QSize( 128, 128 );
    mNotificationDefaultColor = QColor("#0000FF");
    mNotificationBorderColor = QColor("#FFFFFF");
    mNotificationBorderWidth = 15;
    mBlinkSpeed = 0;
    mShowHideThunderbird = false;
    mLaunchThunderbird = false;
    mHideWhenMinimized = false;
    mExitThunderbirdWhenQuit = false;
    mNotificationFontWeight = 50;
    mMonitorThunderbirdWindow = false;
    mRestartThunderbird = false;
    mHideWhenStarted = false;
    mHideWhenRestarted = false;
    mAllowSuppressingUnreads = false;
    mLaunchThunderbirdDelay = 0;
    mShowUnreadEmailCount = true;
    mThunderbirdWindowMatch = "- Mozilla Thunderbird";
    mNotificationMinimumFontSize = 4;
    mNotificationMaximumFontSize = 512;
    mWatchFileTimeout = 150;
    mBlinkingUseAlphaTransition = false;
    mUpdateOnStartup = false;
    mUnreadOpacityLevel = 0.75;
    mNewEmailMenuEnabled = false;
    mThunderbirdCmdLine = Utils::getDefaultThunderbirdCommand();
}

Settings::~Settings()
{
    delete(mSettings);
}

void Settings::save()
{
    mSettings->setValue("common/notificationfont", mNotificationFont.toString() );
    mSettings->setValue("common/defaultcolor", mNotificationDefaultColor.name() );
    mSettings->setValue(BORDER_COLOR_KEY, mNotificationBorderColor.name());
    mSettings->setValue(BORDER_WIDTH_KEY, mNotificationBorderWidth);
    mSettings->setValue("common/blinkspeed", mBlinkSpeed );
    mSettings->setValue("common/showhidethunderbird", mShowHideThunderbird );
    mSettings->setValue("common/launchthunderbird", mLaunchThunderbird );
    mSettings->setValue("common/hidewhenminimized", mHideWhenMinimized );
    mSettings->setValue("common/exitthunderbirdonquit", mExitThunderbirdWhenQuit );
    mSettings->setValue("common/restartthunderbird", mRestartThunderbird );
    mSettings->setValue("common/notificationfontweight", mNotificationFontWeight );
    mSettings->setValue("common/monitorthunderbirdwindow", mMonitorThunderbirdWindow );
    mSettings->setValue("common/hidewhenstarted", mHideWhenStarted );
    mSettings->setValue("common/hidewhenrestarted", mHideWhenRestarted );
    mSettings->setValue("common/allowsuppressingunread", mAllowSuppressingUnreads );
    mSettings->setValue("common/launchthunderbirddelay", mLaunchThunderbirdDelay );
    mSettings->setValue("common/showunreademailcount", mShowUnreadEmailCount );

    mSettings->setValue("advanced/tbcmdline", mThunderbirdCmdLine );
    mSettings->setValue("advanced/tbwindowmatch", mThunderbirdWindowMatch );
    mSettings->setValue("advanced/notificationfontminsize", mNotificationMinimumFontSize );
    mSettings->setValue("advanced/notificationfontmaxsize", mNotificationMaximumFontSize );
    mSettings->setValue("advanced/watchfiletimeout", mWatchFileTimeout );
    mSettings->setValue("advanced/blinkingusealpha", mBlinkingUseAlphaTransition );
    mSettings->setValue("advanced/unreadopacitylevel", mUnreadOpacityLevel );
    mSettings->setValue(UPDATE_ON_STARTUP_KEY, mUpdateOnStartup );
    mSettings->setValue("advanced/ignoreUpdateVersion", mIgnoreUpdateVersion );

    // Convert the map into settings
    mSettings->setValue("accounts/count", mFolderNotificationColors.size() );
    int index = 0;
    
    for (const QString &path : mFolderNotificationList) {
        QString entry = "accounts/account" + QString::number(index);
        mSettings->setValue(entry + "Color", mFolderNotificationColors[path].name());
        mSettings->setValue(entry + "URI", path);
        index++;
    }

    // Convert new email data into settings
    mSettings->setValue("newemail/enabled", mNewEmailMenuEnabled );
    mSettings->setValue("newemail/count", mNewEmailData.size() );

    for ( index = 0; index < mNewEmailData.size(); index++ )
    {
        QString entry = "newemail/id" + QString::number( index );
        mSettings->setValue( entry, mNewEmailData[index].toByteArray() );
    }

    if ( !mNotificationIconUnread.isNull() )
        savePixmap( "common/notificationiconunread", mNotificationIconUnread );
    else
        mSettings->remove( "common/notificationiconunread" );

    savePixmap( "common/notificationicon", mNotificationIcon );

    mSettings->sync();
}

void Settings::load()
{
    if ( mSettings->contains( "common/notificationfont" ) )
        mNotificationFont.fromString( mSettings->value( "common/notificationfont", "" ).toString() );

    mNotificationIcon = loadPixmap( "common/notificationicon" );
    mNotificationIconUnread = loadPixmap( "common/notificationiconunread" );

    (void) getNotificationIcon(); // Load the default

    mNotificationDefaultColor = QColor( mSettings->value(
            "common/defaultcolor", mNotificationDefaultColor.name() ).toString() );
    mNotificationBorderColor = QColor(mSettings->value(
            BORDER_COLOR_KEY, mNotificationBorderColor.name()).toString());
    mNotificationBorderWidth = mSettings->value( // Disable border on existing installations
            BORDER_WIDTH_KEY, mSettings->value("common/defaultcolor").isNull() ?
                              0 : mNotificationBorderWidth).toUInt();
    mBlinkSpeed = mSettings->value("common/blinkspeed", mBlinkSpeed ).toInt();
    mShowHideThunderbird = mSettings->value(
            "common/showhidethunderbird", mShowHideThunderbird ).toBool();
    mLaunchThunderbird = mSettings->value("common/launchthunderbird", mLaunchThunderbird ).toBool();
    mHideWhenMinimized = mSettings->value("common/hidewhenminimized", mHideWhenMinimized ).toBool();
    mExitThunderbirdWhenQuit = mSettings->value(
            "common/exitthunderbirdonquit", mExitThunderbirdWhenQuit ).toBool();
    mNotificationFontWeight = qMin(99, mSettings->value(
            "common/notificationfontweight", mNotificationFontWeight ).toInt());
    mMonitorThunderbirdWindow = mSettings->value(
            "common/monitorthunderbirdwindow", mMonitorThunderbirdWindow ).toBool();
    mRestartThunderbird = mSettings->value(
            "common/restartthunderbird", mRestartThunderbird ).toBool();
    mHideWhenStarted = mSettings->value("common/hidewhenstarted", mHideWhenStarted ).toBool();
    mHideWhenRestarted = mSettings->value("common/hidewhenrestarted", mHideWhenRestarted ).toBool();
    mAllowSuppressingUnreads = mSettings->value(
            "common/allowsuppressingunread", mAllowSuppressingUnreads ).toBool();
    mLaunchThunderbirdDelay = mSettings->value(
            "common/launchthunderbirddelay", mLaunchThunderbirdDelay ).toInt();
    mShowUnreadEmailCount = mSettings->value(
            "common/showunreademailcount", mShowUnreadEmailCount ).toBool();

    QStringList thunderbirdCommand = mSettings->value(
            "advanced/tbcmdline", mThunderbirdCmdLine).toStringList();
    if (!thunderbirdCommand.isEmpty() && !thunderbirdCommand[0].isEmpty()) {
        mThunderbirdCmdLine = thunderbirdCommand;
    }
    mThunderbirdWindowMatch = mSettings->value(
            "advanced/tbwindowmatch", mThunderbirdWindowMatch ).toString();
    mNotificationMinimumFontSize = mSettings->value(
            "advanced/notificationfontminsize", mNotificationMinimumFontSize ).toInt();
    mNotificationMaximumFontSize = mSettings->value(
            "advanced/notificationfontmaxsize", mNotificationMaximumFontSize ).toInt();
    mWatchFileTimeout = mSettings->value("advanced/watchfiletimeout", mWatchFileTimeout ).toUInt();
    mBlinkingUseAlphaTransition = mSettings->value(
            "advanced/blinkingusealpha", mBlinkingUseAlphaTransition ).toBool();
    mUnreadOpacityLevel = mSettings->value(
            "advanced/unreadopacitylevel", mUnreadOpacityLevel ).toDouble();
    mUpdateOnStartup = mSettings->value(UPDATE_ON_STARTUP_KEY, mUpdateOnStartup ).toBool();
    mIgnoreUpdateVersion = mSettings->value(
            "advanced/ignoreUpdateVersion", mIgnoreUpdateVersion ).toString();

    mFolderNotificationColors.clear();

    // Convert the map into settings
    int total = mSettings->value("accounts/count", 0 ).toInt();

    for ( int index = 0; index < total; index++ )
    {
        QString entry = "accounts/account" + QString::number( index );
        QString key = mSettings->value( entry + "URI", "" ).toString();
        while (key.isEmpty() && index < total) {
            Utils::debug("Removing invalid account %d", index);
            QString lastEntry = "accounts/account" + QString::number( total - 1 );
            if (index != total - 1) {
                key = mSettings->value(lastEntry + "URI", "").toString();
                mSettings->setValue(entry + "URI", key);
                mSettings->setValue(entry + "Color",
                        mSettings->value(lastEntry + "Color", "").toString());
            }
            mSettings->remove(lastEntry + "URI");
            mSettings->remove(lastEntry + "Color");
            mSettings->setValue("accounts/count", --total);
        }
        if (key.isEmpty()) {
            break;
        }
        mFolderNotificationColors[ key ] = QColor( mSettings->value( entry + "Color", "" ).toString() );
        mFolderNotificationList.push_back( key );
    }

    // Load new email data from settings
    mNewEmailMenuEnabled = mSettings->value("newemail/enabled", mNewEmailMenuEnabled ).toBool();

    mNewEmailData.clear();
    total = mSettings->value("newemail/count", 0 ).toInt();

    for ( int index = 0; index < total; index++ )
    {
        QString entry = "newemail/id" + QString::number( index );
        mNewEmailData.push_back( Setting_NewEmail::fromByteArray( mSettings->value( entry, "" ).toByteArray() ) );
    }
    if (!mSettings->value(READ_INSTALL_CONFIG_KEY, false).toBool()) {
        loadInstallerConfiguration();
    }
#if VERSION_MAJOR == 1 && VERSION_MINOR <= 8
    QString profilePath = mSettings->value("common/profilepath").toString();
    if (!profilePath.isNull() && std::any_of(mFolderNotificationColors.keyBegin(),
            mFolderNotificationColors.keyEnd(),
            [](const QString &path) {return !path.endsWith(".msf");})) {
        bool foundMigrationProblem = false;
        QDir profileDir(profilePath);
        for (const QString &path : mFolderNotificationColors.keys()) {
            if (path.endsWith(".msf")) {
                continue;
            }
            QUrl uri(path);
            QString scheme;
            QString account;
            QString folder;
            if (uri.isValid()) {
                scheme = uri.scheme();
                account = uri.host();
                folder = uri.path();
            } else {
                QString decodedPath = QUrl::fromPercentEncoding(path.toUtf8());
                scheme = decodedPath.section('/', 0, 0);
                scheme.chop(1);
                account = decodedPath.section('/', 2, 2);
                int index;
                if ((index = account.indexOf('@')) != -1) {
                    account = account.mid(index + 1);
                }
                folder = decodedPath.section('/', 3);
                if (!folder.isEmpty()) {
                    folder = '/' + folder;
                }
            }
            const QString mailFolder = scheme == "mailbox" ? "Mail" :
                                       scheme[0].toUpper() + scheme.midRef(1) + "Mail";
            account = Utils::decodeIMAPutf7(account);
            QDir accountDir(profileDir.absoluteFilePath(mailFolder) + '/' + account);
            
            QStringList mockFiles;
            if (!folder.isNull() && !folder.isEmpty()) {
                mockFiles << accountDir.absoluteFilePath(
                        Utils::decodeIMAPutf7(folder).mid(1).split('/').join(".sbd/") + ".msf");
            } else {
                QDirIterator it(accountDir.absolutePath(), {"*.msf"},
                        QDir::Files, QDirIterator::Subdirectories);
                if (!it.hasNext()) {
                    foundMigrationProblem = true;
                }
                while (it.hasNext()) {
                    mockFiles << it.next();
                }
            }
            for (const QString &mockFile: mockFiles) {
                if (!QFile::exists(mockFile)) {
                    foundMigrationProblem = true;
                } else if (!mFolderNotificationColors.contains(mockFile)) {
                    mFolderNotificationColors[mockFile] = mFolderNotificationColors[path];
                    mFolderNotificationList.append(mockFile);
                }
            }
            mFolderNotificationColors.remove(path);
            mFolderNotificationList.removeAll(path);
        }
        mSettings->remove("common/profilepath");
        save();
        if (foundMigrationProblem) {
            QMessageBox::warning(nullptr,
                    QCoreApplication::tr("Sqlite based accounts migrated"), QCoreApplication::tr(
                            "You had configured monitoring of one or more mail folders using "
                            "the Sqlite parser. This method has been removed. Your configurations "
                            "has been migrated to the Mork parser, but some configured mail "
                            "folders could not be found."));
        } else {
            QMessageBox::information(nullptr,
                    QCoreApplication::tr("Sqlite based accounts migrated"), QCoreApplication::tr(
                            "You had configured monitoring of one or more mail accounts using "
                            "the Sqlite parser. This method has been removed. Your configurations "
                            "has been migrated to the Mork parser. Please verify that all accounts "
                            "were mapped correctly."));
        }
    }
#else
#  error "This code converts attempts to convert from legacy sqlite based mail account configuration to mork based configuration. A few releases have passed, we should remove thin now."
#endif /* Birdtray version check */
}

QString Settings::getThunderbirdCommand(QStringList &arguments) {
    if (mThunderbirdCmdLine.isEmpty()) {
        return QString();
    }
    arguments = mThunderbirdCmdLine;
    QString executable = Utils::expandPath(arguments.takeFirst());
#if defined (Q_OS_WIN)
    return '"' + QFileInfo(executable).absoluteFilePath() + '"';
#else
    return QFileInfo(executable).absoluteFilePath();
#endif
}

const QPixmap &Settings::getNotificationIcon() {
    if (mNotificationIcon.isNull()) {
        if (!mNotificationIcon.load(":res/thunderbird.png")) {
            Utils::fatal(QCoreApplication::tr("Cannot load default system tray icon."));
        }
    }
    return mNotificationIcon;
}

void Settings::setNotificationIcon(const QPixmap& icon) {
    mNotificationIcon = icon;
}

void Settings::savePixmap(const QString &key, const QPixmap &pixmap)
{
    // Store the notification icon in the icondata buffer
    QByteArray icondata;
    QBuffer buffer(&icondata);
    buffer.open(QIODevice::WriteOnly);
    pixmap.save(&buffer, "PNG");
    buffer.close();

    mSettings->setValue( key, icondata );
}

QPixmap Settings::loadPixmap(const QString &key)
{
    QPixmap pix;

    if ( mSettings->contains( key ) )
    {
        pix = QPixmap( mIconSize );
        pix.loadFromData( mSettings->value( key, "" ).toByteArray(), "PNG" );
        pix.detach();
    }

    return pix;
}

void Settings::loadInstallerConfiguration() {
    QStringList configDirs = QStandardPaths::standardLocations(QStandardPaths::AppConfigLocation);
    for (const QString& configDir : configDirs) {
        QFileInfo installConfigFile(QDir(configDir), ".installConfig.ini");
        if (installConfigFile.exists()) {
            QSettings installConfig(installConfigFile.absoluteFilePath(), QSettings::IniFormat);
            QVariant value;
            if (!(value = installConfig.value("updateOnStartup")).isNull()) {
                mUpdateOnStartup = value.toBool();
                mSettings->setValue(UPDATE_ON_STARTUP_KEY, mUpdateOnStartup);
            }
        }
    }
    mSettings->setValue(READ_INSTALL_CONFIG_KEY, true);
    mSettings->sync();
}
