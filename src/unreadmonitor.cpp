#include <QTimer>
#include <QDir>

#include <sqlite3.h>

#include "unreadmonitor.h"
#include "sqlite_statement.h"
#include "morkparser.h"
#include "trayicon.h"
#include "utils.h"
#include "log.h"
#include "databaseaccounts.h"
#include "birdtrayapp.h"

UnreadMonitor::UnreadMonitor( TrayIcon * parent )
    : QThread( 0 ), mChangedMSFtimer(this)
{
    mSqlitedb = 0;
    mLastReportedUnread = 0;

    // We get notification once either sqlite file or Mork files have been modified.
    // This way we don't need to pull the db often
    connect( &mDBWatcher, &QFileSystemWatcher::fileChanged, this, &UnreadMonitor::watchedFileChanges );

    // Settings changed
    connect( parent, &TrayIcon::settingsChanged, this, &UnreadMonitor::slotSettingsChanged );

    // Set up the watched file timer
    mChangedMSFtimer.setInterval(BirdtrayApp::get()->getSettings()->mWatchFileTimeout);
    mChangedMSFtimer.setSingleShot( true );

    connect( &mChangedMSFtimer, &QTimer::timeout, this, &UnreadMonitor::updateUnread );

    // Set up the forced update timer
    mForceUpdateTimer.setSingleShot( false );
    connect( &mForceUpdateTimer, &QTimer::timeout, this, &UnreadMonitor::updateUnread );

    // And activate it if settings specify so
    if ( BirdtrayApp::get()->getSettings()->mIndexFilesRereadIntervalSec > 0 )
    {
        mForceUpdateTimer.setInterval( BirdtrayApp::get()->getSettings()->mIndexFilesRereadIntervalSec * 1000 );
        mForceUpdateTimer.start();
    }
}

UnreadMonitor::~UnreadMonitor() {
    if (isRunning()) {
        quit();
        wait();
    }
    if (mSqlitedb) {
        sqlite3_close_v2(mSqlitedb);
    }
}

void UnreadMonitor::run()
{
    mSqliteDbFile = DatabaseAccounts::getDatabasePath(
            BirdtrayApp::get()->getSettings()->mThunderbirdFolderPath);

    // Start it as soon as thread starts its event loop
    QTimer::singleShot( 0, [=](){ updateUnread(); } );

    // Start the event loop
    exec();
}

const QMap<QString, QString> &UnreadMonitor::getWarnings() const {
    return warnings;
}

void UnreadMonitor::slotSettingsChanged()
{
    // And activate it if settings specify so
    if ( BirdtrayApp::get()->getSettings()->mIndexFilesRereadIntervalSec > 0 )
    {
        mForceUpdateTimer.setInterval( BirdtrayApp::get()->getSettings()->mIndexFilesRereadIntervalSec * 1000 );
        mForceUpdateTimer.start();
    }
    else
        mForceUpdateTimer.stop();

    // We reinitialize everything because the settings changed
    if ( mSqlitedb )
    {
        sqlite3_close_v2( mSqlitedb );
        mSqlitedb = 0;
    }

    mMorkUnreadCounts.clear();
    QStringList accountsList = BirdtrayApp::get()->getSettings()->mFolderNotificationList;
    for (const QString &path : warnings.keys()) {
        if (!accountsList.contains(path)) {
            clearWarning(path);
        }
    }
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
        setWarning(tr("Error opening sqlite database: %1").arg(sqlite3_errmsg(mSqlitedb)));
        return false;
    }

    // Find the IDs for monitored accounts
    mFolderColorMap.clear();
    mAllFolderIDs.clear();

    SQLiteStatement stmt;
    
    if (!stmt.prepare(mSqlitedb, "SELECT id,folderURI FROM folderlocations")) {
        setWarning(tr("Cannot query database: %1").arg(sqlite3_errmsg(mSqlitedb)));
        return false;
    }
    clearWarning();

    // Make a copy as we'd delete them when found
    auto folders = BirdtrayApp::get()->getSettings()->mFolderNotificationColors;

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
            clearWarning(uri);
        }
    }

    // If anything left, we didn't find those
    for (const QString &uri: folders.keys()) {
        setWarning(tr("Folder %1 was not found in database.").arg(uri), uri);
    }
    if (!folders.isEmpty()) {
        return false;
    }

    for ( int id : mFolderColorMap.keys() )
        Log::debug("Color for ID %d: %s", id, qPrintable( mFolderColorMap[id].name() ) );

    Log::debug("List of all IDs: %s", qPrintable( mAllFolderIDs ) );

    mDBWatcher.addPath( mSqliteDbFile );

    // We're good
    return true;
}

