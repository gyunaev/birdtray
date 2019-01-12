#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSize>
#include <QFont>
#include <QColor>
#include <QMap>
#include <QPixmap>
#include <QSettings>

#include "setting_newemail.h"

class Settings
{
    public:
        Settings();

        // Desired icon size
        QSize   mIconSize;

        // Notification icon
        QPixmap mNotificationIcon;

        // Notification icon for unread emails.
        // If null, the mNotificationIcon is used
        QPixmap mNotificationIconUnread;

        // Font for use in notifications
        QFont   mNotificationFont;

        // Notification font weight
        unsigned int mNotificationFontWeight;

        // Default notification color
        QColor  mNotificationDefaultColor;

        // Blinking speed
        unsigned int    mBlinkSpeed;

        // Opacity level for the tray icon when unread email is present (0.0-1.0)
        double          mUnreadOpacityLevel;

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

        // The delay in seconds to launch Thunderbird
        int     mLaunchThunderbirdDelay;

        // Whether to hide Thunderbird window after starting
        bool    mHideWhenStarted;

        // Whether to quit Thunderbird when the app quits
        bool    mExitThunderbirdWhenQuit;

        // Whether to restart Thunderbird if it was closed
        bool    mRestartThunderbird;

        // Whether to hide Thunderbird window after restarting
        bool    mHideWhenRestarted;

        // Whether to monitor Thunderbird running
        bool    mMonitorThunderbirdWindow;

        // Whether to use Mork parser for new mail scanning; if false, sqlite is used
        bool    mUseMorkParser;

        // Whether to use alpha transition when blinking
        bool    mBlinkingUseAlphaTransition;

        // Whether to allow suppression of unread emails
        bool    mAllowSuppressingUnreads;

        // Whether to show the unread email count
        bool    mShowUnreadEmailCount;

        // Watching file timeout (ms)
        unsigned int mWatchFileTimeout;

        // The smallest and the largest allowed font in notification
        unsigned int mNotificationMinimumFontSize;
        unsigned int mNotificationMaximumFontSize;

        // New email data
        bool    mNewEmailMenuEnabled;
        QList< Setting_NewEmail >   mNewEmailData;

        // Maps the folder URI or full path (for Mork) to the notification color.
        // The original order of strings is stored in mFolderNotificationList (to show in UI)
        QMap< QString, QColor >   mFolderNotificationColors;
        QStringList               mFolderNotificationList;

        // Load and save them
        void    save();
        void    load();

    private:
        void    savePixmap( QSettings& settings, const QString& key, const QPixmap& pixmap );
        QPixmap loadPixmap( QSettings& settings, const QString& key );
};

extern Settings * pSettings;

#endif // SETTINGS_H
