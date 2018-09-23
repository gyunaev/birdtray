#include <QStandardPaths>
#include <QSettings>
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

    settings.setValue("common/notificationfont", mTextFont.toString() );
    settings.setValue("common/defaultcolor", mTextColor.name() );
    settings.setValue("common/profilepath", mThunderbirdFolderPath );
    settings.setValue("common/blinkspeed", mBlinkSpeed );
    settings.setValue("common/showhidethunderbird", mShowHideThunderbird );
    settings.setValue("common/launchthunderbird", mLaunchThunderbird );
    settings.setValue("common/hidewhenminimized", mHideWhenMinimized );

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
}

void Settings::load()
{
    QSettings settings;

    if ( settings.contains( "common/notificationfont" ) )
        mTextFont.fromString( settings.value( "common/notificationfont", "" ).toString() );

    mTextColor = QColor( settings.value( "common/defaultcolor", "#00FF00" ).toString() );
    mThunderbirdFolderPath = settings.value( "common/profilepath", "" ).toString();
    mBlinkSpeed = settings.value("common/blinkspeed", 0 ).toInt();
    mShowHideThunderbird = settings.value("common/showhidethunderbird", false ).toBool();
    mLaunchThunderbird = settings.value("common/launchthunderbird", false ).toBool();
    mHideWhenMinimized = settings.value("common/hidewhenminimized", false ).toBool();

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
