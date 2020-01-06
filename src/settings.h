#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSize>
#include <QFont>
#include <QColor>
#include <QMap>
#include <QPixmap>
#include <QSettings>

#include "setting_newemail.h"

class QSettings;

class Settings
{
    public:
        explicit Settings(bool verboseOutput);
        ~Settings();

        // Desired icon size
        QSize   mIconSize;

        // Notification icon for unread emails.
        // If null, the mNotificationIcon is used
        QPixmap mNotificationIconUnread;

        // Font for use in notifications
        QFont   mNotificationFont;

        // Notification font weight (0 - 99)
        unsigned int mNotificationFontWeight;

        // Default notification color
        QColor  mNotificationDefaultColor;
        
        /**
         * The border color to use when drawing the unread mail counter.
         */
        QColor mNotificationBorderColor;
    
        /**
         * The width of the border for the unread mail counter.
         */
        unsigned int mNotificationBorderWidth;

        // Blinking speed
        unsigned int    mBlinkSpeed;

        // Opacity level for the tray icon when unread email is present (0.0-1.0)
        double          mUnreadOpacityLevel;

        // Path to Thunderbird folder
        QString mThunderbirdFolderPath;

        // The command to start Thunderbird. The first element is the executable to launch.
        QStringList mThunderbirdCmdLine;

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
        
        // Whether to check for a new Birdtray version on startup or not.
        bool    mUpdateOnStartup;
        
        // The new Birdtray version that the user selected to ignore.
        QString mIgnoreUpdateVersion;

        // Whether to allow suppression of unread emails
        bool    mAllowSuppressingUnreads;

        // Whether to show the unread email count
        bool    mShowUnreadEmailCount;
        
        /**
         * Ignore the total number of unread emails that are present at startup.
         */
        bool    ignoreStartUnreadCount;
        
        /**
         * Enables or disabled the dialog on startup that shows if no accounts were configured.
         */
        bool    showDialogIfNoAccountsConfigured;

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

        // Whether to spam debugging stuff
        bool    mVerboseOutput;

        // Load and save them
        void    save();
        void    load();
        
        /**
         * @param executable The actual executable necessary to start (not necessary Thunderbird itself - may be shell or script).
         * @param arguments The arguments necessary to start Thunderbird.
         * @return true if executable/arguments is set, false if error
         */
        bool getStartThunderbirdCmdline( QString& executable, QStringList &arguments );
        
        /**
         * @return The icon to use for the system tray.
         */
        const QPixmap& getNotificationIcon();
        
        /**
         * Set the icon to use for the system tray to the new icon.
         *
         * @param icon The new icon.
         */
        void setNotificationIcon(const QPixmap& icon);

    private:
        // Notification icon
        QPixmap mNotificationIcon;

        QSettings *mSettings;
    
        void    savePixmap( const QString& key, const QPixmap& pixmap );
        QPixmap loadPixmap( const QString& key );
    
        /**
         * At first start, load the configuration configured during installation of Birdtray.
         */
        void loadInstallerConfiguration();
};

#endif // SETTINGS_H
