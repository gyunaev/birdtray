#ifndef TRAYICON_H
#define TRAYICON_H

#include <QTimer>
#include <QDateTime>
#include <QWidget>
#include <QImage>
#include <QSystemTrayIcon>

class UnreadMonitor;
class WindowTools;

class TrayIcon : public QSystemTrayIcon
{
    Q_OBJECT

    public:
        TrayIcon();

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
        void    actionQuit();
        void    actionSettings();
        void    actionActivate();
        void    actionSnoozeFor();
        void    actionUnsnooze();
        void    actionNewEmail();

        void    actionSystrayIconActivated( QSystemTrayIcon::ActivationReason reason );

        void    startThunderbird();

    private:
        void    createMenu();
        void    createUnreadCounterThread();
        bool    hasThunderbirdRecentlyStarted() const;

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

        // Unsnooze menu item
        QAction *       mMenuUnsnooze;
        QDateTime       mSnoozedUntil;

        // Unread counter thread
        UnreadMonitor * mUnreadMonitor;

        // State checking timer (once a second)
        QTimer          mStateTimer;

        // Thunderbird restart timestamp (to avoid restarting too often)
        QDateTime       mThunderbirdRestartTime;

        // Current status
        QString         mCurrentStatus;

        // If true, Thunderbird window existed anytime before, but not necessarily now
        // (we use this to distinguish between start and restart)
        bool            mThunderbirdWindowExisted;

        // If true, Thunderbird window exists right now
        bool            mThunderbirdWindowExists;

        // If true, it will hide Thunderbird window as soon as its shown
        bool            mThunderbirdWindowHide;

        // Window tools (show/hide)
        WindowTools *   mWinTools;

        // Cached last drawn icon
        QImage          mLastDrawnIcon;
};

#endif // TRAYICON_H
