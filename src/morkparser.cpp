////////////////////////////////////////////////////////////////////
///
///   MorkParser.cpp - Mozilla Mork Format Parser/Reader implementation
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


#include "morkparser.h"
#include <QtCore>


//	=============================================================
//	MorkParser::MorkParser

MorkParser::MorkParser( int DefaultScope )
{
	initVars();
	defaultScope_ = DefaultScope;
}

//	=============================================================
//	MorkParser::open

bool MorkParser::open( const QString &path )
{
	initVars();

	QFile MorkFile( path );

	// Open file
    if ( !MorkFile.exists() || !MorkFile.open( QIODevice::ReadOnly ) )
	{
        mErrorMessage = "Couldn't open file: " + MorkFile.errorString();
		return false;
	}

	// Check magic header
	QByteArray MagicHeader = MorkFile.readLine();

	if ( !MagicHeader.contains( MorkMagicHeader ) )
	{
        mErrorMessage = "Unsupported version";
        return false;
    }

	morkData_ = MorkFile.readAll();
	MorkFile.close();

	// Parse mork
    try
    {
        parse();
        return true;
    }
    catch ( const char * errormsg )
    {
        mErrorMessage = errormsg;
        return false;
    }
}

//	=============================================================
//	MorkParser::error

inline QString MorkParser::errorMsg()
{
    return mErrorMessage;
}

//	=============================================================
//	MorkParser::initVars

void MorkParser::initVars()
{
	morkPos_ = 0;
	nowParsing_ = NPValues;
	currentCells_ = 0;
	nextAddValueId_ = 0x7fffffff;
}

//	=============================================================
//	MorkParser::parse

void MorkParser::parse()
{
	// Run over mork chars and parse each term
    char cur = nextChar();

	int i = 0;

    while ( cur )
	{
		if ( !isWhiteSpace( cur ) )
		{
			i++;
			// Figure out what a term
			switch ( cur )
			{
			case '<':
				// Dict
                parseDict();
				break;

            case '/':
				// Comment
                parseComment();
				break;

            case '{':
                parseTable();
				// Table
				break;

            case '[':
                parseRow( 0, 0 );
				// Row
				break;

            case '@':
                parseGroup();
				// Group
				break;

            default:
                throw "invalid format";
				break;
			}
		}

		// Get next char
		cur = nextChar();
	}
}

//	=============================================================
//	MorkParser::isWhiteSpace

bool MorkParser::isWhiteSpace( char c )
{
	switch ( c )
	{
	case ' ':
	case '\t':
	case '\r':
	case '\n':
	case '\f':
		return true;
	default:
		return false;
	}
}

//	=============================================================
//	MorkParser::nextChar

inline char MorkParser::nextChar()
{
	char cur = 0;

	if ( morkPos_ < morkData_.length() )
	{
		cur = morkData_[ morkPos_ ];
		morkPos_++;
	}

	if ( !cur )
	{
		cur = 0;
	}

	return cur;
}

void MorkParser::skip( const char * string )
{
    while ( *string )
    {
        if ( *string != nextChar() )
            throw "Parsing error";

        string++;
    }
}

QChar MorkParser::readNext()
{
    QChar ch = peekNext();
    morkPos_++;
    return ch;
}

QChar MorkParser::peekNext()
{
    if ( morkPos_ >= morkData_.length() )
        throw "Unexpected EOF";

    return QChar( morkData_[ morkPos_ ] );
}

QString MorkParser::readHexNumber()
{
    QString out;

    while ( true )
    {
        QChar ch = peekNext();

        if ( !ch.isLetterOrNumber() )
            return out;

        out.append( readNext() );
    }
}

//	=============================================================
//	MorkParser::parseDict

void MorkParser::parseDict()
{
	char cur = nextChar();
	nowParsing_ = NPValues;

    while ( cur != '>' && cur )
	{
		if ( !isWhiteSpace( cur ) )
		{
			switch ( cur )
			{
			case '<':
				if ( morkData_.mid( morkPos_ - 1, strlen( MorkDictColumnMeta ) ) 
					== MorkDictColumnMeta )
				{
					nowParsing_ = NPColumns;
					morkPos_ += strlen( MorkDictColumnMeta ) - 1;
				}
				break;
			case '(':
                parseCell();
				break;

			case '/':
                parseComment();
				break;

			}
		}

		cur = nextChar();
	}
}

//	=============================================================
//	MorkParser::parseComment

