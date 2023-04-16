#include <QBuffer>
#include <QDir>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSaveFile>
#include <QtCore/QStandardPaths>
#include <QCoreApplication>
#include <QMessageBox>
#include <QtCore/QUrl>
#include <QtCore/QDirIterator>

#include "settings.h"
#include "utils.h"
#include "log.h"

#define BORDER_COLOR_KEY "common/bordercolor"
#define BORDER_WIDTH_KEY "common/borderwidth"
#define START_CLOSED_THUNDERBIRD_KEY "common/startClosedThunderbird"
#define HIDE_WHEN_STARTED_MANUALLY_KEY "common/hideWhenStartedManually"
#define THUNDERBIRD_PROCESS_NAME_KEY "advanced/tbprocessname"
#ifdef Q_OS_WIN
#  define THUNDERBIRD_PROCESS_NAME_DEFAULT "thunderbird.exe"
#else
#  define THUNDERBIRD_PROCESS_NAME_DEFAULT "thunderbird"
#endif /* Q_OS_WIN */
#define UPDATE_ON_STARTUP_KEY "advanced/updateOnStartup"
#define ONLY_SHOW_ICON_ON_UNREAD_MESSAGES_KEY "advanced/onlyShowIconOnUnreadMessages"
#define READ_INSTALL_CONFIG_KEY "hasReadInstallConfig"

Settings::Settings()
{
    // This adds support for portable apps which can carry config.json in the same directory
    QFileInfo applicationFilePath( qApp->applicationFilePath() );
    QFileInfo fileInfo( QDir(qApp->applicationDirPath()), "birdtray-config.json" );

    if ( QFileInfo::exists(fileInfo.absoluteFilePath()) ) // Portable
        mSettingsFilename = fileInfo.absoluteFilePath();
    else
        mSettingsFilename = QStandardPaths::writableLocation( QStandardPaths::ConfigLocation ) + "/birdtray-config.json";

    // Make sure the path exists, and create it if it does not
    QDir apppath( QStandardPaths::writableLocation( QStandardPaths::ConfigLocation ) );

    if ( !apppath.exists() )
        apppath.mkpath( apppath.absolutePath() );

    mIconSize = QSize( ICON_SIZE, ICON_SIZE );
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
    startClosedThunderbird = false;
    hideWhenStartedManually = false;
    mAllowSuppressingUnreads = false;
    mLaunchThunderbirdDelay = 0;
    mShowUnreadEmailCount = true;
    ignoreUnreadCountOnStart = false;
    ignoreUnreadCountOnShow = false;
    ignoreUnreadCountOnHide = false;
    showDialogIfNoAccountsConfigured = true;
    mThunderbirdWindowMatch = " Mozilla Thunderbird";
    mThunderbirdProcessName = THUNDERBIRD_PROCESS_NAME_DEFAULT;
    mNotificationMinimumFontSize = 4;
    mNotificationMaximumFontSize = 512;
    mWatchFileTimeout = 150;
    mBlinkingUseAlphaTransition = false;
    mUpdateOnStartup = false;
    onlyShowIconOnUnreadMessages = false;
    mUnreadOpacityLevel = 0.75;
    mNewEmailMenuEnabled = false;
    mIndexFilesRereadIntervalSec = 0;
    mThunderbirdCmdLine = Utils::getDefaultThunderbirdCommand();
    mIgnoreNETWMhints = false;
}

Settings::~Settings()
{
}

