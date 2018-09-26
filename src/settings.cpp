#include <QStandardPaths>
#include <QSettings>
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
    settings.setValue("common/notificationfontweight", mNotificationFontWeight );
    settings.setValue("common/monitorthunderbirdwindow", mMonitorThunderbirdWindow );

    settings.setValue("advanced/tbcmdline", mThunderbirdCmdLine );
    settings.setValue("advanced/tbwindowmatch", mThunderbirdWindowMatch );

    // Convert the map into settings
    settings.setValue("accounts/count", mFolderNotificationColors.size() );
    int index = 0;

    for ( QString uri : mFolderNotificationColors.keys() )
    {
        QString entry = "accounts/account" + QString::number( index );
        settings.setValue( entry + "Color", mFolderNotificationColors[uri].name() );
        settings.setValue( entry + "URI", uri );

        index++;
    }

    // Store the notification icon in the icondata buffer
    QByteArray icondata;
    QBuffer buffer(&icondata);
    buffer.open(QIODevice::WriteOnly);
    mNotificationIcon.save(&buffer, "PNG");
    buffer.close();

    settings.setValue("common/notificationicon", icondata );
}

void Settings::load()
{
    QSettings settings;

    if ( settings.contains( "common/notificationfont" ) )
        mNotificationFont.fromString( settings.value( "common/notificationfont", "" ).toString() );

    if ( settings.contains( "common/notificationicon" ) )
    {
        mNotificationIcon = QPixmap( mIconSize );
        mNotificationIcon.loadFromData( settings.value("common/notificationicon", "" ).toByteArray(), "PNG" );
    }

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

    mThunderbirdCmdLine = settings.value("advanced/tbcmdline", "/usr/bin/thunderbird" ).toString();
    mThunderbirdWindowMatch = settings.value("advanced/tbwindowmatch", "- Mozilla Thunderbird" ).toString();

    mFolderNotificationColors.clear();

    // Convert the map into settings
    int total = settings.value("accounts/count", 0 ).toInt();

    for ( int index = 0; index < total; index++ )
    {
        QString entry = "accounts/account" + QString::number( index );
        mFolderNotificationColors[ settings.value( entry + "URI", "" ).toString() ] = QColor( settings.value( entry + "Color", "" ).toString() );
    }
}