inline void MorkParser::parseComment()
{
	char cur = nextChar();

    if ( '/' != cur ) {
        throw "invalid comment";
    }

	while ( cur != '\r' && cur != '\n' && cur )
	{
		cur = nextChar();
	}
}

//	=============================================================
//	MorkParser::parseCell

void MorkParser::parseCell()
{
    //bool bColumnOid = false;
    bool bValueOid = false;
	bool bColumn = true;
	int Corners = 0;

	// Column = Value
    QString Column;
    QString Text;
	Column.reserve( 4 );
	Text.reserve( 32 );

	char cur = nextChar();

	// Process cell start with column (bColumn == true)
    while ( cur != ')' && cur )
	{
		switch ( cur )
		{
		case '^':
			// Oids
			Corners++;
			if ( 1 == Corners )
			{
                //bColumnOid = true;
			}
			else if ( 2 == Corners )
			{
				bColumn = false;
				bValueOid = true;
			}
			else
			{
				Text += cur;
			}

			break;
		case '=':
			// From column to value
			if ( bColumn )
			{
				bColumn = false;
			}
			else
			{
				Text += cur;
			}
			break;
		case '\\':
			{
				// Get next two chars
				char NextChar= nextChar();
				if ( '\r' != NextChar && '\n' != NextChar )
				{
					Text += NextChar;
				}
				else nextChar();
			}
			break;
		case '$':
			{
				// Get next two chars
                QString HexChar;
				HexChar += nextChar();
				HexChar += nextChar();
                Text += ( char ) HexChar.toInt( 0, 16 );
			}
			break;
		default:
			// Just a char
			if ( bColumn )
			{
				Column += cur;
			}
			else
			{
				Text += cur;
			}
			break;
		}

		cur = nextChar();
	}

	// Apply column and text
    int ColumnId = Column.toInt( 0, 16 );

	if ( NPRows != nowParsing_ )
	{
		// Dicts
		if ( "" != Text )
		{
			if ( nowParsing_ == NPColumns )
			{
				columns_[ ColumnId ] = Text;
			}
			else
			{
				values_[ ColumnId ] = Text;
			}
		}
	}
	else
	{
		if ( "" != Text )
		{
			// Rows
            int ValueId = Text.toInt( 0, 16 );

			if ( bValueOid  )
			{
				( *currentCells_ )[ ColumnId ] = ValueId;
			}
			else
			{
				nextAddValueId_--;
				values_[ nextAddValueId_ ] = Text;
				( *currentCells_ )[ ColumnId ] = nextAddValueId_;
			}
		}
	}
}

//	=============================================================
//	MorkParser::parseTable

void MorkParser::parseTable()
{
    QString TextId;
	int Id = 0, Scope = 0;

	char cur = nextChar();

	// Get id
	while ( cur != '{' && cur != '[' && cur != '}' && cur )
	{
		if ( !isWhiteSpace( cur ) )
		{
			TextId += cur;
		}

		cur = nextChar();
	}

	parseScopeId( TextId, &Id, &Scope );

	// Parse the table
    while ( cur != '}' && cur )
	{
		if ( !isWhiteSpace( cur ) )
		{
			switch ( cur )
			{
			case '{':
                parseMeta( '}' );
				break;

			case '[':
                parseRow( Id, Scope );
				break;

			case '-':
			case '+':
				break;

			default:
				{
                    QString JustId;
					while ( !isWhiteSpace( cur ) && cur )
					{
						JustId += cur;
						cur = nextChar();

						if ( cur == '}' )
						{
                            return;
						}
					}

					int JustIdNum = 0, JustScopeNum = 0;
					parseScopeId( JustId, &JustIdNum, &JustScopeNum );
					setCurrentRow( Scope, Id, JustScopeNum, JustIdNum );
				}
				break;
			}
		}

		cur = nextChar();
    }
}

//	=============================================================
//	MorkParser::parseScopeId

void MorkParser::parseScopeId(const QString &TextId, int *Id, int *Scope )
{
	int Pos = 0;

    if ( ( Pos = TextId.indexOf( ':' ) ) >= 0 )
	{
        QString tId = TextId.mid( 0, Pos );
        QString tSc = TextId.mid( Pos + 1, TextId.length() - Pos );

		if ( tSc.length() > 1 && '^' == tSc[ 0 ] )
		{
			// Delete '^'
            tSc.remove( 0, 1 );
		}

        *Id = tId.toInt( 0, 16 );
        *Scope = tSc.toInt( 0, 16 );
	}
	else
	{
        *Id = TextId.toInt( 0, 16 );
	}
}

