#include <QDir>

#include "databaseaccounts.h"
#include "sqlite_statement.h"
#include "sqlite3.h"

DatabaseAccounts::DatabaseAccounts( const QString& databasePath  )
{
    moveToThread( this );
    mDbPath = databasePath;
}

const QList<DatabaseAccounts::Account> &DatabaseAccounts::accounts() const
{
    return mAccounts;
}

void DatabaseAccounts::run()
{
    // Query the accounts and post the events
    queryAccounts();

    // Post the quit event
    this->quit();

    // Run the event loop
    exec();
}

void DatabaseAccounts::queryAccounts()
{
    // Open the database
    sqlite3 * sqlitedb;
    if ( sqlite3_open_v2( mDbPath.toUtf8().data(),
                           &sqlitedb,
                           SQLITE_OPEN_READONLY, 0 ) != SQLITE_OK )
    {
        emit done( QString("Error opening sqlite database: %1") .arg( sqlite3_errmsg(sqlitedb) ) );
        return;
    }

    SQLiteStatement stmt;

    if ( !stmt.prepare( sqlitedb, "SELECT id,folderURI FROM folderlocations") )
    {
        emit done ("Cannot access the database. If you're using Thunderbird 68+, this method no longer works. Please use the Mork parser." );
        return;
    }

    int res;

    while ( (res = stmt.step()) != SQLITE_DONE )
    {
        Account acc;

        // Get the ID and data
        acc.id = stmt.columnInt64( 0 );
        acc.uri = stmt.columnText( 1 );

        mAccounts.push_back( acc );
    }

    emit done ( "" );
}

const QString DatabaseAccounts::getDatabasePath(const QString &directory) {
    return QDir(directory).filePath("global-messages-db.sqlite");
}
