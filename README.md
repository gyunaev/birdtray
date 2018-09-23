# Birdtray is a system tray new mail notification for Thunderbird 60+ which does not require extensions.

Birdtray is a free, cross-platform (Linux/Windows/OS X) system tray notification for new mail for Thunderbird. 

The latest release is 0.2 at Sep 22, 2018.

This is the alpha version, so please report any issues.

## Features

- Shows the unread email counter in the Thunderbird system tray icon;

- Optionally can animate the Thunderbird system tray icon if new mail is received;

- You can snooze new mail notifications for a specific time period;

- Birdtray checks the unread e-mail status directly by reading the Thunderbird email search database. This means it does not need any extensions, and thus is immune to any future extension API changes in Thunderbird;

- Starting from version 0.2 if you click on Birdtray icon, it can hide the Thunderbird window, and restore it. There is also context menu for that (this currently only works on Linux);

- You can configure which accounts you want to check for unread emails on;

- You can choose different font colors for different email accounts. This allows you, for example, to have blue unread count for personal emails, red unread count for work emails, and green unread count if both folders have unread mail.



## Compiling

Currently the only way to test Birdtray is to build it from source, which should be easy on Linux, but may be cumbersome on other OSes. You would need the following libraries:

- Qt 5.5 or higher;
- sqlite3


To build, please do the following:

    cd src
    qmake (or qmake-qt5)
    make
    
Launch the ./birdtray executable from the local directory. It will show Thunderbird icon in system tray.

Right-click on this icon, and click Settings. Select the Thunderbird profile folder - must contain the file "global-messages-db.sqlite".
Then select the font and default color (which will be used if more than one monitored folder has new mail).

Then click on Accounts tab, and add one or more folders. You can specify different notification colors for each folder. Birdtray will show the new email count using this color if only this folder has new mail. If more than one folder has new mail, the default color will be used.

You can also enable birdtray to start Thunderbird when you start Birdtray, or enable show/hide Thunderbird when the system tray icon is clicked, in settings.

Once you change settings, you must restart birdtray for the new settings to take effect.

Because Thunderbird updates database after 5-10 seconds when the emails are read or new emails arrived, there is a delay between a new email is arrived and birdtray "sees" it. This is a known issue which currently I do not know how to address.

If you have no unread email, but Birdtray still shows there is unread email, this is caused by Thunderbird not properly updating the messages database. Use the "Fix" button in Settings dialog to fix that.

## Submitting bugs and feature requests

Please use Github issue tracker.

## Author and license

Birdtray is written by George Yunaev, and is licensed under GPLv3 license.
