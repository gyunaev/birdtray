#include <QStandardPaths>
#include <QBuffer>
#include <QDir>

#include "settings.h"
#include "utils.h"

#ifdef Q_OS_WIN
#  define THUNDERBIRD_EXE_PATH "\"%ProgramFiles(x86)%\\Mozilla Thunderbird\\thunderbird.exe\""
#else
#  define THUNDERBIRD_EXE_PATH "/usr/bin/thunderbird"
#endif

Settings * pSettings;

Settings::Settings(bool verboseOutput)
{
    mVerboseOutput = verboseOutput;
    mIconSize = QSize( 128, 128 );
    mNotificationDefaultColor = QColor("#00FF00");
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
    mUnreadOpacityLevel = 0.75;
    mNewEmailMenuEnabled = false;
}

void Settings::save()
{
    QSettings settings;

    settings.setValue("common/notificationfont", mNotificationFont.toString() );
    settings.setValue("common/defaultcolor", mNotificationDefaultColor.name() );
    settings.setValue("common/profilepath", mThunderbirdFolderPath );
    settings.setValue("common/blinkspeed", mBlinkSpeed );
    settings.setValue("common/showhidethunderbird", mShowHideThunderbird );
    settings.setValue("common/launchthunderbird", mLaunchThunderbird );
    settings.setValue("common/hidewhenminimized", mHideWhenMinimized );
    settings.setValue("common/exitthunderbirdonquit", mExitThunderbirdWhenQuit );
    settings.setValue("common/restartthunderbird", mRestartThunderbird );
    settings.setValue("common/notificationfontweight", mNotificationFontWeight );
    settings.setValue("common/monitorthunderbirdwindow", mMonitorThunderbirdWindow );
    settings.setValue("common/hidewhenstarted", mHideWhenStarted );
    settings.setValue("common/hidewhenrestarted", mHideWhenRestarted );
    settings.setValue("common/allowsuppressingunread", mAllowSuppressingUnreads );
    settings.setValue("common/launchthunderbirddelay", mLaunchThunderbirdDelay );
    settings.setValue("common/showunreademailcount", mShowUnreadEmailCount );

    settings.setValue("advanced/tbcmdline", mThunderbirdCmdLine );
    settings.setValue("advanced/tbwindowmatch", mThunderbirdWindowMatch );
    settings.setValue("advanced/unreadmorkparser", mUseMorkParser );
    settings.setValue("advanced/notificationfontminsize", mNotificationMinimumFontSize );
    settings.setValue("advanced/notificationfontmaxsize", mNotificationMaximumFontSize );
    settings.setValue("advanced/watchfiletimeout", mWatchFileTimeout );
    settings.setValue("advanced/blinkingusealpha", mBlinkingUseAlphaTransition );
    settings.setValue("advanced/unreadopacitylevel", mUnreadOpacityLevel );

    // Convert the map into settings
    settings.setValue("accounts/count", mFolderNotificationColors.size() );
    int index = 0;

    for ( QString uri : mFolderNotificationList )
    {
        QString entry = "accounts/account" + QString::number( index );
        settings.setValue( entry + "Color", mFolderNotificationColors[uri].name() );
        settings.setValue( entry + "URI", uri );

        index++;
    }

    // Convert new email data into settings
    settings.setValue("newemail/enabled", mNewEmailMenuEnabled );
    settings.setValue("newemail/count", mNewEmailData.size() );

    for ( index = 0; index < mNewEmailData.size(); index++ )
    {
        QString entry = "newemail/id" + QString::number( index );
        settings.setValue( entry, mNewEmailData[index].toByteArray() );
    }

    if ( !mNotificationIconUnread.isNull() )
        savePixmap( settings, "common/notificationiconunread", mNotificationIconUnread );
    else
        settings.remove( "common/notificationiconunread" );

    savePixmap( settings, "common/notificationicon", mNotificationIcon );
}