void Settings::save()
{
    QJsonObject out;

    out[ "common/notificationfont" ] = mNotificationFont.toString();
    out[ "common/defaultcolor" ] = mNotificationDefaultColor.name();
    out[ BORDER_COLOR_KEY ] = mNotificationBorderColor.name();
    out[ BORDER_WIDTH_KEY ] = static_cast<int>( mNotificationBorderWidth );
    out[ "common/blinkspeed" ] = static_cast<int>( mBlinkSpeed );
    out[ "common/showhidethunderbird" ] = mShowHideThunderbird;
    out[ "common/launchthunderbird" ] = mLaunchThunderbird;
    out[ "common/hidewhenminimized" ] = mHideWhenMinimized;
    out[ "common/exitthunderbirdonquit" ] = mExitThunderbirdWhenQuit;
    out[ "common/restartthunderbird" ] = mRestartThunderbird;
    out[ "common/notificationfontweight" ] = static_cast<int>( mNotificationFontWeight );
    out[ "common/monitorthunderbirdwindow" ] = mMonitorThunderbirdWindow;
    out[ "common/hidewhenstarted" ] = mHideWhenStarted;
    out[ "common/hidewhenrestarted" ] = mHideWhenRestarted;
    out[ START_CLOSED_THUNDERBIRD_KEY ] = startClosedThunderbird;
    out[ HIDE_WHEN_STARTED_MANUALLY_KEY ] = hideWhenStartedManually;
    out[ "common/allowsuppressingunread" ] = mAllowSuppressingUnreads;
    out[ "common/launchthunderbirddelay" ] = mLaunchThunderbirdDelay;
    out[ "common/showunreademailcount" ] = mShowUnreadEmailCount;
    out[ "common/ignoreStartUnreadCount" ] = ignoreUnreadCountOnStart;
    out[ "common/ignoreShowUnreadCount" ] = ignoreUnreadCountOnShow;
    out[ "common/forceIgnoreUnreadEmailsOnMinimize" ] = ignoreUnreadCountOnHide;
    out[ "common/showDialogIfNoAccountsConfigured"  ] = showDialogIfNoAccountsConfigured;

    out[ "advanced/tbcmdline" ] = QJsonArray::fromStringList( mThunderbirdCmdLine );
    out[ "advanced/tbwindowmatch" ] = mThunderbirdWindowMatch;
    out[ THUNDERBIRD_PROCESS_NAME_KEY ] = mThunderbirdProcessName;
    out[ "advanced/notificationfontminsize" ] = static_cast<int>( mNotificationMinimumFontSize );
    out[ "advanced/notificationfontmaxsize" ] = static_cast<int>( mNotificationMaximumFontSize );
    out[ "advanced/watchfiletimeout" ] = static_cast<int>( mWatchFileTimeout );
    out[ "advanced/blinkingusealpha" ] = mBlinkingUseAlphaTransition;
    out[ "advanced/unreadopacitylevel" ] = mUnreadOpacityLevel;
    out[ UPDATE_ON_STARTUP_KEY ] = mUpdateOnStartup;
    out[ "advanced/ignoreUpdateVersion" ] = mIgnoreUpdateVersion;
    out[ ONLY_SHOW_ICON_ON_UNREAD_MESSAGES_KEY ] = onlyShowIconOnUnreadMessages;
    out[ "advanced/forcedRereadInterval" ] = static_cast<int>( mIndexFilesRereadIntervalSec );
    out[ "advanced/runProcessOnChange" ] = mProcessRunOnCountChange;
    out[ "advanced/ignoreNetWMhints" ] = mIgnoreNETWMhints;

    // Store the account map
    QJsonArray accounts;

    for ( const QString& path : watchedMorkFiles.orderedKeys() )
    {
        QJsonObject ac;
        ac[ "color" ] = watchedMorkFiles[path].name();
        ac[ "path" ] = path;

        accounts.push_back( ac );
    }

    out[ "accounts" ] = accounts;

    // Store the new email data
    QJsonArray newemaildata;

    out[ "common/newemailEnabled" ] = mNewEmailMenuEnabled;

    for ( const Setting_NewEmail& e :  mNewEmailData )
        newemaildata.push_back( e.toJSON() );

    if ( !newemaildata.isEmpty() )
        out[ "newemails" ] = newemaildata;

    if ( !mNotificationIconUnread.isNull() ) {
        out[ "common/notificationiconunread" ] = Utils::pixmapToString( mNotificationIconUnread );
    }
    out[ "common/notificationicon" ] = Utils::pixmapToString( mNotificationIcon );

    // QSaveFile is an I/O device for writing text and binary files, without
    // losing existing data if the writing operation fails.
    QSaveFile file( mSettingsFilename );

    if ( !file.open(QIODevice::WriteOnly | QIODevice::Truncate) )
    {
        QMessageBox::critical(nullptr, tr("Could not save the settings"),
                tr("Could not save the settings into file %1:\n%2")
                        .arg(file.fileName()).arg(file.errorString()));
        return;
    }

    QJsonDocument document( out );

    // QSaveFile will remember the write error happen, so no need to check
    file.write( document.toJson() );
    file.commit();
}

