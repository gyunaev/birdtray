#include <QBuffer>
#include <QDir>
#include <QtCore/QStandardPaths>
#ifdef Q_OS_WIN
#include <QCoreApplication>
#endif

#include "settings.h"
#include "utils.h"

#ifdef Q_OS_WIN
#  define THUNDERBIRD_EXE_PATH "\"%ProgramFiles(x86)%\\Mozilla Thunderbird\\thunderbird.exe\""
#else
#  define THUNDERBIRD_EXE_PATH "/usr/bin/thunderbird"
#endif

#define BORDER_COLOR_KEY "common/bordercolor"
#define BORDER_WIDTH_KEY "common/borderwidth"
#define UPDATE_ON_STARTUP_KEY "advanced/updateOnStartup"
#define READ_INSTALL_CONFIG_KEY "hasReadInstallConfig"

Settings * pSettings;

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
    mThunderbirdCmdLine = THUNDERBIRD_EXE_PATH;
    mThunderbirdWindowMatch = "- Mozilla Thunderbird";
    mNotificationMinimumFontSize = 4;
    mNotificationMaximumFontSize = 512;
    mUseMorkParser = true;
    mWatchFileTimeout = 150;
    mBlinkingUseAlphaTransition = false;
    mUpdateOnStartup = false;
    mUnreadOpacityLevel = 0.75;
    mNewEmailMenuEnabled = false;
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
    mSettings->setValue("common/profilepath", mThunderbirdFolderPath );
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
    mSettings->setValue("advanced/unreadmorkparser", mUseMorkParser );
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

    for ( QString uri : mFolderNotificationList )
    {
        QString entry = "accounts/account" + QString::number( index );
        mSettings->setValue( entry + "Color", mFolderNotificationColors[uri].name() );
        mSettings->setValue( entry + "URI", uri );

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
    mThunderbirdFolderPath = mSettings->value(
            "common/profilepath", mThunderbirdFolderPath ).toString();
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

    mThunderbirdCmdLine = mSettings->value("advanced/tbcmdline", mThunderbirdCmdLine).toString();
    mThunderbirdWindowMatch = mSettings->value(
            "advanced/tbwindowmatch", mThunderbirdWindowMatch ).toString();
    mNotificationMinimumFontSize = mSettings->value(
            "advanced/notificationfontminsize", mNotificationMinimumFontSize ).toInt();
    mNotificationMaximumFontSize = mSettings->value(
            "advanced/notificationfontmaxsize", mNotificationMaximumFontSize ).toInt();
    mUseMorkParser = mSettings->value("advanced/unreadmorkparser", mUseMorkParser ).toBool();
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
}

QString Settings::getThunderbirdExecutablePath()
{
    QString path = mThunderbirdCmdLine;

    if (path.startsWith('"')) {
        path = path.section('"', 1, 1);
    }

    path = Utils::expandPath(path);
#if defined (Q_OS_WIN)
    return '"' + QFileInfo(path).absoluteFilePath() + '"';
#else
    return QFileInfo(path).absoluteFilePath();
#endif
}

const QPixmap &Settings::getNotificationIcon() {
    if (mNotificationIcon.isNull()) {
        if (!mNotificationIcon.load(":res/thunderbird.png")) {
            Utils::fatal(QCoreApplication::tr("Cannot load default system tray icon"));
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
