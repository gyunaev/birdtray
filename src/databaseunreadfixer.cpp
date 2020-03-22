#include <QJsonDocument>
#include <QJsonObject>
#include <QDir>

#include "databaseunreadfixer.h"
#include "sqlite_statement.h"
#include "sqlite3.h"
#include "utils.h"
#include "log.h"

DatabaseUnreadFixer::DatabaseUnreadFixer(const QString &databasePath )
    : QThread()
{
    moveToThread( this );
    mDbPath = databasePath;
}

void DatabaseUnreadFixer::run()
{
    // Update the database and post the events
    updateDatabase();

    // Post the quit event
    this->quit();

    // Run the event loop
    exec();
}

void DatabaseUnreadFixer::updateDatabase()
{
    // Open the database
    sqlite3 * sqlitedb;
    if ( sqlite3_open_v2( mDbPath.toUtf8().data(),
                           &sqlitedb,
                           SQLITE_OPEN_READWRITE, 0 ) != SQLITE_OK )
    {
        emit done(tr("Error opening sqlite database: %1").arg(sqlite3_errmsg(sqlitedb)));
        return;
    }

    // Find the total rows to change
    qint64 totalrows = 0, currentrow = 0;

    SQLiteStatement stmt, stmtrows;
    
    if (!stmtrows.prepare(
            sqlitedb,
            "SELECT COUNT(id) FROM messages WHERE json_extract( jsonAttributes, '$.59' ) = 0") ||
        stmtrows.step() != SQLITE_ROW) {
        emit done(tr("Cannot query database."));
        return;
    }

    totalrows = stmtrows.columnInt64( 0 );
    Log::debug("total rows to change: %lld", totalrows );

    if ( totalrows == 0 )
    {
        emit done ("");
        return;
    }

    // This returns the number of unread messages (JSON attribute "59": false)
    // Ideally it would be just:
    // update messages set jsonAttributes=(select json_replace( jsonAttributes, '$.59', true ) from messages) where json_extract( jsonAttributes, '$.59' ) = 0"
    // but sqlite would set the value of 59 to 1, instead of true. There seem to be no way to force it to true.
    if ( !stmt.prepare( sqlitedb, "SELECT id,jsonAttributes FROM messages WHERE json_extract( jsonAttributes, '$.59' ) = 0") )
    {
        emit done(tr("Cannot query database."));
        return;
    }

    int res;

    while ( (res = stmt.step()) != SQLITE_DONE )
    {
        // Update the progress bar
        currentrow++;

        emit progress( currentrow * 100 / totalrows );

        // Get the ID and attributes
        qint64 id = stmt.columnInt64( 0 );

        QJsonDocument document = QJsonDocument::fromJson( stmt.columnText( 1 ).toUtf8() );

        if ( !document.isObject() )
        {
            Log::debug("No JSON for id %lld, skipped", id );
            continue;
        }

        QJsonObject obj = document.object();
        Log::debug("Original JSON for %lld: %s",  id, qPrintable( document.toJson() ) );
        obj["59"] = true;

        SQLiteStatement ustmt;
        QStringList args;
        args << QJsonDocument( obj ).toJson() << QString::number(id);

        Log::debug("Modified JSON for %lld: %s",  id, qPrintable( QJsonDocument( obj ).toJson() ) );

        if ( !ustmt.prepare( sqlitedb, "UPDATE messages SET jsonAttributes=? WHERE id=?", args )
             || ustmt.step() != SQLITE_DONE )
        {
            Log::debug("Error updating JSON for message id %lld", id );
            continue;
        }

        Log::debug("Updated JSON for message id %lld", id );
    }

    emit done ("");
}