void Settings::load()
{
    // Load the settings file
    QFile file( mSettingsFilename );

    if ( file.open(QIODevice::ReadOnly ) )
    {
        QByteArray data = file.readAll();

        if ( !data.isEmpty() )
        {
            QJsonDocument document = QJsonDocument::fromJson(data);

            if ( document.isObject() )
            {
                // Parse the settings from JSON
                fromJSON( document.object() );
                return;
            }
        }
    }

#ifdef Q_OS_WIN
    QFileInfo applicationFilePath(qApp->applicationFilePath());
    QFileInfo fileInfo(QDir(qApp->applicationDirPath()), QString("%1.ini").arg(applicationFilePath.baseName()));

    // Portable
    if (QFileInfo::exists(fileInfo.absoluteFilePath()))
        fromQSettings( new QSettings(fileInfo.absoluteFilePath(), QSettings::IniFormat) );
    else
#endif
        fromQSettings( new QSettings() );

    save();
}

void Settings::fromJSON( const QJsonObject& settings )
{
    if ( settings.contains( "common/notificationfont" ) )
        mNotificationFont.fromString( settings.value( "common/notificationfont" ).toString() );

    mNotificationIcon = Utils::pixmapFromString(
            settings.value("common/notificationicon").toString());
    mNotificationIconUnread = Utils::pixmapFromString(
            settings.value("common/notificationiconunread").toString());

    (void) getNotificationIcon(); // Load the default

    mNotificationDefaultColor = QColor( settings.value( "common/defaultcolor" ).toString() );
    mNotificationBorderColor = QColor(settings.value(BORDER_COLOR_KEY).toString() );

    // Disable border on existing installations
    if ( settings.contains( "common/defaultcolor") )
        mNotificationBorderWidth = settings.value( BORDER_WIDTH_KEY ).toInt();

    mBlinkSpeed = settings.value("common/blinkspeed").toInt();
    mShowHideThunderbird = settings.value("common/showhidethunderbird").toBool();
    mLaunchThunderbird = settings.value("common/launchthunderbird").toBool();
    mHideWhenMinimized = settings.value("common/hidewhenminimized").toBool();
    mExitThunderbirdWhenQuit = settings.value("common/exitthunderbirdonquit").toBool();
    mNotificationFontWeight = qMin(99, settings.value("common/notificationfontweight").toInt());
    mMonitorThunderbirdWindow = settings.value("common/monitorthunderbirdwindow").toBool();
    mRestartThunderbird = settings.value("common/restartthunderbird").toBool();
    mHideWhenStarted = settings.value("common/hidewhenstarted").toBool();
    mHideWhenRestarted = settings.value("common/hidewhenrestarted").toBool();
    startClosedThunderbird = settings.value(START_CLOSED_THUNDERBIRD_KEY).toBool();
    hideWhenStartedManually = settings.value(HIDE_WHEN_STARTED_MANUALLY_KEY).toBool();
    mAllowSuppressingUnreads = settings.value("common/allowsuppressingunread").toBool();
    mLaunchThunderbirdDelay = settings.value("common/launchthunderbirddelay").toInt();
    mShowUnreadEmailCount = settings.value("common/showunreademailcount").toBool();
    ignoreUnreadCountOnStart = mAllowSuppressingUnreads &&
            settings.value("common/ignoreStartUnreadCount").toBool();
    ignoreUnreadCountOnShow = mAllowSuppressingUnreads &&
            settings.value("common/ignoreShowUnreadCount").toBool();
    ignoreUnreadCountOnHide = mAllowSuppressingUnreads &&
            settings.value("common/forceIgnoreUnreadEmailsOnMinimize").toBool();
    showDialogIfNoAccountsConfigured = settings.value("common/showDialogIfNoAccountsConfigured").toBool();

    mThunderbirdWindowMatch = settings.value("advanced/tbwindowmatch").toString();
    mThunderbirdProcessName = settings.value(THUNDERBIRD_PROCESS_NAME_KEY).toString(mThunderbirdProcessName);
    mNotificationMinimumFontSize = settings.value("advanced/notificationfontminsize").toInt();
    mNotificationMaximumFontSize = settings.value("advanced/notificationfontmaxsize").toInt();
    mWatchFileTimeout = settings.value("advanced/watchfiletimeout").toInt();
    mBlinkingUseAlphaTransition = settings.value("advanced/blinkingusealpha").toBool();
    mUnreadOpacityLevel = settings.value("advanced/unreadopacitylevel").toDouble();
    mUpdateOnStartup = settings.value(UPDATE_ON_STARTUP_KEY).toBool();
    onlyShowIconOnUnreadMessages = settings.value(ONLY_SHOW_ICON_ON_UNREAD_MESSAGES_KEY).toBool();
    mIgnoreUpdateVersion = settings.value("advanced/ignoreUpdateVersion").toString();
    mIndexFilesRereadIntervalSec = settings.value("advanced/forcedRereadInterval").toInt();
    mProcessRunOnCountChange = settings.value( "advanced/runProcessOnChange" ).toString();
    mIgnoreNETWMhints = settings.value( "advanced/ignoreNetWMhints").toBool();

    QStringList thunderbirdCommand = settings.value("advanced/tbcmdline").toVariant().toStringList();
    if ( !thunderbirdCommand.isEmpty() && !thunderbirdCommand[0].isEmpty() )
        mThunderbirdCmdLine = thunderbirdCommand;

    watchedMorkFiles.clear();

    // Convert the map into settings
    if ( settings["accounts"].isArray() )
    {
        for ( const QJsonValueRef& a : settings["accounts"].toArray() )
        {
            QString path = a.toObject().value("path").toString();
            QString color = a.toObject().value("color").toString();

            if ( path.isEmpty() )
                continue;

            watchedMorkFiles[ path ] = QColor( color );
        }
    }

    // Load new email data from settings
    mNewEmailMenuEnabled = settings.value("common/newemailEnabled").toBool();

    mNewEmailData.clear();

    if ( settings["newemails"].isArray() )
    {
        for ( const QJsonValueRef& a : settings["newemails"].toArray() )
            mNewEmailData.push_back( Setting_NewEmail::fromJSON( a.toObject() ) );
    }

    if ( !settings.contains( READ_INSTALL_CONFIG_KEY ) )
    {
        loadInstallerConfiguration();
        settings[ READ_INSTALL_CONFIG_KEY ] = true;
    }
}

