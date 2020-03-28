////////////////////////////////////////////////////////////////////
///
///  This MorkParser has been modified by George Yunaev gyunaev@ulduzsoft.com
///  Copyright (C) George Yunaev 2018
///  The modifications switched the internal logic to use exceptions, and added
///  support for transactions.
///
///  The original copyright notice is remained below.
///
///   MorkParser.h - Mozilla Mork Format Parser/Reader 
///
///   Copyright (C) 2007 ScalingWeb.com
///   All rights reserved. 
/// 
///   Authors: Yuriy Soroka <ysoroka@scalingweb.com>
///	       Anton Fedoruk <afedoruk@scalingweb.com>
///
///
/// This program is free software; you can redistribute it and/or
/// modify it under the terms of the GNU General Public License
/// as published by the Free Software Foundation; either version 2
/// of the License, or (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
/// 
/// You should have received a copy of the GNU General Public License
/// along with this library; if not, write to the Free Software
/// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
/// 
///////////////////////////////////////////////////////////////////


#ifndef __MorkParser_h__
#define __MorkParser_h__

#include <QMap>
#include <QByteArray>
class QString;

// Types

typedef QMap< int, QString > MorkDict;
typedef QMap< int, int > MorkCells;					// ColumnId : ValueId
typedef QMap< int, MorkCells > MorkRowMap;			// Row id
typedef QMap< int, MorkRowMap > RowScopeMap;		// Row scope
typedef QMap< int, RowScopeMap > MorkTableMap;		// Table id
typedef QMap< int, MorkTableMap > TableScopeMap;	// Table Scope

// Mork header of supported format version
const char MorkMagicHeader[] = "// <!-- <mdb:mork:z v=\"1.4\"/> -->";

const char MorkDictColumnMeta[] = "<(a=c)>";

// Error codes
enum MorkErrors
{
    NoError = 0,
    FailedToOpen,
    UnsupportedVersion,
    DefectedFormat
};

// Mork term types
enum MorkTerm
{
    NoneTerm = 0,
    DictTerm,
    GroupTerm,
    TableTerm,
    RowTerm,
    CellTerm,
    CommentTerm,
    LiteralTerm
};


/// Class MorkParser

class MorkParser
{
public:

    MorkParser( int defaultScope = 0x80 );

    ///
    /// Open and parse mork file

    bool open( const QString &path );

    ///
    /// Return error status

    QString errorMsg();

    ///
    /// Returns all tables of specified scope

    MorkTableMap *getTables( int tableScope );

    ///
    /// Returns all rows under specified scope

    MorkRowMap *getRows( int rowScope, RowScopeMap *table );

    // Returns all rows for a specific table scope, table ID and row scope.
    // Return an empty map if not found
    const MorkRowMap * rows(int tablescope, int tableid, int rowscope );

    ///
    /// Return value of specified value oid

    QString getValue( int oid );

    ///
    /// Return value of specified column oid

    QString getColumn( int oid );

    static int dumpMorkFile( const QString& filename );

protected: // Members
    void    initVars();

    bool    isWhiteSpace( char c );
    char    nextChar();

    // Skips the sequence which must follow; throws exception if it does not
    void    skip( const char * string );

    // Reads the next char, increasing the counter. Throws if no next char.
    QChar   readNext();

    // Peeks the next char, which must exist (throws if not)
    QChar   peekNext();

    // Reads the hex number, until the first non-hex character
    QString readHexNumber();

    void    parseScopeId( const QString &TextId, int *Id, int *Scope );
    void    setCurrentRow( int TableScope, int TableId, int RowScope, int RowId );

    // Parse methods
    void    parse();
    void    parseDict();
    void    parseComment();
    void    parseCell(QList<int>* parsedIds = nullptr);
    void    parseTable();
    void    parseMeta( char c );
    void    parseRow( int TableId, int TableScope );
    void    parseGroup();

protected: // Data

    // Columns in mork means value names
    MorkDict columns_;
    MorkDict values_;

    // All mork file data
    TableScopeMap mork_;
    MorkCells *currentCells_;
    QMap<int, QMap<int, QPair<int, int>>> rowMappings;

    // Error status of last operation
    QString mErrorMessage;

    // All Mork data
    QByteArray morkData_;

    int morkPos_;
    int nextAddValueId_;
    int defaultScope_;

    // Indicates the entity that is being parsed
    enum { NPColumns, NPValues, NPRows } nowParsing_;
};


/**
 * A mork parse for mail databases.
 */
class MailMorkParser : public MorkParser {
public:
    /**
     * @return The number of unread emails in the mork file.
     */
    unsigned int getNumUnreadMessages();
};

#endif // __MorkParser_h__
