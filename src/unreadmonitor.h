#ifndef UNREAD_MONITOR_H
#define UNREAD_MONITOR_H

#include <QThread>
#include <QString>
#include <QColor>
#include <QMap>
#include <QTimer>
#include <QStringList>
#include <QFileSystemWatcher>

class TrayIcon;

class UnreadMonitor : public QThread
{
    Q_OBJECT

    public:
        UnreadMonitor( TrayIcon * parent );
        virtual ~UnreadMonitor();

        // Thread run function
        virtual void run() override;
    
        /**
         * Get the current warnings for the watched paths.
         * The null-string key can contain a warning for all paths.
         *
         * @return A path to warnings mapping.
         */
        const QMap<QString, QString> &getWarnings() const;

    signals:
        // Unread counter changed
        void    unreadUpdated( unsigned int total, QColor color );

        /**
         * A warning was added or removed for the given watched path.
         *
         * @param path The watched path or null-string for a global warning.
         */
        void warningChanged(const QString &path);

    public slots:
        void    slotSettingsChanged();
        void    watchedFileChanges( const QString& filechanged );
        void    updateUnread();

    private:
        void    getUnreadCount_Mork( int & count, QColor& color );
        int     getMorkUnreadCount( const QString& path );
    
        /**
         * Set a warning for a given path or for all paths, if no path is given.
         * Overwrites any previous warning.
         *
         * @param message The warning message.
         * @param path The watched path.
         */
        void setWarning(const QString &message, const QString &path = QString());
        
        /**
         * Reset the warning if there was one for the given watched path.
         * Or reset the global warning for all paths, if no path is given.
         *
         * @param path The path to the watched mork file.
         */
        void clearWarning(const QString &path = QString());

    private:
        // Maps the Mork files to unread counts
        QMap< QString, quint32 >  mMorkUnreadCounts;

        // Watches the files for changes
        QFileSystemWatcher  mDBWatcher;

        // Thunderbird tends to do lots of modifications to the MSF file
        // each time a new email arrives. This results in lots of notifications,
        // and thus lots of unread calls. To avoid this, we set up the timer each
        // time a new notification is issued, and reset it for each further notification.
        // We only read the file(s) if the timer expires.
        QTimer              mChangedMSFtimer;

        // List of changed files (for MSF monitoring)
        QList<QString>      mChangedMSFfiles;

        // This timer is used when force update is set, triggering such update
        QTimer              mForceUpdateTimer;

        // Last reported unread
        int    mLastReportedUnread;
        QColor mLastColor;
    
        /**
         * This contains mappings from watched paths to warning messages for that path.
         * If there is a warning in the null-string key, it applies to all paths.
         */
        QMap<QString, QString> warnings;
};

#endif /* UNREAD_MONITOR_H */
