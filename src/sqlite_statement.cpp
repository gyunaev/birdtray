/**************************************************************************
 *  Spivak Karaoke PLayer - a free, cross-platform desktop karaoke player *
 *  Copyright (C) 2015-2016 George Yunaev, support@ulduzsoft.com          *
 *                                                                        *
 *  This program is free software: you can redistribute it and/or modify  *
 *  it under the terms of the GNU General Public License as published by  *
 *  the Free Software Foundation, either version 3 of the License, or     *
 *  (at your option) any later version.                                   *
 *																	      *
 *  This program is distributed in the hope that it will be useful,       *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *  GNU General Public License for more details.                          *
 *                                                                        *
 *  You should have received a copy of the GNU General Public License     *
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 **************************************************************************/

#include <QThread>

#include <sqlite3.h>

#include "settings.h"
#include "sqlite_statement.h"


SQLiteStatement::SQLiteStatement()
{
    stmt = 0;
}

SQLiteStatement::~SQLiteStatement()
{
    if ( stmt )
        sqlite3_finalize( stmt );
}

bool SQLiteStatement::prepare(sqlite3 *db, const QString &sql, const QStringList &args)
{
    // Prepare the sql
    if ( sqlite3_prepare_v2( db, qPrintable(sql), -1, &stmt, 0 ) != SQLITE_OK )
        return false;

    // Bind values if there are any
    for ( int i = 0; i < args.size(); i++ )
    {
        m_args.push_back( args[i].toUtf8() );
        if ( sqlite3_bind_text( stmt, i + 1, m_args.last().data(), -1, 0 ) != SQLITE_OK )
            return false;
    }

    return true;
}

qint64 SQLiteStatement::columnInt64(int column)
{
    return sqlite3_column_int64( stmt, column );
}

int SQLiteStatement::columnInt(int column)
{
    return sqlite3_column_int( stmt, column );
}

QString SQLiteStatement::columnText(int column)
{
    return QString::fromUtf8( (const char*) sqlite3_column_text( stmt, column ), sqlite3_column_bytes( stmt, column ) );
}

int SQLiteStatement::step()
{
    while ( true )
    {
        int res = sqlite3_step( stmt );

        if ( res == SQLITE_BUSY)
        {
            QThread::msleep( 100 );
            continue;
        }

        return res;
    }
}
