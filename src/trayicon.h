#ifndef TRAYICON_H
#define TRAYICON_H

#include <QTimer>
#include <QDateTime>
#include <QWidget>
#include <QSystemTrayIcon>

class UnreadMonitor;

class TrayIcon : public QSystemTrayIcon
{
    Q_OBJECT

    public:
        TrayIcon();

    public slots:
        void    unreadCounterUpdate(unsigned int total, QColor color );

        // An error happened
        void    unreadCounterError( QString message );


    private slots:
        // Updates the icon, this is called in blinking and snooze
        void    updateIcon();

        // Set or reset blinking. Each timeoutms the opacity changes by percentagechange
        // For example, for on-off every 500ms call it with (500, 100)
        void    setBlinking( int timeoutms, int percentagechange );

        // Context menu actions
        void    actionQuit();
        void    actionSettings();
        void    actionActivate();
        void    actionSnoozeFor();
        void    actionUnsnooze();

    private:
        void    createMenu();
        void    createUnreadCounterThread();

        // The system tray icon
        QPixmap         mIconPixmap;

        // State variables for blinking; mBlinkingTimeout=0 means we are not blinking
        double          mBlinkingIconOpacity;
        double          mBlinkingDelta;
        unsigned int    mBlinkingTimeout;
        QTimer          mBlinkingTimer;

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

        // Current status
        QString         mCurrentStatus;
};

#endif // TRAYICON_H
