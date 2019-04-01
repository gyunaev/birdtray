#ifndef TRAYICON_H
#define TRAYICON_H

#include <QTimer>
#include <QDateTime>
#include <QWidget>
#include <QImage>
#include <QProcess>
#include <QSystemTrayIcon>
#include "processhandle.h"

class UnreadMonitor;
class WindowTools;

class TrayIcon : public QSystemTrayIcon
{
    Q_OBJECT

    public:
        explicit TrayIcon(bool showSettings);
        ~TrayIcon() override;

    signals:
        void    settingsChanged();

    public slots:
        void    unreadCounterUpdate(unsigned int total, QColor color );

        // An error happened
        void    unreadCounterError( QString message );


    private slots:
        // Updates the icon, this is called in blinking and snooze
        void    updateIcon();

        // Set or reset blinking. Each timeoutms the opacity changes by percentagechange
        // For example, for on-off every 500ms call it with (500, 100)
        void    enableBlinking( bool enabled );

        // Checks the application current state
        void    updateState();

        // Blinking timer
        void    blinkTimeout();

        // Context menu actions
        static void actionQuit();
        void    actionSettings();
        void    actionActivate();
        void    actionSnoozeFor();
        void    actionUnsnooze();
        void    actionNewEmail();
        void    actionIgnoreEmails();

        void    actionSystrayIconActivated( QSystemTrayIcon::ActivationReason reason );

        void    startThunderbird();
        
        /**
         * Callback if Thunderbird exits or fails to start.
         * @param exitReason The reason for the exit.
         */
        void    tbProcessFinished(ProcessHandle::ExitReason exitReason);
    
       /**
        * Callback if the Thunderbird updater exits.
        * @param exitReason The reason for the exit.
        */
        void    tbUpdaterProcessFinished(ProcessHandle::ExitReason exitReason);
    
        /**
         * Callback that is called when we are about to quit.
         */
        void    onQuit();

    private:
        void    createMenu();
        void    createUnreadCounterThread();
        void    hideThunderbird();
        void    showThunderbird();
        void    updateIgnoredUnreads();

        // State variables for blinking; mBlinkingTimeout=0 means we are not blinking
        double          mBlinkingIconOpacity;
        double          mBlinkingDelta;
        unsigned int    mBlinkingTimeout;
        QTimer          mBlinkingTimer;

        // To distinguish whe
        bool            mBlinkTick;

        // Current unread messages count and color
        unsigned int    mUnreadCounter;
        QColor          mUnreadColor;

        // Show/hide Thunderbird menu item (we modify its text)
        QAction *       mMenuShowHideThunderbird;

        // Ignore unread emails item (we modify its text) - only if we have this functionality
        QAction *       mMenuIgnoreUnreads;

        // Unsnooze menu item
        QAction *       mMenuUnsnooze;
        QDateTime       mSnoozedUntil;

        // Unread counter thread
        UnreadMonitor * mUnreadMonitor;

        // State checking timer (once a second)
        QTimer          mStateTimer;

        // Current status
        QString         mCurrentStatus;

        // Time when Thunderbird could be started
        QDateTime       mThunderbirdStartTime;

        // If true, Thunderbird window existed anytime before, but not necessarily now
        // (we use this to distinguish between start and restart)
        bool            mThunderbirdWindowExisted;

        // If true, Thunderbird window exists right now
        bool            mThunderbirdWindowExists;

        // If true, it will hide Thunderbird window as soon as its shown
        bool            mThunderbirdWindowHide;
        
        // If true, we tried to start Thunderbird before and it failed.
        bool            mThunderbirdStartFailed;

        // Number of suppressed unread emails, if nonzero
        unsigned int    mIgnoredUnreadEmails;

        // Window tools (show/hide)
        WindowTools *   mWinTools;

        // Cached last drawn icon
        QImage          mLastDrawnIcon;

        // A reference to a Thunderbird process.
        ProcessHandle* mThunderbirdProcess;

        // A reference to a Thunderbird updater process.
        ProcessHandle* mThunderbirdUpdaterProcess;
};

#endif // TRAYICON_H
