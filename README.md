# Birdtray is a system tray new mail notification for Thunderbird 60+ which does not require extensions. [![Build Status](https://travis-ci.org/gyunaev/birdtray.svg?branch=master)](https://travis-ci.org/gyunaev/birdtray)

Birdtray is a free system tray notification for new mail for Thunderbird. It supports Linux and Windows (credit for adding and maintaining Windows support goes to @Abestanis). Patches to support other platforms are welcome.

## Features

- Shows the unread email counter in the Thunderbird system tray icon;

- Optionally can animate the Thunderbird system tray icon if new mail is received;

- You can snooze new mail notifications for a specific time period;

- Birdtray checks the unread e-mail status directly by reading the Thunderbird email search database. This means it does not need any extensions, and thus is immune to any future extension API changes in Thunderbird;

- Starting from version 0.2 if you click on Birdtray icon, it can hide the Thunderbird window, and restore it. There is also context menu for that (this currently only works on Linux);

- You can configure which accounts you want to check for unread emails on;

- You can choose different font colors for different email accounts. This allows you, for example, to have blue unread count for personal emails, red unread count for work emails, and green unread count if both folders have unread mail.

- Can launch Thunderbird when Birdtray starts, and terminate it when Birdtray quits (configurable).

- You can choose the tray icon, or use Thunderbird original icon;

- Can monitor that Thunderbird is running, and indicate it if you accidentally closed it;

- Has configurable "New Email" functionality, allowing pre-configured email templates.


## Building

To build Birdtray from source, you would need the following libraries:

- Qt 5.5 or higher with "x11extras-dev" or "x11extras-devel"  module installed (it is usually NOT installed by default);
- sqlite3 (i.e. libsqlite3-dev or libsqlite3-devel)

On Debian you need to install the following packages: ``qt5-defaults libsqlite3-dev libqt5x11extras-dev``

On OpenSuSE you need to install ``libqt5-qtbase-devel libqt5-qtx11extras-devel sqlite3-devel``


To build, please do the following:

    cd src
    qmake (or qmake-qt5)
    make
    
Launch the ./birdtray executable from the local directory. It will show Thunderbird icon in system tray.

Right-click on this icon, and click Settings. Go to Monitoring tab ans select the Thunderbird MSF file for the mailbox you'd like to monitor. You can specify different notification colors for each mailbox. Birdtray will show the new email count using this color if only this folder has new mail. If more than one folder has new mail, the default color will be used.

Then select the font and default color (which will be used if more than one monitored folder has new mail).

You can also enable birdtray to start Thunderbird when you start Birdtray, or enable show/hide Thunderbird when the system tray icon is clicked, in settings.

Once you change settings, often you need to restart birdtray for the new settings to take effect.

## Troubleshooting

If you have lots of unread messages shown, and you are using global search database to look for unread messages, it may be because the database is corrupt or too old. You may delete the file global-messages-db.sqlite and restart Thunderbird which would rebuild this file. This will also help if "search" function in Thunderbird finds emails which no longer exist.

## Submitting bugs and feature requests

Please use Github issue tracker.

## Author and license

Birdtray is written by George Yunaev, and is licensed under GPLv3 license.
