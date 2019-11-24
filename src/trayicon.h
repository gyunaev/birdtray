#ifndef TRAYICON_H
#define TRAYICON_H

#include <QTimer>
#include <QDateTime>
#include <QWidget>
#include <QImage>
#include <QProcess>
#include <QSystemTrayIcon>
#include <QtNetwork/QNetworkConfigurationManager>
#ifdef Q_OS_WIN
#  include "processhandle.h"
#endif /* Q_OS_WIN */
#include "dialogsettings.h"

class UnreadMonitor;
class WindowTools;

class TrayIcon : public QSystemTrayIcon
{
    Q_OBJECT

    public:
        explicit TrayIcon(bool showSettings);
        ~TrayIcon() override;
        
        /**
         * @return The window tools used by the tray icon.
         */
        WindowTools* getWindowTools() const;

    signals:
        void    settingsChanged();

    public slots:
        void    unreadCounterUpdate(unsigned int total, QColor color );

        /**
         * The warning status of a watched path in the unread monitor changed.
         *
         * @param path The path whose warning has changed or a null-string for a global warning.
         */
        void unreadMonitorWarningChanged(const QString &path);


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
         * Callback if Thunderbird fails to start.
         * @param error The reason for the start failure.
         */
        void    tbProcessError( QProcess::ProcessError error);
        void    tbProcessFinished( int exitCode, QProcess::ExitStatus exitStatus );
#ifdef Q_OS_WIN
        /**
         * Callback if the Thunderbird updater exits.
         * @param exitReason The reason for the exit.
         */
        void    tbUpdaterProcessFinished(const ProcessHandle::ExitReason& exitReason);
#endif /* Q_OS_WIN */
        /**
         * Callback that is called when we are about to quit.
         */
        void    onQuit();
        
        /**
         * Called when the auto update finished.
         *
         * @param errorMessage A message indicating an error during the check, or a null string.
         */
        void    onAutoUpdateCheckFinished(const QString& errorMessage);

    private:
        void    createMenu();
        void    createUnreadCounterThread();
        void    hideThunderbird();
        void    showThunderbird();
        void    updateIgnoredUnreads();
        
        /**
         * Do an automatic check for a new version of Birdtray.
         */
        void    doAutoUpdateCheck();

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
        
        /**
         * The number of unread emails at Birdtray startup.
         */
        long            unreadEmailsAtStart = -1;

        // Show/hide Thunderbird menu item (we modify its text)
        QAction *       mMenuShowHideThunderbird;

        // Ignore unread emails item (we modify its text) - only if we have this functionality
        QAction *       mMenuIgnoreUnreads;

        // Unsnooze menu item
        QAction *       mMenuUnsnooze;
        QDateTime       mSnoozedUntil;

        // Unread counter thread
        UnreadMonitor * mUnreadMonitor = nullptr;

        // State checking timer (once a second)
        QTimer          mStateTimer;

        // Time when Thunderbird could be started
        QDateTime       mThunderbirdStartTime;

        // If true, Thunderbird window existed anytime before, but not necessarily now
        // (we use this to distinguish between start and restart)
        bool            mThunderbirdWindowExisted;

        // If true, Thunderbird window exists right now
        bool            mThunderbirdWindowExists;

        // If true, it will hide Thunderbird window as soon as its shown
        bool            mThunderbirdWindowHide;

        // Number of suppressed unread emails, if nonzero
        unsigned int    mIgnoredUnreadEmails;

        // Window tools (show/hide)
        WindowTools *   mWinTools;

        // Cached last drawn icon
        QImage          mLastDrawnIcon;

        // Thunderbird process which we have started. This can be nullptr if Thunderbird
        // was started before Birdtray (thus our process would just activate it and exit)
        // Thus checking this pointer for null doesn't mean TB is not started.
        QProcess    *   mThunderbirdProcess;

        // System tray context menu. Once set, it remains there, so we have to modify existing one
        QMenu       *   mSystrayMenu;
    
        /**
         * The currently opened settings dialog.
         */
        DialogSettings* settingsDialog = nullptr;

#ifdef Q_OS_WIN
        // A reference to a Thunderbird updater process.
        ProcessHandle* mThunderbirdUpdaterProcess;
#endif /* Q_OS_WIN */
        
        /**
         * A manager to check for network connectivity.
         */
        QNetworkConfigurationManager* networkConnectivityManager = nullptr;
};

#endif // TRAYICON_H
