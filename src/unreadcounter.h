#ifndef FOLDERWATCHER_H
#define FOLDERWATCHER_H

#include <QThread>
#include <QString>
#include <QColor>
#include <QMap>
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

    private:
        bool    openDatabase();
        void    updateUnread( const QString& filechanged = "" );

        void    getUnreadCount_SQLite( int & count, QColor& color );
        void    getUnreadCount_Mork( const QString& path, int & count, QColor& color );
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

        // Last reported unread
        int    mLastReportedUnread;
};

#endif // FOLDERWATCHER_H
