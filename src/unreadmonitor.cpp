#include <QTimer>
#include <QDir>

#include "unreadmonitor.h"
#include "morkparser.h"
#include "trayicon.h"
#include "utils.h"
#include "birdtrayapp.h"

UnreadMonitor::UnreadMonitor(TrayIcon* parent) : QThread(parent), changedMorkTimer(this) {
    mLastReportedUnread = 0;
    
    // Everything should be owned by our thread.
    moveToThread(this);
    
    // We get notification once Mork files have been modified.
    // This way we don't need to pull them too often.
    connect(&morkFilesWatcher, &QFileSystemWatcher::fileChanged, this,
            &UnreadMonitor::watchedFileChanges);
    connect(parent, &TrayIcon::settingsChanged, this, &UnreadMonitor::slotSettingsChanged);
    
    // Set up the watched file timer
    changedMorkTimer.setInterval(
            static_cast<int>(BirdtrayApp::get()->getSettings()->mWatchFileTimeout));
    changedMorkTimer.setSingleShot(true);
    
    connect(&changedMorkTimer, &QTimer::timeout, this, &UnreadMonitor::updateUnread);
}

void UnreadMonitor::run() {
    // Start it as soon as thread starts its event loop
    QTimer::singleShot(0, [=]() { updateUnread(); });
    // Start the event loop
    exec();
}

void UnreadMonitor::slotSettingsChanged() {
    mMorkUnreadCounts.clear();
    updateUnread();
}

void UnreadMonitor::watchedFileChanges(const QString &fileChanged) {
    if (!changedMorkFiles.contains(fileChanged)) {
        changedMorkFiles.push_back(fileChanged);
    }
    changedMorkTimer.start();
}

void UnreadMonitor::updateUnread() {
    Utils::debug("Triggering the unread counter update");
    // We execute a single statement and then parse the groups and decide on colors.
    QColor chosenColor;
    int total = static_cast<int>(getTotalUnreadCount(chosenColor));
    if (total != mLastReportedUnread || chosenColor != mLastColor) {
        emit unreadUpdated(total, chosenColor);
        mLastReportedUnread = total;
        mLastColor = chosenColor;
    }
}

unsigned int UnreadMonitor::getTotalUnreadCount(QColor &color) {
    unsigned int count = 0;
    Settings* settings = BirdtrayApp::get()->getSettings();
    bool rescanAll = false;
    
    // We rebuild and rescan the whole map if there is no such path in there, or map is empty (first run)
    if (mMorkUnreadCounts.isEmpty()) {
        rescanAll = true;
    } else {
        for (const QString &path : changedMorkFiles) {
            if (!mMorkUnreadCounts.contains(path)) {
                rescanAll = true;
                break;
            }
        }
    }
    
    if (rescanAll) {
        mMorkUnreadCounts.clear();
        for (const QString &path : settings->mFolderNotificationColors.keys()) {
            mMorkUnreadCounts[path] = getMorkUnreadCount(path);
            if (!morkFilesWatcher.files().contains(path) && !morkFilesWatcher.addPath(path)) {
                emit error(tr("Unable to watch %1 for changes.").arg(path));
            }
        }
    } else {
        for (const QString &path : changedMorkFiles) {
            mMorkUnreadCounts[path] = getMorkUnreadCount(path);
        }
    }
    
    // Find the total, and set the color
    QColor chosenColor;
    for (const QString &path : mMorkUnreadCounts.keys()) {
        if (mMorkUnreadCounts[path] > 0) {
            count += mMorkUnreadCounts[path];
            if (chosenColor.isValid()) {
                if (chosenColor != settings->mFolderNotificationColors[path]) {
                    chosenColor = settings->mNotificationDefaultColor;
                }
            } else {
                chosenColor = settings->mFolderNotificationColors[path];
            }
        }
    }
    color = chosenColor;
    changedMorkFiles.clear();
    return count;
}

unsigned int UnreadMonitor::getMorkUnreadCount(const QString &path) {
    MorkParser parser;
    
    if (!parser.open(path)) {
        return 0;
    }
    
    unsigned int unread = 0;
    
    // First we parse the unreadChildren column (generic view)
    const MorkRowMap* rows = parser.rows(0x80, 0, 0x80);
    
    if (rows) {
        for (MorkRowMap::const_iterator rit = rows->begin(); rit != rows->cend(); rit++) {
            MorkCells cells = rit.value();
            
            for (int colId : cells.keys()) {
                QString columnName = parser.getColumn(colId);
                
                if (columnName == "unreadChildren") {
                    bool correct;
                    unsigned int value = parser.getValue(cells[colId]).toUInt(&correct, 16);
                    
                    if (correct) {
                        unread += value;
                    } else {
                        Utils::debug("Incorrect Mork value: %s",
                                qPrintable(parser.getValue(cells[colId])));
                    }
                }
            }
        }
    } else {
        // Now parse the smart inbox
        rows = parser.rows(0x80, 0, 0x9F);
        
        if (rows) {
            for (MorkRowMap::const_iterator rowIterator = rows->begin();
                 rowIterator != rows->cend(); rowIterator++) {
                MorkCells cells = rowIterator.value();
                
                for (int colId : cells.keys()) {
                    QString columnName = parser.getColumn(colId);
                    
                    if (columnName == "numNewMsgs") {
                        bool correct;
                        unsigned int value = parser.getValue(cells[colId]).toInt(&correct, 16);
                        
                        if (correct) {
                            unread += value;
                        } else {
                            Utils::debug("Incorrect Mork value: %s",
                                    qPrintable(parser.getValue(cells[colId])));
                        }
                    }
                }
            }
        }
    }
    
    Utils::debug("Unread counter for %s: %d", qPrintable(path), unread);
    return unread;
}