void UnreadMonitor::updateUnread()
{
    Log::debug("Triggering the unread counter update");

    // We execute a single statement and then parse the groups and decide on colors.
    QColor chosenColor;
    int total = 0;

    if (BirdtrayApp::get()->getSettings()->mUseMorkParser) {
        getUnreadCount_Mork(total, chosenColor);
    } else {
        getUnreadCount_SQLite(total, chosenColor);
    }

    if ( total != mLastReportedUnread || chosenColor != mLastColor )
    {
        emit unreadUpdated( total, chosenColor );
        mLastReportedUnread = total;
        mLastColor = chosenColor;
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
    if (!stmt.prepare(mSqlitedb,QString(
            "SELECT folderID FROM messages WHERE folderID IN (%1) "
            "AND json_extract( jsonAttributes, '$.59' ) = 0").arg(mAllFolderIDs))) {
        setWarning(tr("Cannot query database."));
        this->exit(0);
    } else {
        clearWarning();
    }

    int res;
    QColor chosenColor;

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
            if ( chosenColor.isValid() )
            {
                if ( chosenColor != mFolderColorMap[ folderId ] ) {
                    chosenColor = BirdtrayApp::get()->getSettings()->mNotificationDefaultColor;
                }
            } else {
                chosenColor = mFolderColorMap[ folderId ];
            }
        }
    }
    color = chosenColor;
}

void UnreadMonitor::getUnreadCount_Mork(int &count, QColor &color)
{
    Settings* settings = BirdtrayApp::get()->getSettings();
    bool rescanall = false;

    // We rebuild and rescan the whole map if there is no such path in there, or map is empty (first run)
    if ( mMorkUnreadCounts.isEmpty() )
        rescanall = true;
    else
    {
        for ( QString path : mChangedMSFfiles )
            if ( !mMorkUnreadCounts.contains( path ) )
                rescanall = true;
    }

    if ( rescanall )
    {
        mMorkUnreadCounts.clear();
        for (const QString &path : settings->mFolderNotificationColors.keys()) {
            mMorkUnreadCounts[path] = getMorkUnreadCount(path);
            if (!mDBWatcher.files().contains(path) && !mDBWatcher.addPath(path)) {
                setWarning(tr("Unable to watch %1 for changes.")
                        .arg(QFileInfo(path).fileName()), path);
            } else {
                clearWarning(path);
            }
        }
    }
    else
    {
        for ( QString path : mChangedMSFfiles )
            mMorkUnreadCounts[ path ] = getMorkUnreadCount( path );
    }

    // Find the total, and set the color
    QColor chosenColor;
    for ( const QString& tpath : mMorkUnreadCounts.keys() )
    {
        if ( mMorkUnreadCounts[ tpath ] > 0 )
        {
            count += mMorkUnreadCounts[ tpath ];

            if ( chosenColor.isValid() ) {
                if (chosenColor != settings->mFolderNotificationColors[tpath]) {
                    chosenColor = settings->mNotificationDefaultColor;
                }
            } else {
                chosenColor = settings->mFolderNotificationColors[ tpath ];
            }
        }
    }
    color = chosenColor;
    mChangedMSFfiles.clear();
}

int UnreadMonitor::getMorkUnreadCount(const QString &path)
{
    MailMorkParser parser;
    if (!parser.open(path)) {
        setWarning(tr("Unable to read from %1.").arg(QFileInfo(path).fileName()), path);
        return 0;
    } else {
        clearWarning(path);
    }
    int unread = static_cast<int>(parser.getNumUnreadMessages());
    Log::debug("Unread counter for %s: %d", qPrintable( path ), unread );
    return unread;
}

void UnreadMonitor::setForcedUpdateTimer()
{

}

void UnreadMonitor::setWarning(const QString &message, const QString &path) {
    if (warnings.value(path) != message) {
        warnings.insert(path, message);
        emit warningChanged(path);
    }
}

void UnreadMonitor::clearWarning(const QString &path) {
    if (warnings.remove(path) > 0) {
        emit warningChanged(path);
    }
}
