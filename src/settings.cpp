#include <QStandardPaths>
#include <QBuffer>
#include <QDir>

#include "settings.h"

Settings * pSettings;

Settings::Settings()
{
    mIconSize = QSize( 128, 128 );
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
            qFatal("cannot load icon");
    }

    mNotificationDefaultColor = QColor( settings.value( "common/defaultcolor", "#00FF00" ).toString() );
    mThunderbirdFolderPath = settings.value( "common/profilepath", "" ).toString();
    mBlinkSpeed = settings.value("common/blinkspeed", 0 ).toInt();
    mShowHideThunderbird = settings.value("common/showhidethunderbird", false ).toBool();
    mLaunchThunderbird = settings.value("common/launchthunderbird", false ).toBool();
    mHideWhenMinimized = settings.value("common/hidewhenminimized", false ).toBool();
    mExitThunderbirdWhenQuit = settings.value("common/exitthunderbirdonquit", false ).toBool();
    mNotificationFontWeight = settings.value("common/notificationfontweight", 50 ).toInt();
    mMonitorThunderbirdWindow = settings.value("common/monitorthunderbirdwindow", false ).toBool();
    mRestartThunderbird = settings.value("common/restartthunderbird", false ).toBool();
    mHideWhenStarted = settings.value("common/hidewhenstarted", false ).toBool();
    mHideWhenRestarted = settings.value("common/hidewhenrestarted", false ).toBool();
    mAllowSuppressingUnreads = settings.value("common/allowsuppressingunread", false ).toBool();

    mThunderbirdCmdLine = settings.value("advanced/tbcmdline", "/usr/bin/thunderbird" ).toString();
    mThunderbirdWindowMatch = settings.value("advanced/tbwindowmatch", "- Mozilla Thunderbird" ).toString();
    mNotificationMinimumFontSize = settings.value("advanced/notificationfontminsize", 4 ).toInt();
    mNotificationMaximumFontSize = settings.value("advanced/notificationfontmaxsize", 512 ).toInt();
    mUseMorkParser = settings.value("advanced/unreadmorkparser", true ).toBool();
    mWatchFileTimeout = settings.value("advanced/watchfiletimeout", 150 ).toUInt();
    mBlinkingUseAlphaTransition = settings.value("advanced/blinkingusealpha", false ).toBool();
    mUnreadOpacityLevel = settings.value("advanced/unreadopacitylevel", 0.75 ).toDouble();

    mFolderNotificationColors.clear();

    // Convert the map into settings
    int total = settings.value("accounts/count", 0 ).toInt();

    for ( int index = 0; index < total; index++ )
    {
        QString entry = "accounts/account" + QString::number( index );
        QString key = settings.value( entry + "URI", "" ).toString();
        mFolderNotificationColors[ key ] = QColor( settings.value( entry + "Color", "" ).toString() );
        mFolderNotificationList.push_back( key );
    }

    // Load new email data from settings
    mNewEmailMenuEnabled = settings.value("newemail/enabled", false ).toBool();

    mNewEmailData.clear();
    total = settings.value("newemail/count", 0 ).toInt();

    for ( int index = 0; index < total; index++ )
    {
        QString entry = "newemail/id" + QString::number( index );
        mNewEmailData.push_back( Setting_NewEmail::fromByteArray( settings.value( entry, "" ).toByteArray() ) );
    }
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
