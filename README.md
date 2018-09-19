# Birdtray is a system tray new mail notification for Thunderbird. It supports Thunderbird 60+ and does not require installing extensions.

Birdtray is a free, cross-platform (Linux/Windows/OS X) system tray notification for new mail for Thunderbird. It checks the e-mail status directly using the Thunderbird email search database, and thus does not need any extensions.

This is the alpha version, so please report any issues.


## Compiling

Currently the only way to test Birdtray is to build it from source, which should be easy on Linux, but may be cumbersome on other OSes. You would need the following libraries:

- Qt 5.5 (might work on prior versions too - this means the code may need changes which are doable - but will NOT work on Qt4);
- sqlite3


To build, please do the following:

    cd src
    qmake (or qmake-qt5)
    make
    
Launch the ./birdtray executable from the local directory. It will show Thunderbird icon in system tray.

Right-click on this icon, and click Settings. Select the Thunderbird profile folder - must contain the file "global-messages-db.sqlite".
Then select the font and default color (which will be used if more than one monitored folder has new mail).

Then click on Accounts tab, and add one or more folders. You can specify different notification colors for each folder. Birdtray will show the new email count using this color if only this folder has new mail. If more than one folder has new mail, the default color will be used.

Because Thunderbird updates database after 5-10 seconds when the emails are read or new emails arrived, there is a delay between a new email is arrived and birdtray "sees" it. This is a known issue which currently I do not know how to address.

## Submitting bugs and feature requests

Please use Github issue tracker.

## Author and license

Birdtray is written by George Yunaev, and is licensed under GPLv3 license.
