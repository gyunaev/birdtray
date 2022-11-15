#include <QTimer>
#include <QDir>

#include "unreadmonitor.h"
#include "morkparser.h"
#include "trayicon.h"
#include "log.h"
#include "birdtrayapp.h"

UnreadMonitor::UnreadMonitor( TrayIcon * parent )
    : QThread( 0 ), mChangedMSFtimer(this), mForceUpdateTimer(this)
{
    moveToThread( this );
    mLastReportedUnread = 0;

    // We get notification once Mork files have been modified.
    connect( &mDBWatcher, &QFileSystemWatcher::fileChanged, this, &UnreadMonitor::watchedFileChanges );

    // Settings changed
    connect( parent, &TrayIcon::settingsChanged, this, &UnreadMonitor::slotSettingsChanged );

    // Set up the watched file timer
    mChangedMSFtimer.setInterval(BirdtrayApp::get()->getSettings()->mWatchFileTimeout);
    mChangedMSFtimer.setSingleShot( true );

    connect( &mChangedMSFtimer, &QTimer::timeout, this, &UnreadMonitor::updateUnread );

    // Set up the forced update timer
    mForceUpdateTimer.setSingleShot( false );
    connect( &mForceUpdateTimer, &QTimer::timeout, this, &UnreadMonitor::forceUpdateUnread );
}

void UnreadMonitor::run()
{
    // Start it as soon as thread starts its event loop
    QTimer::singleShot( 0, [=](){ updateUnread(); } );
    
    // And activate the forced update timer if settings specify so
    unsigned int rereadIntervalSec = BirdtrayApp::get()->getSettings()->mIndexFilesRereadIntervalSec;
    if ( rereadIntervalSec > 0 )
    {
        mForceUpdateTimer.setInterval( static_cast<int>(rereadIntervalSec * 1000) );
        mForceUpdateTimer.start();
    }

    // Start the event loop
    exec();
}

const QMap<QString, QString> &UnreadMonitor::getWarnings() const {
    return warnings;
}

void UnreadMonitor::slotSettingsChanged()
{
    Settings* settings = BirdtrayApp::get()->getSettings();
    // And activate it if settings specify so
    if ( settings->mIndexFilesRereadIntervalSec > 0 )
    {
        mForceUpdateTimer.setInterval( settings->mIndexFilesRereadIntervalSec * 1000 );
        mForceUpdateTimer.start();
    }
    else
        mForceUpdateTimer.stop();

    // We reinitialize everything because the settings changed
    mMorkUnreadCounts.clear();

    const QStringList &accountsList = settings->watchedMorkFiles.orderedKeys();
    for (const QString &path : warnings.keys()) {
        if (!accountsList.contains(path)) {
            clearWarning(path);
        }
    }
    updateUnread();
}

void UnreadMonitor::watchedFileChanges(const QString &filechanged)
{
    if ( !mChangedMSFfiles.contains( filechanged ) )
        mChangedMSFfiles.push_back( filechanged );

    mChangedMSFtimer.start();
}

void UnreadMonitor::updateUnread()
{
    Log::debug("Triggering the unread counter update");

    // We execute a single statement and then parse the groups and decide on colors.
    QColor chosenColor;
    int total = 0;

    getUnreadCount_Mork(total, chosenColor);

    if ( total != mLastReportedUnread || chosenColor != mLastColor )
    {
        emit unreadUpdated( total, chosenColor );
        mLastReportedUnread = total;
        mLastColor = chosenColor;
    }
}

void UnreadMonitor::forceUpdateUnread()
{
    mChangedMSFfiles = BirdtrayApp::get()->getSettings()->watchedMorkFiles.orderedKeys();
    updateUnread();
}

void UnreadMonitor::getUnreadCount_Mork(int &count, QColor &color)
{
    Settings* settings = BirdtrayApp::get()->getSettings();
    bool rescanall = false;

    // We rebuild and rescan the whole map if there is no such path in there, or map is empty (first run)
    if ( mMorkUnreadCounts.isEmpty() )
        rescanall = true;
    else
    {
        for ( QString path : mChangedMSFfiles )
            if ( !mMorkUnreadCounts.contains( path ) )
                rescanall = true;
    }

    if ( rescanall )
    {
        mMorkUnreadCounts.clear();
        for (const QString &path : settings->watchedMorkFiles.orderedKeys()) {
            mMorkUnreadCounts[path] = getMorkUnreadCount(path);
            if (!mDBWatcher.files().contains(path) && !mDBWatcher.addPath(path)) {
                setWarning(tr("Unable to watch %1 for changes.")
                        .arg(QFileInfo(path).fileName()), path);
            } else {
                clearWarning(path);
            }
        }
    }
    else
    {
        for ( QString path : mChangedMSFfiles )
            mMorkUnreadCounts[ path ] = getMorkUnreadCount( path );
    }

    // Find the total, and set the color
    QColor chosenColor;
    for ( const QString& tpath : mMorkUnreadCounts.keys() )
    {
        if ( mMorkUnreadCounts[ tpath ] > 0 )
        {
            count += mMorkUnreadCounts[ tpath ];

            if ( chosenColor.isValid() ) {
                if (chosenColor != settings->watchedMorkFiles[tpath]) {
                    chosenColor = settings->mNotificationDefaultColor;
                }
            } else {
                chosenColor = settings->watchedMorkFiles[tpath];
            }
        }
    }
    color = chosenColor;
    mChangedMSFfiles.clear();
}

int UnreadMonitor::getMorkUnreadCount(const QString &path)
{
    MailMorkParser parser;
    if (!parser.open(path)) {
        Log::debug("Unable to parser mork file %s: %s", qPrintable( path ), qPrintable(parser.errorMsg()));
        setWarning(tr("Unable to read from %1.").arg(QFileInfo(path).fileName()), path);
        return 0;
    } else {
        clearWarning(path);
    }
    int unread = static_cast<int>(parser.getNumUnreadMessages());
    Log::debug("Unread counter for %s: %d", qPrintable( path ), unread );
    return unread;
}

void UnreadMonitor::setWarning(const QString &message, const QString &path) {
    if (warnings.value(path) != message) {
        warnings.insert(path, message);
        emit warningChanged(path);
    }
}

void UnreadMonitor::clearWarning(const QString &path) {
    if (warnings.remove(path) > 0) {
        emit warningChanged(path);
    }
}
