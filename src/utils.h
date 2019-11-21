#ifndef UTILS_H
#define UTILS_H

#include <QString>

class Utils
{
    public:
        // Decodes IMAP UTF7 data
        static QString  decodeIMAPutf7( const QString& param );
    
        /**
         * Expand the path as a shell would do. This will expand variables and ~/.
         * This function will also remove wrapping quotes.
         *
         * @param path The path.
         * @return The expanded path.
         */
        static QString expandPath(const QString& path);
        
        /**
         * @return The version of Birdtray as a string.
         */
        static QString getBirdtrayVersion();
        
        /**
         * @return The name of the Thunderbird updater executable.
         */
        static QString getThunderbirdUpdaterName();

        /**
         * Convert a std::wstring to a QString.
         *
         * @param str The std:wstring.
         * @return The QString with the contents of the original string.
         */
        static QString stdWToQString(const std::wstring &str);

        /**
         * Convert a QString to a std::wstring.
         *
         * @param str The QString.
         * @return The std::wstring with the contents of the original string.
         */
        static std::wstring qToStdWString(const QString &str);

        static void debug( const char * fmt, ... ) Q_ATTRIBUTE_FORMAT_PRINTF(1, 2);
        
        /**
         * Displays the message to the user and exits.
         *
         * @param message The message to display to the user.
         */
        Q_NORETURN static void fatal(const QString &message);

        /**
         * @return A list of possible locations of the directory
         *         that contains the Thunderbird profiles.
         * @see https://support.mozilla.org/en-US/kb/profiles-where-thunderbird-stores-user-data#w_profile-location-summary
         */
        static QStringList getThunderbirdProfilesPaths();

        /**
         * @return The default Thunderbird command for the current platform.
         */
        static QStringList getDefaultThunderbirdCommand();
};

#endif // UTILS_H
