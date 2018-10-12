#include <QTimer>
#include <QDir>

#include <sqlite3.h>

#include "unreadcounter.h"
#include "sqlite_statement.h"
#include "morkparser.h"
#include "settings.h"
#include "trayicon.h"


UnreadMonitor::UnreadMonitor( TrayIcon * parent )
    : QThread( 0 ), mChangedMSFtimer(this)
{
    mSqlitedb = 0;
    mLastReportedUnread = 0;

    // Everything should be owned by our thread
    moveToThread( this );

    // We get notification once either sqlite file or Mork files have been modified.
    // This way we don't need to pull the db often
    connect( &mDBWatcher, &QFileSystemWatcher::fileChanged, this, &UnreadMonitor::watchedFileChanges );

    // Settings changed
    connect( parent, &TrayIcon::settingsChanged, this, &UnreadMonitor::slotSettingsChanged );

    // Set up the watched file timer
    mChangedMSFtimer.setInterval( pSettings->mWatchFileTimeout );
    mChangedMSFtimer.setSingleShot( true );

    connect( &mChangedMSFtimer, &QTimer::timeout, this, &UnreadMonitor::updateUnread );
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
    QTimer::singleShot( 0, [=](){ updateUnread(); } );

    // Start the event loop
    exec();
}

void UnreadMonitor::slotSettingsChanged()
{
    // We reinitialize everything because the settings changed
    if ( mSqlitedb )
    {
        sqlite3_close_v2( mSqlitedb );
        mSqlitedb = 0;
    }

    mMorkUnreadCounts.clear();

    updateUnread();
}

void UnreadMonitor::watchedFileChanges(const QString &filechanged)
{
    if ( !mChangedMSFfiles.contains( filechanged ) )
        mChangedMSFfiles.push_back( filechanged );

    mChangedMSFtimer.start();
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
    qDebug("Triggering the unread counter update");

    // We execute a single statement and then parse the groups and decide on colors.
    QColor chosencolor;
    int total = 0;

    if ( pSettings->mUseMorkParser )
        getUnreadCount_Mork( total, chosencolor );
    else
        getUnreadCount_SQLite( total, chosencolor);

    if ( total != mLastReportedUnread )
    {
        emit unreadUpdated( total, chosencolor );
        mLastReportedUnread = total;
    }
}

void UnreadMonitor::getUnreadCount_SQLite(int &count, QColor &color)
{
    if ( !mSqlitedb && !openDatabase() )
    {
        // It already reported the error, so exit this thread
        this->exit( 0 );
        return;
    }

    // Let Thunderbird commit it
    this->msleep( 200 );

    SQLiteStatement stmt;

    // This returns the number of unread messages (JSON attribute "59": false)
    if ( !stmt.prepare( mSqlitedb, QString("SELECT folderID FROM messages WHERE folderID IN (%1) AND json_extract( jsonAttributes, '$.59' ) = 0") .arg( mAllFolderIDs) ) )
    {
        emit error("Cannot query database");
        this->exit( 0 );
    }

    int res;
    QColor chosencolor;

    while ( (res = stmt.step()) == SQLITE_ROW )
    {
        // Here we go through all unread messages
        count++;

        // Get the folder id
        qint64 folderId = stmt.columnInt64( 0 );

        // Maybe TB changed the IDs
        if ( mFolderColorMap.contains( folderId ) )
        {
            // IF we have more than one color, bump to default color
            if ( color.isValid() )
            {
                if ( chosencolor != mFolderColorMap[ folderId ] )
                    color = pSettings->mNotificationDefaultColor;
            }
            else
                color = mFolderColorMap[ folderId ];
        }
    }
}

void UnreadMonitor::getUnreadCount_Mork(int &count, QColor &color)
{
    bool rescanall = false;

    // We rebuild and rescan the whole map if there is no such path in there, or map is empty (first run)
    if ( mMorkUnreadCounts.isEmpty() )
        rescanall = true;
    else
    {
        for ( QString path : mChangedMSFfiles )
            if ( !mMorkUnreadCounts.contains( path ) )
                rescanall = false;
    }

    if ( rescanall )
    {
        mMorkUnreadCounts.clear();

        for ( const QString& tpath : pSettings->mFolderNotificationColors.keys() )
        {
            mMorkUnreadCounts[ tpath ] = getMorkUnreadCount( tpath );
            mDBWatcher.addPath( tpath );
        }
    }
    else
    {
        for ( QString path : mChangedMSFfiles )
            mMorkUnreadCounts[ path ] = getMorkUnreadCount( path );
    }

    // Find the total, and set the color
    for ( const QString& tpath : mMorkUnreadCounts.keys() )
    {
        if ( mMorkUnreadCounts[ tpath ] > 0 )
        {
            count += mMorkUnreadCounts[ tpath ];

            if ( color.isValid() )
                color = pSettings->mNotificationDefaultColor;
            else
                color = pSettings->mFolderNotificationColors[ tpath ];
        }
    }

    mChangedMSFfiles.clear();
}

int UnreadMonitor::getMorkUnreadCount(const QString &path)
{
    MorkParser parser;

    if ( !parser.open( path ) )
        return -1;

    int unread = 0;

    // First we parse the unreadChildren column (generic view)
    const MorkRowMap * rows = parser.rows( 0x80, 0, 0x80 );

    if ( rows )
    {
        for ( MorkRowMap::const_iterator rit = rows->begin(); rit != rows->cend(); rit++ )
        {
            MorkCells cells = rit.value();

            for ( int colid : cells.keys() )
            {
                QString columnName = parser.getColumn( colid );

                if ( columnName == "unreadChildren" )
                {
                    unsigned int value = parser.getValue(cells[colid ]).toInt( nullptr, 16 );
                    unread += value;
                }
            }
        }
    }
    else
    {
        // Now parse the smart inbox
        rows = parser.rows( 0x80, 0, 0x9F );

        if ( rows )
        {
            for ( MorkRowMap::const_iterator rit = rows->begin(); rit != rows->cend(); rit++ )
            {
                MorkCells cells = rit.value();

                for ( int colid : cells.keys() )
                {
                    QString columnName = parser.getColumn( colid );

                    if ( columnName == "numNewMsgs" )
                    {
                        unsigned int value = parser.getValue(cells[colid ]).toInt( nullptr, 16 );
                        unread += value;
                    }
                }
            }
        }
    }

    qDebug("Unread counter for %s: %d", qPrintable( path ), unread );
    return unread;
}
