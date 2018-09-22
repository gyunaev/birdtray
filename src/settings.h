#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSize>
#include <QFont>
#include <QColor>
#include <QMap>
#include <QJsonObject>

class Settings
{
    public:
        Settings();

        // Parameters
        QSize   mIconSize;
        QFont   mTextFont;
        QColor  mTextColor;
        unsigned int    mBlinkSpeed;

        // Path to Thunderbird folder
        QString mThunderbirdFolderPath;

        // Thunderbird binary and command line
        QString mThunderbirdCmdLine;

        // Thunderbird window match
        QString mThunderbirdWindowMatch;

        // Whether to show/hide Thunderbird on button click
        bool    mShowHideThunderbird;

        // Whether to launch Thunderbird when the app starts
        bool    mLaunchThunderbird;

        // Maps the folder URI to the notification color
        QMap< QString, QColor >   mFolderNotificationColors;



        // Load and save them
        void    save();
        void    load();

    private:
};

extern Settings * pSettings;

#endif // SETTINGS_H