void Settings::load()
{
    QSettings settings;

    if ( settings.contains( "common/notificationfont" ) )
        mNotificationFont.fromString( settings.value( "common/notificationfont", "" ).toString() );

    mNotificationIcon = loadPixmap( settings, "common/notificationicon" );
    mNotificationIconUnread = loadPixmap( settings, "common/notificationiconunread" );

    if ( mNotificationIcon.isNull() )
    {
        if ( !mNotificationIcon.load( ":res/thunderbird.png" ) )
            Utils::fatal("Cannot load default system tray icon");
    }

    mNotificationDefaultColor = QColor( settings.value(
            "common/defaultcolor", mNotificationDefaultColor.name() ).toString() );
    mThunderbirdFolderPath = settings.value(
            "common/profilepath", mThunderbirdFolderPath ).toString();
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
    mAllowSuppressingUnreads = settings.value(
            "common/allowsuppressingunread", mAllowSuppressingUnreads ).toBool();
    mLaunchThunderbirdDelay = settings.value(
            "common/launchthunderbirddelay", mLaunchThunderbirdDelay ).toInt();
    mShowUnreadEmailCount = settings.value(
            "common/showunreademailcount", mShowUnreadEmailCount ).toBool();

    mThunderbirdCmdLine = settings.value("advanced/tbcmdline", mThunderbirdCmdLine).toString();
    mThunderbirdWindowMatch = settings.value(
            "advanced/tbwindowmatch", mThunderbirdWindowMatch ).toString();
    mNotificationMinimumFontSize = settings.value(
            "advanced/notificationfontminsize", mNotificationMinimumFontSize ).toInt();
    mNotificationMaximumFontSize = settings.value(
            "advanced/notificationfontmaxsize", mNotificationMaximumFontSize ).toInt();
    mUseMorkParser = settings.value("advanced/unreadmorkparser", mUseMorkParser ).toBool();
    mWatchFileTimeout = settings.value("advanced/watchfiletimeout", mWatchFileTimeout ).toUInt();
    mBlinkingUseAlphaTransition = settings.value(
            "advanced/blinkingusealpha", mBlinkingUseAlphaTransition ).toBool();
    mUnreadOpacityLevel = settings.value(
            "advanced/unreadopacitylevel", mUnreadOpacityLevel ).toDouble();

    mFolderNotificationColors.clear();

    // Convert the map into settings
    int total = settings.value("accounts/count", 0 ).toInt();

    for ( int index = 0; index < total; index++ )
    {
        QString entry = "accounts/account" + QString::number( index );
        QString key = settings.value( entry + "URI", "" ).toString();
        while (key.isEmpty() && index < total) {
            Utils::debug("Removing invalid account %d", index);
            QString lastEntry = "accounts/account" + QString::number( total - 1 );
            if (index != total - 1) {
                key = settings.value(lastEntry + "URI", "").toString();
                settings.setValue(entry + "URI", key);
                settings.setValue(entry + "Color",
                        settings.value(lastEntry + "Color", "").toString());
            }
            settings.remove(lastEntry + "URI");
            settings.remove(lastEntry + "Color");
            settings.setValue("accounts/count", --total);
        }
        if (key.isEmpty()) {
            break;
        }
        mFolderNotificationColors[ key ] = QColor( settings.value( entry + "Color", "" ).toString() );
        mFolderNotificationList.push_back( key );
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
}

QString Settings::getThunderbirdExecutablePath() {
    QString path = mThunderbirdCmdLine;
    if (path.startsWith('"')) {
        path = path.section('"', 1, 1);
    }
    path = Utils::expandPath(path);
    return '"' + QFileInfo(path).absoluteFilePath() + '"';
}

const QPixmap &Settings::getNotificationIcon() {
    if (mNotificationIcon.isNull()) {
        if (!mNotificationIcon.load(":res/thunderbird.png")) {
            Utils::fatal("Cannot load default system tray icon");
        }
    }
    return mNotificationIcon;
}

void Settings::setNotificationIcon(const QPixmap& icon) {
    mNotificationIcon = icon;
}

void Settings::savePixmap(QSettings &settings, const QString &key, const QPixmap &pixmap)
{
    // Store the notification icon in the icondata buffer
    QByteArray icondata;
    QBuffer buffer(&icondata);
    buffer.open(QIODevice::WriteOnly);
    pixmap.save(&buffer, "PNG");
    buffer.close();

    settings.setValue( key, icondata );
}

QPixmap Settings::loadPixmap(QSettings &settings, const QString &key)
{
    QPixmap pix;

    if ( settings.contains( key ) )
    {
        pix = QPixmap( mIconSize );
        pix.loadFromData( settings.value( key, "" ).toByteArray(), "PNG" );
        pix.detach();
    }

    return pix;
}