//	=============================================================
//	MorkParser::setCurrentRow

inline void MorkParser::setCurrentRow( int TableScope, int TableId, int RowScope, int RowId )
{
	if ( !RowScope )
	{
		RowScope = defaultScope_;
	}

	if ( !TableScope )
	{
		TableScope = defaultScope_;
	}

	currentCells_ = &( mork_[ abs( TableScope ) ][ abs( TableId ) ][ abs( RowScope ) ][ abs( RowId ) ] );
}

//	=============================================================
//	MorkParser::parseRow

void MorkParser::parseRow( int TableId, int TableScope )
{
    QString TextId;
	int Id = 0, Scope = 0;
	nowParsing_ = NPRows;

	char cur = nextChar();

	// Get id
	while ( cur != '(' && cur != ']' && cur != '[' && cur )
	{
		if ( !isWhiteSpace( cur ) )
		{
			TextId += cur;
		}

		cur = nextChar();
	}

	parseScopeId( TextId, &Id, &Scope );
	setCurrentRow( TableScope, TableId, Scope, Id );

	// Parse the row
    while ( cur != ']' && cur )
	{
		if ( !isWhiteSpace( cur ) )
		{
			switch ( cur )
			{
			case '(':
                parseCell();
				break;
			case '[':
                parseMeta( ']' );
				break;
			default:
                throw "format error";
				break;
			}
		}

		cur = nextChar();
	}
}

//	=============================================================
//	MorkParser::parseGroup

void MorkParser::parseGroup()
{
    // See https://developer.mozilla.org/en-US/docs/Mozilla/Tech/Mork/Structure
    // In version 1.4, @ is always followed by $$ as part of the group markup syntax
    skip( "$${" );

    // Group ID
    QString id = readHexNumber();

    // Skip transaction start
    skip( "{@" );

    // From here we have the whole transaction. Find out the transaction end
    QString endCommit = "@$$}" + id + "}@";
    QString endAbort = "@$$}~abort~" + id + "}@";

    // Find the end of this group
    int ofst = morkData_.indexOf( endAbort, morkPos_ );

    if ( ofst != -1 )
    {
        // Transaction aborted (does this ever happen?)
        morkPos_ = ofst + endAbort.length();
        return;
    }

    // Now look up for success
    ofst = morkData_.indexOf( endCommit, morkPos_ );

    if ( ofst == -1 )
        throw "Unexpected end of group";

    // Transaction succeeded. Copy the transaction data
    QByteArray transaction = morkData_.mid( morkPos_, ofst - morkPos_ );

    // Now we want to reuse the parse() routine, so we store the old data
    if ( !transaction.trimmed().isEmpty() )
    {
        QByteArray oldmork = morkData_;

        // and replace it with transaction
        morkData_ = transaction;
        morkPos_ = 0;

        // Parse it
        parse();

        // And restore the old values back
        morkData_ = oldmork;
    }

    morkPos_ = ofst + endCommit.length();
}

//	=============================================================
//	MorkParser::parseMeta

void MorkParser::parseMeta( char c )
{
	char cur = nextChar();

	while ( cur != c && cur )
	{
		cur = nextChar();
	}
}

//	=============================================================
//	MorkParser::getTables

MorkTableMap *MorkParser::getTables( int TableScope )
{
	TableScopeMap::iterator iter;
	iter = mork_.find( TableScope );

	if ( iter == mork_.end() )
	{
		return 0;
	}

	return &iter.value();
}

//	=============================================================
//	MorkParser::getRows

MorkRowMap *MorkParser::getRows( int RowScope, RowScopeMap *table )
{
	RowScopeMap::iterator iter;
	iter = table->find( RowScope );

	if ( iter == table->end() )
	{
		return 0;
	}

	return &iter.value();
}

//	=============================================================
//	MorkParser::getValue

QString MorkParser::getValue( int oid )
{
	MorkDict::iterator foundIter = values_.find( oid );

	if ( values_.end() == foundIter )
	{
        return QString();
	}

	return *foundIter;
}

//	=============================================================
//	MorkParser::getColumn

QString MorkParser::getColumn( int oid )
{
	MorkDict::iterator foundIter = columns_.find( oid );

	if ( columns_.end() == foundIter )
	{
        return QString();
	}

	return *foundIter;
}
