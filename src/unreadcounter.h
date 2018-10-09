#ifndef FOLDERWATCHER_H
#define FOLDERWATCHER_H

#include <QThread>
#include <QString>
#include <QColor>
#include <QMap>
#include <QTimer>
#include <QStringList>
#include <QFileSystemWatcher>

struct sqlite3;
class TrayIcon;

class UnreadMonitor : public QThread
{
    Q_OBJECT

    public:
        UnreadMonitor( TrayIcon * parent );
        virtual ~UnreadMonitor();

        // Thread run function
        virtual void run() override;

    signals:
        // Unread counter changed
        void    unreadUpdated( unsigned int total, QColor color );

        // An error happened
        void    error( QString message );

    public slots:
        void    slotSettingsChanged();
        void    watchedFileChanges( const QString& filechanged );
        void    updateUnread();

    private:
        bool    openDatabase();

        void    getUnreadCount_SQLite( int & count, QColor& color );
        void    getUnreadCount_Mork( int & count, QColor& color );
        int     getMorkUnreadCount( const QString& path );

    private:
        QString         mSqliteDbFile;
        sqlite3 *       mSqlitedb;

        // The list of all folder IDs which we monitor
        QString         mAllFolderIDs;

        // Maps the database folder ID to the notification color
        QMap< qint64, QColor > mFolderColorMap;

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

        // Last reported unread
        int    mLastReportedUnread;
};

#endif // FOLDERWATCHER_H
