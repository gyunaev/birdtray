# Birdtray is a system tray new mail notification for Thunderbird, which does not require extensions.[![Build Status](https://travis-ci.org/gyunaev/birdtray.svg?branch=master)](https://travis-ci.org/gyunaev/birdtray)

Birdtray is a free system tray notification for new mail for Thunderbird. It supports Linux and Windows (credit for adding and maintaining Windows support goes to @Abestanis). Patches to support other platforms are welcome.

For those of you using Thunderbird 68+, the sqlite-based parser will no longer work. Please switch to Mork parser.

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

To build Birdtray from source, you would need the following components:

- A C++ compiler
- Cmake
- Qt 5.6 or higher with "x11extras-dev" or "x11extras-devel"  module installed (it is usually NOT installed by default);
- sqlite3 (i.e. libsqlite3-dev or libsqlite3-devel)

On Debian you need to install the following packages: ``qt5-default libsqlite3-dev libqt5x11extras5-dev qttools5-dev``

On OpenSuSE you need to install ``libqt5-qtbase-devel libqt5-qtx11extras-devel sqlite3-devel``

On Fedora (30 or later) you need to install ``libsqlite3x-devel qt5-qtbase-devel qt5-qtx11extras-devel``

To build, please do the following:

```shell script
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

Launch the `./birdtray` executable from the build directory.

## Installation

Run `cmake --build . --target install` to install Birdtray.
On Unix systems, you can configure the install location by running
`cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr ..` before the command above.
On Windows, the command will build a graphical installer and execute it.
It requires [NSIS](https://nsis.sourceforge.io/Main_Page) to be installed on your system.
It is recommended for Windows users to use the
[precompiled installers for the latest release](https://github.com/gyunaev/birdtray/releases/latest).  

## Usage

Once started, Birdtray will show the Thunderbird icon in system tray.

Right-click on this icon, and click Settings. Go to Monitoring tab ans select the Thunderbird MSF file for the mailbox you'd like to monitor. You can specify different notification colors for each mailbox. Birdtray will show the new email count using this color if only this folder has new mail. If more than one folder has new mail, the default color will be used.

Then select the font and default color (which will be used if more than one monitored folder has new mail).

You can also enable birdtray to start Thunderbird when you start Birdtray, or enable show/hide Thunderbird when the system tray icon is clicked, in settings.

Once you change settings, often you need to restart birdtray for the new settings to take effect.

All birdtray settings are stored on a per-user basis in `$HOME/.config/ulduzsoft/birdtray.conf`.

## Troubleshooting

If you have lots of unread messages shown, and you are using global search database to look for unread messages, it may be because the database is corrupt or too old. You may delete the file global-messages-db.sqlite and restart Thunderbird which would rebuild this file. This will also help if "search" function in Thunderbird finds emails which no longer exist.

## Submitting bugs and feature requests

Please use Github issue tracker.

### Translations

Translations are maintained by the community.

###### Language tag

All translation are connected to their language via the language tag of that language.
The language tag consists of the [ISO 639‑1 code](https://en.wikipedia.org/wiki/List_of_ISO_639-1_codes)
and optionally the [ISO 3166-1 alpha-2 country code](https://en.wikipedia.org/wiki/ISO_3166-1_alpha-2#Officially_assigned_code_elements)
joined with a `_` (e.g. `en` for english or `en_US` for american english).

###### Adding a new translation:

To add a new translation to Birdtray, you need to know the [language tag](#language-tag) for the language you want to add.
Choose one `main_*.ts`, a `dynamic_*.ts` and a `installer_*.ts` file from [here](src/translations), where `*` is another language tag.
Copy the files into the same directory, but replace the previous language tag with the tag for your language.
Now open both files with a text editor and find the line near the top that looks like this: `<TS version="2.1" language="*">`
Again, `*` will be another language tag. Replace that tag with your new language tag.
Now you can continue to edit the new files as described in the next section.

###### Edit an existing translation:

To change an existing translation, you need to edit the `main_*.ts`, `dynamic_*.ts` and / or `installer_*.ts` translation file
of that language in the [translations directory](src/translations).
The `*` represents the [language tag](#language-tag) of the language you want to change. To edit these files,
you can use a text editor or the [Qt Linguist](https://doc.qt.io/qt-5/linguist-translators.html).

Once you done, create a pull request with your changes to [this repository](https://github.com/gyunaev/birdtray).

## Author and license

Birdtray is written by George Yunaev, and is licensed under GPLv3 license.

![birdtray-settings](screenshots/birdtray-settings.png)
