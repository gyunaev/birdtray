#ifndef UTILS_H
#define UTILS_H

#include <QString>

class Utils
{
    public:
        // Decodes IMAP UTF7 data
        static QString  decodeIMAPutf7( const QString& param );
    
        /**
         * Expand the path as a shell would do. This will expand variables and ~/
         *
         * @param path The path.
         * @return The expanded path.
         */
        static QString expandPath(const QString& path);

        static void debug( const char * fmt, ... );
};

#endif // UTILS_H