void Settings::fromQSettings( QSettings * psettings )
{
    QSettings & settings = *psettings;

    if ( settings.contains( "common/notificationfont" ) )
        mNotificationFont.fromString( settings.value( "common/notificationfont", "" ).toString() );

    mNotificationIcon = Utils::pixmapFromString(
            settings.value("common/notificationicon", "").toString());
    mNotificationIconUnread = Utils::pixmapFromString(
            settings.value("common/notificationiconunread", "").toString());

    (void) getNotificationIcon(); // Load the default

    mNotificationDefaultColor = QColor( settings.value(
            "common/defaultcolor", mNotificationDefaultColor.name() ).toString() );
    mNotificationBorderColor = QColor(settings.value(
            BORDER_COLOR_KEY, mNotificationBorderColor.name()).toString());
    mNotificationBorderWidth = settings.value( // Disable border on existing installations
            BORDER_WIDTH_KEY, settings.value("common/defaultcolor").isNull() ?
                              0 : mNotificationBorderWidth).toUInt();
    mBlinkSpeed = settings.value("common/blinkspeed", mBlinkSpeed ).toInt();
    mShowHideThunderbird = settings.value(
            "common/showhidethunderbird", mShowHideThunderbird ).toBool();
    mLaunchThunderbird = settings.value("common/launchthunderbird", mLaunchThunderbird ).toBool();
    mHideWhenMinimized = settings.value("common/hidewhenminimized", mHideWhenMinimized ).toBool();
    mExitThunderbirdWhenQuit = settings.value(
            "common/exitthunderbirdonquit", mExitThunderbirdWhenQuit ).toBool();
    mNotificationFontWeight = qMin(99, settings.value(
            "common/notificationfontweight", mNotificationFontWeight ).toInt());
    mMonitorThunderbirdWindow = settings.value(
            "common/monitorthunderbirdwindow", mMonitorThunderbirdWindow ).toBool();
    mRestartThunderbird = settings.value(
            "common/restartthunderbird", mRestartThunderbird ).toBool();
    mHideWhenStarted = settings.value("common/hidewhenstarted", mHideWhenStarted ).toBool();
    mHideWhenRestarted = settings.value("common/hidewhenrestarted", mHideWhenRestarted ).toBool();
    startClosedThunderbird = settings.value(START_CLOSED_THUNDERBIRD_KEY, startClosedThunderbird ).toBool();
    hideWhenStartedManually = settings.value(HIDE_WHEN_STARTED_MANUALLY_KEY, hideWhenStartedManually ).toBool();
    mAllowSuppressingUnreads = settings.value(
            "common/allowsuppressingunread", mAllowSuppressingUnreads ).toBool();
    mLaunchThunderbirdDelay = settings.value(
            "common/launchthunderbirddelay", mLaunchThunderbirdDelay ).toInt();
    mShowUnreadEmailCount = settings.value(
            "common/showunreademailcount", mShowUnreadEmailCount ).toBool();
    ignoreUnreadCountOnStart = mAllowSuppressingUnreads && settings.value(
            "common/ignoreStartUnreadCount", ignoreUnreadCountOnStart).toBool();
    ignoreUnreadCountOnShow = mAllowSuppressingUnreads && settings.value(
            "common/ignoreShowUnreadCount", ignoreUnreadCountOnShow).toBool();
    ignoreUnreadCountOnHide = mAllowSuppressingUnreads && settings.value(
            "common/forceIgnoreUnreadEmailsOnMinimize", ignoreUnreadCountOnHide).toBool();
    showDialogIfNoAccountsConfigured = settings.value(
            "common/showDialogIfNoAccountsConfigured", showDialogIfNoAccountsConfigured).toBool();

    QStringList thunderbirdCommand = settings.value(
            "advanced/tbcmdline", mThunderbirdCmdLine).toStringList();
    if (!thunderbirdCommand.isEmpty() && !thunderbirdCommand[0].isEmpty()) {
        mThunderbirdCmdLine = thunderbirdCommand;
    }
    mThunderbirdWindowMatch = settings.value(
            "advanced/tbwindowmatch", mThunderbirdWindowMatch ).toString();
    mThunderbirdProcessName = settings.value(THUNDERBIRD_PROCESS_NAME_KEY, mThunderbirdProcessName ).toString();
    mNotificationMinimumFontSize = settings.value(
            "advanced/notificationfontminsize", mNotificationMinimumFontSize ).toInt();
    mNotificationMaximumFontSize = settings.value(
            "advanced/notificationfontmaxsize", mNotificationMaximumFontSize ).toInt();
    mWatchFileTimeout = settings.value("advanced/watchfiletimeout", mWatchFileTimeout ).toUInt();
    mBlinkingUseAlphaTransition = settings.value(
            "advanced/blinkingusealpha", mBlinkingUseAlphaTransition ).toBool();
    mUnreadOpacityLevel = settings.value(
            "advanced/unreadopacitylevel", mUnreadOpacityLevel ).toDouble();
    mUpdateOnStartup = settings.value(UPDATE_ON_STARTUP_KEY, mUpdateOnStartup ).toBool();
    onlyShowIconOnUnreadMessages = settings.value(
            ONLY_SHOW_ICON_ON_UNREAD_MESSAGES_KEY, onlyShowIconOnUnreadMessages ).toBool();
    mIgnoreUpdateVersion = settings.value(
            "advanced/ignoreUpdateVersion", mIgnoreUpdateVersion ).toString();
    mIndexFilesRereadIntervalSec = settings.value("advanced/forcedRereadInterval", mIndexFilesRereadIntervalSec ).toUInt();

    watchedMorkFiles.clear();

    // Convert the map into settings
    int total = settings.value("accounts/count", 0 ).toInt();

    for ( int index = 0; index < total; index++ )
    {
        QString entry = "accounts/account" + QString::number( index );
        QString key = settings.value( entry + "URI", "" ).toString();

        watchedMorkFiles[ key ] = QColor( settings.value( entry + "Color", "" ).toString() );
    }

    // Load new email data from settings
    mNewEmailMenuEnabled = settings.value("newemail/enabled", mNewEmailMenuEnabled ).toBool();

    mNewEmailData.clear();
    total = settings.value("newemail/count", 0 ).toInt();

    for ( int index = 0; index < total; index++ )
    {
        QString entry = "newemail/id" + QString::number( index );
        mNewEmailData.push_back( Setting_NewEmail::fromByteArray( settings.value( entry, "" ).toByteArray() ) );
    }

    if (!settings.value(READ_INSTALL_CONFIG_KEY, false).toBool()) {
        loadInstallerConfiguration();
    }
    QString profilePath = psettings->value("common/profilepath").toString();
    delete psettings;
    const QStringList watchedMorkFilePaths = watchedMorkFiles.orderedKeys();
    if (!profilePath.isNull() && std::any_of(watchedMorkFilePaths.begin(),
            watchedMorkFilePaths.end(),
            [](const QString &path) { return !path.endsWith(".msf"); })) {
        bool foundMigrationProblem = false;
        QDir profileDir(profilePath);
        for (const QString &path : watchedMorkFilePaths) {
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
                                       scheme[0].toUpper() + scheme.mid(1) + "Mail";
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
                } else if (!watchedMorkFiles.orderedKeys().contains(mockFile)) {
                    watchedMorkFiles[mockFile] = watchedMorkFiles[path];
                }
            }
            watchedMorkFiles.remove(path);
        }
        if (foundMigrationProblem) {
            QMessageBox::warning(nullptr,
                    tr("Sqlite based accounts migrated"), tr(
                            "You had configured monitoring of one or more mail folders using "
                            "the Sqlite parser. This method has been removed. Your configurations "
                            "has been migrated to the Mork parser, but some configured mail "
                            "folders could not be found."));
        } else {
            QMessageBox::information(nullptr,
                    tr("Sqlite based accounts migrated"), tr(
                            "You had configured monitoring of one or more mail accounts using "
                            "the Sqlite parser. This method has been removed. Your configurations "
                            "has been migrated to the Mork parser. Please verify that all accounts "
                            "were mapped correctly."));
        }
    }
}

bool Settings::getStartThunderbirdCmdline( QString& executable, QStringList &arguments )
{
    if ( mThunderbirdCmdLine.isEmpty() )
        return false;

    arguments = mThunderbirdCmdLine;
    executable = QFileInfo( Utils::expandPath( arguments.takeFirst() ) ).absoluteFilePath();

#if defined (Q_OS_WIN)
    executable = '"' + executable + '"';
#endif

    return true;
}

const QPixmap &Settings::getNotificationIcon() {
    if (mNotificationIcon.isNull()) {
        if (!mNotificationIcon.load(":res/thunderbird.png")) {
            Log::fatal( tr("Cannot load default system tray icon.") );
        }
    }
    return mNotificationIcon;
}

void Settings::setNotificationIcon(const QPixmap& icon) {
    mNotificationIcon = icon;
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
            }
        }
    }
}
