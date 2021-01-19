#pragma once

#include <QMap>
#include <QString>
#include <QtCore/QFileInfo>

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

        // Splits the string into stringlist using space as separator, but ignoring spaces inside quoted content
        static QStringList splitCommandLine( const QString& src );
        
        /**
         * Get the mail folder name from the mork path.
         *
         * @param morkPath The file info for a msf file.
         * @return The mail folder name or a null string.
         */
        static QString getMailFolderName(const QFileInfo &morkFile);
        
        /**
         * Get the mil account name from a mork path
         *
         * @param morkPath The file info for a msf file.
         * @return The mail account name or a null string.
         */
        static QString getMailAccountName(const QFileInfo &morkFile);
        
        /**
         * Format a Github markdown string:
         *
         * - Add links to the Github account pages for "@name" mentions.
         * - for old Qt versions without markdown support, change links
         *   from `[text](url)` to `text (url)`.
         *
         * @param markdown The markdown string.
         * @return A reference to the new markdown string with t the links.
         */
        static QString formatGithubMarkdown(const QString& markdown);
    
        /**
         * Convert a pixmap to a base64 encoded string.
         *
         * @param pixmap The pixmap to convert.
         * @return The base64 encoded image data.
         */
        static QString pixmapToString(const QPixmap &pixmap);

        /**
         * Convert a base64 encoded png into a pixmap.
         *
         * @param data The base64 encoded image data.
         * @return The pixmap representing the data.
         */
        static QPixmap pixmapFromString(const QString &data);
};


/**
 * A map that keeps track of the insertion order it's elements.
 *
 * @tparam Key The type of the map keys.
 * @tparam T The type of the element values.
 */
template<class Key, class T>
class OrderedMap {
public:
    
    /**
     * Access an element of the map by it's key or create a new entry.
     * @param key The key of the (new) element.
     * @return The value stored in the map for the given key.
     */
    T &operator[](const Key &key) {
        if (!storage.contains(key)) {
            order.push_back(key);
        }
        return storage[key];
    }
    
    /**
     * Access an element of the map by it's key.
     * @param key The key of the element.
     * @return The value stored in the map for the given key.
     */
    T operator[](const Key &key) const {
        return storage[key];
    }
    
    /**
     * @return The keys of all elements stored in the map in the order they were inserted.
     */
    const QList<Key> &orderedKeys() const {
        return order;
    }
    
    /**
     * Clear all elements from the map.
     */
    void clear() {
        storage.clear();
        order.clear();
    }
    
    /**
     * Remove an element from the map.
     *
     * @param key The key of the element to remove.
     */
    void remove(Key key) {
        storage.remove(key);
        order.removeOne(key);
    }
    
    /**
     * @return Whether or not the map is empty.
     */
    bool isEmpty() const {
        return storage.isEmpty();
    }

private:
    /**
     * A list storing the keys of the values in the map in insertion order.
     */
    QList<Key> order = {};
    
    /**
     * The actual map that sores the key - value pairs.
     */
    QMap<Key, T> storage = {};
};
