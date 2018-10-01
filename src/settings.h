#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSize>
#include <QFont>
#include <QColor>
#include <QMap>
#include <QPixmap>
#include <QJsonObject>

class Settings
{
    public:
        Settings();

        // Desired icon size
        QSize   mIconSize;

        // Notification icon
        QPixmap mNotificationIcon;

        // Font for use in notifications
        QFont   mNotificationFont;

        // Notification font weight
        unsigned int mNotificationFontWeight;

        // Default notification color
        QColor  mNotificationDefaultColor;

        // Blinking speed
        unsigned int    mBlinkSpeed;

        // Path to Thunderbird folder
        QString mThunderbirdFolderPath;

        // Thunderbird binary and command line
        QString mThunderbirdCmdLine;

        // Thunderbird window match
        QString mThunderbirdWindowMatch;

        // Whether to show/hide Thunderbird on button click
        bool    mShowHideThunderbird;

        // Whether to hide Thunderbird when its window is minimized
        bool    mHideWhenMinimized;

        // Whether to launch Thunderbird when the app starts
        bool    mLaunchThunderbird;

        // Whether to quit Thunderbird when the app quits
        bool    mExitThunderbirdWhenQuit;

        // Whether to launch Thunderbird when the app starts
        bool    mRestartThunderbird;

        // Whether to monitor Thunderbird running
        bool    mMonitorThunderbirdWindow;

        // The smallest and the largest allowed font in notification
        unsigned int mNotificationMinimumFontSize;
        unsigned int mNotificationMaximumFontSize;

        // Maps the folder URI to the notification color
        QMap< QString, QColor >   mFolderNotificationColors;

        // Load and save them
        void    save();
        void    load();

    private:
};

extern Settings * pSettings;

#endif // SETTINGS_H
