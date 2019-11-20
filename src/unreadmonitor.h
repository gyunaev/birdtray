#ifndef UNREAD_MONITOR_H
#define UNREAD_MONITOR_H

#include <QThread>
#include <QString>
#include <QColor>
#include <QMap>
#include <QTimer>
#include <QStringList>
#include <QFileSystemWatcher>

class TrayIcon;

class UnreadMonitor : public QThread {
Q_OBJECT

public:
    explicit UnreadMonitor(TrayIcon* parent);
    
    // Thread run function
    void run() override;
    
    /**
     * Safely enqueues the this unread monitor for deletion.
     */
    void quitAndDelete();

signals:
    
    // Unread counter changed
    void unreadUpdated(unsigned int total, QColor color);
    
    // An error happened
    void error(QString message);

public slots:
    
    void slotSettingsChanged();
    
    void watchedFileChanges(const QString &fileChanged);
    
    void updateUnread();

private:
    unsigned int getTotalUnreadCount(QColor &color);
    
    static unsigned int getMorkUnreadCount(const QString &path);

private:
    // Maps the Mork files to unread counts
    QMap<QString, quint32> mMorkUnreadCounts;
    
    // Watches the files for changes
    QFileSystemWatcher morkFilesWatcher;
    
    // Thunderbird tends to do lots of modifications to the MSF file
    // each time a new email arrives. This results in lots of notifications,
    // and thus lots of unread calls. To avoid this, we set up the timer each
    // time a new notification is issued, and reset it for each further notification.
    // We only read the file(s) if the timer expires.
    QTimer changedMorkTimer;
    
    // List of changed mork files
    QStringList changedMorkFiles;
    
    // Last reported unread
    int mLastReportedUnread;
    QColor mLastColor;
};

#endif /* UNREAD_MONITOR_H */
