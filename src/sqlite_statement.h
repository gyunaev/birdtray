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

#ifndef DATABASE_STATEMENT_H
#define DATABASE_STATEMENT_H

#include <QList>
#include <QByteArray>

struct sqlite3;
struct sqlite3_stmt;

// A SQLite statement wrapper ensuring finalize() is called at the end, and parsing important fields
class SQLiteStatement
{
    public:
        SQLiteStatement();
        ~SQLiteStatement();

        bool prepare( sqlite3 * db, const QString& sql, const QStringList& args = QStringList() );

        // Field access
        qint64 columnInt64( int column );
        int columnInt( int column );
        QString columnText( int column );

        // Single step
        int step();

    public:
        sqlite3_stmt * stmt;

        // Stores UTF-8 strings of args as sqlite needs them to live until the statement is executed
        QList<QByteArray>   m_args;
};

#endif // DATABASE_STATEMENT_H
