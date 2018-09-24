#include <QTimer>
#include <QDir>

#include <sqlite3.h>

#include "unreadcounter.h"
#include "sqlite_statement.h"
#include "settings.h"


UnreadMonitor::UnreadMonitor()
    : QThread( 0 )
{
    mSqlitedb = 0;
    mLastReportedUnread = 0;

    // Everything should be owned by our thread
    moveToThread( this );

    // We get notification once sqlite file has been modified. This way we don't need to pull the db often
    connect( &mDBWatcher, &QFileSystemWatcher::fileChanged, this, &UnreadMonitor::updateUnread );
}

UnreadMonitor::~UnreadMonitor()
{
    if ( mSqlitedb )
        sqlite3_close_v2( mSqlitedb );
}

void UnreadMonitor::run()
{
    mSqliteDbFile = pSettings->mThunderbirdFolderPath + QDir::separator() + "global-messages-db.sqlite";

    // Start it as soon as thread starts its event loop
    QTimer::singleShot( 0, this, &UnreadMonitor::updateUnread );

    // Start the event loop
    exec();
}

bool UnreadMonitor::openDatabase()
{
    // Could be used for reopening as well
    if ( mSqlitedb )
        sqlite3_close_v2( mSqlitedb );


    // Open the database
    if ( sqlite3_open_v2( mSqliteDbFile.toUtf8().data(),
                           &mSqlitedb,
                           SQLITE_OPEN_READONLY, 0 ) != SQLITE_OK )
    {
        emit error( QString("Error opening sqlite database: %1") .arg( sqlite3_errmsg(mSqlitedb) ) );
        return false;
    }

    // Find the IDs for monitored accounts
    mFolderColorMap.clear();
    mAllFolderIDs.clear();

    SQLiteStatement stmt;

    if ( !stmt.prepare( mSqlitedb, "SELECT id,folderURI FROM folderlocations") )
        return false;

    // Make a copy as we'd delete them when found
    auto folders = pSettings->mFolderNotificationColors;

    while ( stmt.step() == SQLITE_ROW )
    {
        int id = stmt.columnInt( 0 );
        QString uri = stmt.columnText( 1 );

        if ( folders.contains( uri ) )
        {
            mFolderColorMap[ id ] = folders[uri];

            if ( !mAllFolderIDs.isEmpty() )
                mAllFolderIDs += ",";

            mAllFolderIDs += QString::number( id );
            folders.remove( uri );
        }
    }

    // If anything left, we didn't find those
    if ( !folders.isEmpty() )
    {
        emit error( QString("Folder %1 was not found in database") .arg( folders.firstKey() ) );
        return false;
    }

    for ( int id : mFolderColorMap.keys() )
        qDebug("Color for ID %d: %s", id, qPrintable( mFolderColorMap[id].name() ) );

    qDebug("List of all IDs: %s", qPrintable( mAllFolderIDs ) );

    mDBWatcher.addPath( mSqliteDbFile );

    // We're good
    return true;
}

void UnreadMonitor::updateUnread()
{
    if ( !mSqlitedb && !openDatabase() )
    {
        // It already reported the error, so exit this thread
        this->exit( 0 );
        return;
    }

    // Let Thunderbird commit it
    this->msleep( 200 );

    // We execute a single statement and then parse the groups and decide on colors.
    QColor chosencolor;
    unsigned int total = 0;

    SQLiteStatement stmt;

    // This returns the number of unread messages (JSON attribute "59": false)
    if ( !stmt.prepare( mSqlitedb, QString("SELECT folderID FROM messages WHERE folderID IN (%1) AND json_extract( jsonAttributes, '$.59' ) = 0") .arg( mAllFolderIDs) ) )
    {
        emit error("Cannot query database");
        this->exit( 0 );
    }

    int res;

    while ( (res = stmt.step()) == SQLITE_ROW )
    {
        // Here we go through all unread messages
        total++;

        // Get the folder id
        qint64 folderId = stmt.columnInt64( 0 );

        // Maybe TB changed the IDs
        if ( mFolderColorMap.contains( folderId ) )
        {
            // IF we have more than one color, bump to default color
            if ( chosencolor.isValid() )
            {
                if ( chosencolor != mFolderColorMap[ folderId ] )
                    chosencolor = pSettings->mNotificationDefaultColor;
            }
            else
                chosencolor = mFolderColorMap[ folderId ];
        }
    }

    if ( total != mLastReportedUnread )
    {
        emit unreadUpdated( total, chosencolor );
        mLastReportedUnread = total;
    }

    //QTimer::singleShot( 1000, this, &UnreadMonitor::updateUnread );
}
