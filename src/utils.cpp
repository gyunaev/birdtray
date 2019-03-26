#include "utils.h"
#include "settings.h"

#include <QTextCodec>
#include <QtWidgets/QMessageBox>

#if defined (Q_OS_WIN)
#  include <windows.h>
#elif defined (Q_OS_MAC)
#else
#  include <wordexp.h>
#endif


QString Utils::decodeIMAPutf7(const QString &param)
{
    // See https://tools.ietf.org/html/rfc2060#page-13
    //    By convention, international mailbox names are specified using a
    //    modified version of the UTF-7 encoding described in [UTF-7].  The
    //    purpose of these modifications is to correct the following problems
    //    with UTF-7:
    //
    //          1) UTF-7 uses the "+" character for shifting; this conflicts with
    //             the common use of "+" in mailbox names, in particular USENET
    //             newsgroup names.
    //
    //          2) UTF-7's encoding is BASE64 which uses the "/" character; this
    //             conflicts with the use of "/" as a popular hierarchy delimiter.
    //
    //          3) UTF-7 prohibits the unencoded usage of "\"; this conflicts with
    //             the use of "\" as a popular hierarchy delimiter.
    //
    //          4) UTF-7 prohibits the unencoded usage of "~"; this conflicts with
    //             the use of "~" in some servers as a home directory indicator.
    //
    //          5) UTF-7 permits multiple alternate forms to represent the same
    //             string; in particular, printable US-ASCII chararacters can be
    //             represented in encoded form.
    //
    //    In modified UTF-7, printable US-ASCII characters except for "&"
    //    represent themselves; that is, characters with octet values 0x20-0x25
    //    and 0x27-0x7e.  The character "&" (0x26) is represented by the two-
    //    octet sequence "&-".
    //
    //    All other characters (octet values 0x00-0x1f, 0x7f-0xff, and all
    //    Unicode 16-bit octets) are represented in modified BASE64, with a
    //    further modification from [UTF-7] that "," is used instead of "/".
    //    Modified BASE64 MUST NOT be used to represent any printing US-ASCII
    //    character which can represent itself.
    //
    //    "&" is used to shift to modified BASE64 and "-" to shift back to US-
    //    ASCII.  All names start in US-ASCII, and MUST end in US-ASCII (that
    //    is, a name that ends with a Unicode 16-bit octet MUST end with a "-
    //    ").
    QString out;
    QString decodebuf;
    bool decoding = false;
    QTextCodec * codec = QTextCodec::codecForName("UTF16-BE");

    // This is extremely unlikely but still...
    if ( !codec )
        return "ERROR1-" + param;

    for ( int i = 0; i < param.length(); i++ )
    {
        // Are we already decoding?
        if ( decoding )
        {
            if ( param[i] == '-' )
            {
                // Decode this string as modified UTF7, which is UTF16BE encoded in base64
                QByteArray utf16data = QByteArray::fromBase64( qPrintable( decodebuf ) );

                if ( utf16data.isEmpty() )
                {
                    // Print the warning, and return an error
                    qWarning("Invalid IMAP UTF7 sequence: '%s' contains invalid base64 '%s' - please report", qPrintable(param), qPrintable(decodebuf) );
                    return "ERROR2-" + param;
                }

                // Decode it as UTF16
                out += codec->toUnicode( utf16data );

                // And reset the remaining
                decodebuf.clear();
                decoding = false;
            }
            else
            {
                // This is modified BASE64 where "," is used instead of "/"
                if ( param[i] == ',' )
                    decodebuf.append( '/' );
                else
                    decodebuf.append( param[i] );
            }
        }
        else
        {
            // We are not decoding yet; & indicates possible start of decoding
            if ( param[i] == '&' )
            {
                // this is either start of decoding or &-
                if ( i  + 2 < param.length() && param[i+1] == '-' )
                {
                    // This is "&-" combination which means &, and does not start decoding
                    i++;
                    out.append( '&' );
                }
                else
                    decoding = true;
            }
            else
                out.append( param[i] );
        }
    }

    // The string MUST end with '-' and thus there should be nothing in the decodebuf
    if ( !decodebuf.isEmpty() )
        qWarning("Invalid IMAP UTF7 sequence: '%s' may be decoded incorrectly", qPrintable(param) );

    return out;
}

QString Utils::expandPath(const QString &path) {
#if defined (Q_OS_WIN)
    TCHAR buffer[PATH_MAX];
#if defined(UNICODE)
    std::wstring originalPath = path.toStdWString();
#else
    std::string originalPath = path.toStdWString();
#endif /* defined(UNICODE) */
    DWORD resultSize = ExpandEnvironmentStrings(
            originalPath.data(), buffer, sizeof(buffer) / sizeof(buffer[0]));
    if (resultSize == 0 || resultSize > sizeof(buffer) / sizeof(buffer[0])) {
        return path;
    }
#if defined(UNICODE)
    return QString::fromWCharArray(buffer, static_cast<int>(resultSize - 1));
#else
    return QString(buffer);
#endif /* defined(UNICODE) */

#elif defined (Q_OS_MAC)
    return path;
    
#else
    wordexp_t result;
    if (wordexp(path.toStdString().data(), &result, WRDE_NOCMD) != 0) {
        return path;
    }
    QString expandedPath(result.we_wordv[0]);
    wordfree(&result);
    return expandedPath;
#endif
}

void Utils::debug(const char *fmt, ...)
{
    va_list vl;
    char buf[8192];

    if ( !pSettings->mVerboseOutput )
        return;

    va_start( vl, fmt );
    vsnprintf( buf, sizeof(buf) - 1, fmt, vl );
    va_end( vl );

    qDebug( "%s", buf );
}


void Utils::fatal(const char *fmt, ...)
{
    va_list vl;
    char buf[8192];

    va_start( vl, fmt );
    vsnprintf( buf, sizeof(buf) - 1, fmt, vl );
    va_end( vl );

    QMessageBox::critical(nullptr, "Fatal", buf);
    qFatal("%s", buf);
}
