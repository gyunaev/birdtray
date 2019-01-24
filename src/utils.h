#ifndef UTILS_H
#define UTILS_H

#include <QString>

class Utils
{
    public:
        // Decodes IMAP UTF7 data
        static QString  decodeIMAPutf7( const QString& param );
};

#endif // UTILS_H
