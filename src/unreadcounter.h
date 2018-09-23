#ifndef FOLDERWATCHER_H
#define FOLDERWATCHER_H

#include <QThread>
#include <QString>
#include <QColor>
#include <QMap>
#include <QStringList>
#include <QFileSystemWatcher>

struct sqlite3;

class UnreadMonitor : public QThread
{
    Q_OBJECT

    public:
        UnreadMonitor();
        virtual ~UnreadMonitor();

        // Thread run function
        virtual void run() override;

    signals:
        // Unread counter changed
        void    unreadUpdated( unsigned int total, QColor color );

        // An error happened
        void    error( QString message );

    private:
        bool    openDatabase();
        void    updateUnread();

    private:
        QString         mSqliteDbFile;
        sqlite3 *       mSqlitedb;

        // The list of all folder IDs which we monitor
        QString         mAllFolderIDs;

        // Maps the database folder ID to the notification color
        QMap< qint64, QColor > mFolderColorMap;

        // Watches the sqlite database for changes
        QFileSystemWatcher  mDBWatcher;

        // Last reported unread
        unsigned int    mLastReportedUnread;
};

#endif // FOLDERWATCHER_H
