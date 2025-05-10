#include <QMenu>
#include <QTimer>
#include <QPainter>
#include <QProcess>
#include <QMessageBox>
#include <QPainterPath>

#include "trayicon.h"
#include "unreadmonitor.h"
#include "windowtools.h"
#include "utils.h"
#include "autoupdater.h"
#include "birdtrayapp.h"
#include "log.h"

TrayIcon::TrayIcon(bool showSettings)
{
    mBlinkingIconOpacity = 1.0;
    mBlinkingDelta = 0.0;
    mBlinkingTimeout = 0;

    mUnreadCounter = 0;

    // Context menu
    mSystrayMenu = new QMenu();
    setContextMenu( mSystrayMenu );

    mMenuShowHideThunderbird = 0;
    mMenuIgnoreUnreads = 0;
    mThunderbirdProcess = 0;
#ifdef Q_OS_WIN
    mThunderbirdUpdaterProcess = ProcessHandle::create("updater.exe");
    connect( mThunderbirdUpdaterProcess, &ProcessHandle::finished,
            this, &TrayIcon::tbUpdaterProcessFinished );
#endif /* Q_OS_WIN */
    
    mThunderbirdWindowExists = false;
    mThunderbirdWindowExisted = false;
    mThunderbirdWindowHide = false;
    connect(QApplication::instance(), &QApplication::aboutToQuit, this, &TrayIcon::onQuit);

    Settings* settings = BirdtrayApp::get()->getSettings();
    mThunderbirdStartTime = QDateTime::currentDateTime().addSecs(settings->mLaunchThunderbirdDelay);

    mWinTools = WindowTools::create();
    if (mWinTools) {
        connect(mWinTools, &WindowTools::onWindowShown, this, &TrayIcon::onThunderbirdWindowShown);
        connect(mWinTools, &WindowTools::onWindowHidden, this, &TrayIcon::onThunderbirdWindowHidden);
    }
    createMenu();
    createUnreadCounterThread();

    connect( &mBlinkingTimer, &QTimer::timeout, this, &TrayIcon::blinkTimeout );
    connect( this, &TrayIcon::activated, this, &TrayIcon::actionSystrayIconActivated );

    // State timer
    connect( &mStateTimer, &QTimer::timeout, this, &TrayIcon::updateState );
    mStateTimer.setInterval( 1000 );
    mStateTimer.start();
    
    // Update the state and icon when everything is settled
    updateState();
    updateIcon();
    show();
    
    if (settings->mUpdateOnStartup) {
        doAutoUpdateCheck();
    }
    
    // If the settings are not yet configure, pop up the message
    if (!showSettings && settings->showDialogIfNoAccountsConfigured
        && settings->watchedMorkFiles.isEmpty()) {
        QMessageBox questionDialog(QMessageBox::Question, tr("Would you like to set up Birdtray?"),
                tr("You have not yet configured any email folders to monitor. "
                   "Would you like to do it now?"), QMessageBox::Yes | QMessageBox::No);
        QPushButton* dontAskAgainButton = questionDialog.addButton(
                tr("Don't ask again"), QMessageBox::RejectRole);
        showSettings = questionDialog.exec() == QMessageBox::Yes;
        if (questionDialog.clickedButton() == dontAskAgainButton) {
            settings->showDialogIfNoAccountsConfigured = false;
            settings->save();
        }
    }
    if (showSettings) {
        QTimer::singleShot(0, this, &TrayIcon::showSettings);
    }
}

TrayIcon::~TrayIcon() {
    if (settingsDialog != nullptr) {
        settingsDialog->deleteLater();
    }
#ifdef Q_OS_WIN
    mThunderbirdUpdaterProcess->deleteLater();
#endif /* Q_OS_WIN */
    if (mUnreadMonitor != nullptr) {
        if (mUnreadMonitor->isRunning()) {
            mUnreadMonitor->quit();
            mUnreadMonitor->wait();
        }
        mUnreadMonitor->deleteLater();
        mUnreadMonitor = nullptr;
    }
}

WindowTools* TrayIcon::getWindowTools() const {
    return mWinTools;
}

UnreadMonitor* TrayIcon::getUnreadMonitor() const {
    return mUnreadMonitor;
}

void TrayIcon::unreadCounterUpdate( unsigned int total, QColor color )
{
    Log::debug("unreadCounterUpdate %d", total );
    Settings* settings = BirdtrayApp::get()->getSettings();
    if (settings->ignoreUnreadCountOnStart && !haveUnreadMailsData) {
        // Ignore unread emails that are present at Birdtray startup.
        setIgnoredUnreadMails(total, false);
    }
    if (total < ignoredUnreadEmails) {
        setIgnoredUnreadMails(total, false);
    }
    
    // Execute the hook process
    if ( !settings->mProcessRunOnCountChange.isEmpty() )
    {
        QString cmdline = settings->mProcessRunOnCountChange;

        // Replace the %NEW% with the new unread count
        cmdline.replace( "%NEW%", QString::number( total ) );

        // Replace the %OLD% with the old unread count
        cmdline.replace( "%OLD%", QString::number( mUnreadCounter ) );

        if ( !QProcess::startDetached( cmdline ) )
            Log::debug( "Failed to execute hook command %s", qPrintable( cmdline ) );
        else
            Log::debug( "Executing hook command %s", qPrintable( cmdline ) );
    }

    mUnreadCounter = total;
    mUnreadColor = color;
    haveUnreadMailsData = true;

    updateIcon();
}

void TrayIcon::unreadMonitorWarningChanged(const QString &path) {
    const QString &message = mUnreadMonitor->getWarnings().value(path);
    if (!message.isNull()) {
        qWarning("UnreadMonitor generated a warning for %s: %s",
                qPrintable(path), qPrintable(message));
    }
    updateIcon();
}

// Shamelessly stolen from Spivak Karaoke Player: github.com/gyunaev/spivak
static unsigned int largestFontSize(const QFont &font, int minfontsize, int maxfontsize, const QString &text, const QSize& rectsize )
{
    int cursize = minfontsize;
    QFont testfont( font );

    // We are trying to find the maximum font size which fits by doing the binary search
    while ( maxfontsize - minfontsize > 1 )
    {
        cursize = minfontsize + (maxfontsize - minfontsize) / 2;
        testfont.setPointSize( cursize );
        testfont.setWeight(
                static_cast<QFont::Weight>(BirdtrayApp::get()->getSettings()->mNotificationFontWeight));
        QSize size = QFontMetrics( testfont ).size( Qt::TextSingleLine, text );

        if ( size.width() < rectsize.width() && size.height() <= rectsize.height() )
            minfontsize = cursize;
        else
            maxfontsize = cursize;
    }

    return cursize;
}

void TrayIcon::updateIcon()
{
    Settings* settings = BirdtrayApp::get()->getSettings();
    // How many unread messages are here?
    unsigned int unread = mUnreadCounter;

    // If we are snoozed, ignore the unread messages
    if ( !mSnoozedUntil.isNull() )
    {
        // Hide the unreads
        unread = 0;
    }
    else
    {
        // Subtract the ignored unread mails
        unread -= ignoredUnreadEmails;

        // Are we blinking, and if not, should we be?
        if (unread > 0 && settings->mBlinkSpeed > 0 && mBlinkingTimeout == 0) {
            enableBlinking(true);
        }

        if ( unread == 0 && mBlinkingTimeout != 0 )
            enableBlinking( false );
    }
    if (settings->onlyShowIconOnUnreadMessages && unread == 0) {
        this->hide();
        return;
    }

    QPixmap temp(settings->getNotificationIcon().size());
    QPainter p;

    temp.fill( Qt::transparent );
    p.begin( &temp );

    // We use 0.75 opacity if we have unread count and non-blinking.
    // We use blinking opacity if we have unread count and blinking.
    // And we use 1.0 if we have zero unread count
    if ( !mSnoozedUntil.isNull() )
        p.setOpacity( 0.5 );
    else if ( unread == 0 )
        p.setOpacity( 1.0 );
    else
        p.setOpacity( mBlinkingIconOpacity );

    if (unread != 0 && !settings->mNotificationIconUnread.isNull()) {
        p.drawPixmap(settings->mNotificationIconUnread.rect(), settings->mNotificationIconUnread);
    } else {
        p.drawPixmap(settings->getNotificationIcon().rect(), settings->getNotificationIcon());
    }

    p.setFont(settings->mNotificationFont);

    // Do we need to draw error sign?
    if (settings->mMonitorThunderbirdWindow && !mThunderbirdWindowExists) {
        p.setOpacity( 1.0 );
        QPen pen( Qt::red );
        pen.setWidth( (temp.width() * 10) / 100 );
        p.setPen( pen );
        p.drawLine( 2, 2, temp.width() - 3, temp.height() - 3 );
        p.drawLine( temp.width() - 3, 2, 2, temp.height() - 3 );
        unread = 0;
    }

    // Do we need to draw the unread counter?
    if (unread > 0 && settings->mShowUnreadEmailCount) {
        // Find the suitable font size, starting from 4
        QString countvalue = QString::number( unread );

        int fontsize = static_cast<int>(largestFontSize(
                settings->mNotificationFont,
                static_cast<int>(settings->mNotificationMinimumFontSize),
                static_cast<int>(settings->mNotificationMaximumFontSize),
                countvalue, temp.size() - QSize(2, 2)));

        settings->mNotificationFont.setPointSize(fontsize);
        settings->mNotificationFont.setWeight(static_cast<QFont::Weight>(settings->mNotificationFontWeight));
        QFontMetrics fm(settings->mNotificationFont);
        p.setOpacity( mBlinkingTimeout ? 1.0 - mBlinkingIconOpacity : 1.0 );

        QPainterPath textPath;
        textPath.addText(0, 0, settings->mNotificationFont, countvalue);
        QRectF textBoundingRect = textPath.boundingRect();

        double horizontalMargin = temp.width() - textBoundingRect.width();
        double verticalMargin = temp.height() - textBoundingRect.height();
        double borderMargin = settings->mNotificationBorderWidth;
        double horizontalOffset = ((horizontalMargin - borderMargin) / 2.0) * settings->horizontalUnreadCountOffset;
        double verticalOffset = ((verticalMargin - borderMargin) / 2.0) * -settings->verticalUnreadCountOffset;
        
        double x = std::clamp(horizontalMargin / 2.0 + horizontalOffset, 0.0, horizontalMargin) - textBoundingRect.left();
        double y = std::clamp(verticalMargin / 2.0 + verticalOffset, 0.0, verticalMargin) - textBoundingRect.top();
        textPath.translate(x, y);
        if (settings->mNotificationBorderWidth > 0
            && settings->mNotificationBorderColor.isValid()) {
            p.strokePath(textPath, QPen(
                    settings->mNotificationBorderColor, settings->mNotificationBorderWidth));
        }
        p.fillPath(textPath, mUnreadColor);
    }
    const QMap<QString, QString> warnings = mUnreadMonitor->getWarnings();
    if (warnings.isEmpty()) {
        setToolTip(QString());
    } else {
        QStringList toolTip;
        const QString &globalWarning = warnings.value(QString());
        if (!globalWarning.isNull()) {
            toolTip << tr("Warning: %1").arg(globalWarning);
        }
        QMapIterator<QString, QString> warningsIterator(warnings);
        while (warningsIterator.hasNext()) {
            warningsIterator.next();
            const QString &path = warningsIterator.key();
            if (path.isNull()) {
                continue;
            }
            QFileInfo accountMorkFile(path);
            QString accountName = Utils::getMailAccountName(accountMorkFile);
            QString mailFolderName = Utils::getMailFolderName(accountMorkFile);
            QString name;
            if (accountName.isNull() || mailFolderName.isNull()) {
                name = path;
            } else {
                name = accountName + " [" + mailFolderName + "]";
            }
            const QString &warning = warningsIterator.value();
            toolTip << name + ": " + warning;
        }
        setToolTip(toolTip.join('\n'));
        drawWarningIndicator(p, temp.size());
    }

    p.end();

    // FIXME: this is not very efficient, although at our icon sizes (128x128) is probably ok.
    if ( mLastDrawnIcon != temp.toImage() )
    {
        mLastDrawnIcon = temp.toImage();
        setIcon( temp );
    }
    this->show();
}

void TrayIcon::enableBlinking(bool enabled)
{
    if ( enabled )
    {
        Settings* settings = BirdtrayApp::get()->getSettings();
        mBlinkingIconOpacity = 1.0;

        // If we are using the alpha transition, we have to update icon more often
        if (settings->mBlinkingUseAlphaTransition) {
            mBlinkingDelta = std::min(1.0, (2.0 / settings->mBlinkSpeed));
            mBlinkingTimeout = settings->mBlinkSpeed == 1 ? 50 : 100;
        } else {
            // The blinking speed slider is a value from 0 to 30,
            // so we make it 50x so the maximum is 1500 ms.
            mBlinkingDelta = 0;
            mBlinkingTimeout = settings->mBlinkSpeed * 50;
        }

        mBlinkingTimer.setInterval( mBlinkingTimeout );
        mBlinkingTimer.start();
    }
    else
    {
        mBlinkingTimer.stop();
        mBlinkingIconOpacity = 1.0;
        mBlinkingDelta = 0.0;
        mBlinkingTimeout = 0;
    }
}

void TrayIcon::updateState()
{
    if ( !mSnoozedUntil.isNull() && mSnoozedUntil < QDateTime::currentDateTimeUtc() )
    {
        // We are unsnoozed now
        actionUnsnooze(); // this will call updateIcon again, but with empty mSnoozedUntil
    }

    if ( mWinTools )
    {
        mThunderbirdWindowExists = mWinTools->lookup();

        // Is Thunderbird running?
        if ( mThunderbirdWindowExists )
        {
            // If the window is found, we remember it
            if ( !mThunderbirdWindowExisted )
                mThunderbirdWindowExisted = true;

            if ( !mMenuShowHideThunderbird->isEnabled() )
                mMenuShowHideThunderbird->setEnabled( true );

            // Hide the window if requested
            if ( mThunderbirdWindowHide )
            {
                mThunderbirdWindowHide = false;
                hideThunderbird();
            }
        }
        else
        {
            Settings* settings = BirdtrayApp::get()->getSettings();
            // Thunderbird is not running. Has it run before?
            if (!mThunderbirdWindowExisted) {
                // No. Shall we start it?
                if (settings->mLaunchThunderbird && !mThunderbirdProcess &&
                    mThunderbirdStartTime < QDateTime::currentDateTime()) {
                    startThunderbird();

                    // Hide after?
                    if (settings->mHideWhenStarted) {
                        mThunderbirdWindowHide = true;
                    }
                }
            } else {
                // It has run before, but not running now. Should we restart?
                if (settings->mRestartThunderbird && !mThunderbirdProcess) {
                    startThunderbird();

                    // Hide after?
                    if (settings->mHideWhenRestarted) {
                        mThunderbirdWindowHide = true;
                    }
                }
            }
        }

        // Update the menu text as the window can be hidden in wintools
        if ( mWinTools->isHidden() )
            mMenuShowHideThunderbird->setText( tr("Show Thunderbird") );
        else
            mMenuShowHideThunderbird->setText( tr("Hide Thunderbird") );

        updateIcon();
    }
}

void TrayIcon::blinkTimeout()
{
    // Apply blinking fade-in/fade-out, if needed
    if ( mBlinkingDelta != 0.0 )
    {
        if ( mBlinkingIconOpacity + mBlinkingDelta > 1.0 || mBlinkingIconOpacity + mBlinkingDelta < 0.0 )
            mBlinkingDelta = -mBlinkingDelta;

        mBlinkingIconOpacity += mBlinkingDelta;
    }
    else
    {
        Settings* settings = BirdtrayApp::get()->getSettings();
        // We are either not blinking at all, or doing no transition
        // this depends on nonzero settings->mBlinkSpeed
        if (settings->mBlinkSpeed != 0) {
            // Flip the opacity
            if (mBlinkingIconOpacity == settings->mUnreadOpacityLevel) {
                mBlinkingIconOpacity = 1.0 - settings->mUnreadOpacityLevel;
            } else {
                mBlinkingIconOpacity = settings->mUnreadOpacityLevel;
            }
        } else {
            mBlinkingIconOpacity = settings->mUnreadOpacityLevel;
        }
    }

    updateIcon();
}

void TrayIcon::actionQuit()
{
    QApplication::quit();
}

void TrayIcon::showSettings()
{
    if (settingsDialog != nullptr) {
        settingsDialog->show();
        settingsDialog->raise();
        settingsDialog->activateWindow();
        return;
    }
    Settings* settings = BirdtrayApp::get()->getSettings();
    settingsDialog = new DialogSettings();
    connect(settingsDialog, &QDialog::finished, this, [=](int result) {
        settingsDialog->deleteLater();
        settingsDialog = nullptr;
        if (result != QDialog::Accepted) {
            return;
        }
        settings->save();
        if (!settings->mAllowSuppressingUnreads) {
            setIgnoredUnreadMails(0, false);
        }

        // Recreate menu
        createMenu();

        // Recalculate the delta
        enableBlinking( false );
        updateIcon();
        // TODO: Update on thunderbird path setting change

        emit settingsChanged();
    });
    settingsDialog->show();
}

void TrayIcon::actionActivate()
{
    if ( !mWinTools )
        return;

    Settings* settings = BirdtrayApp::get()->getSettings();
    if ( settings->startClosedThunderbird && !mWinTools->lookup() && mThunderbirdProcess == nullptr ) {
        startThunderbird();
        if (settings->hideWhenStartedManually) {
            mThunderbirdWindowHide = true;
        }
    } else if ( mWinTools->isHidden() ) {
        showThunderbird();
    } else {
        hideThunderbird();
    }
}

void TrayIcon::actionSnoozeFor()
{
    // Snoozing time is added as QAction's userdata
    QAction * action = (QAction *) sender();
    mSnoozedUntil = QDateTime::currentDateTimeUtc().addSecs( action->data().toInt() );

    Log::debug( "Snoozed until %s UTC", qPrintable(mSnoozedUntil.toString() ) );

    // Unhide the unsnoozer
    mMenuUnsnooze->setVisible( true );

    // Reset the blinker
    enableBlinking( false );

    updateIcon();
}

void TrayIcon::actionUnsnooze()
{
    mSnoozedUntil = QDateTime();

    // Hide the snooze menu
    mMenuUnsnooze->setVisible( false );

    updateIcon();
}

void TrayIcon::actionNewEmail() {
    Settings* settings = BirdtrayApp::get()->getSettings();

    QString executable;
    QStringList args;

    if ( !settings->getStartThunderbirdCmdline( executable, args ) )
        return;

    args << "-compose";

    if (!settings->mNewEmailData.isEmpty()) {
        auto* action = (QAction*) sender();
        if (action->data().isValid()) {
            int index = (action->data().toInt());
            if (index < 0 || index > settings->mNewEmailData.size() - 1) {
                return;
            }
            args << settings->mNewEmailData[index].asArgs();
        }
    }

    QProcess::startDetached(executable, args);
}

void TrayIcon::actionIgnoreEmails()
{
    setIgnoredUnreadMails(mUnreadCounter);
}

void TrayIcon::actionSystrayIconActivated(QSystemTrayIcon::ActivationReason reason) {
    if (reason == QSystemTrayIcon::Trigger) {
        Settings* settings = BirdtrayApp::get()->getSettings();
        if (settings->mShowHideThunderbird || (!mThunderbirdWindowExists && settings->startClosedThunderbird)) {
            actionActivate();
        }
    }
}

void TrayIcon::createMenu()
{
    Settings* settings = BirdtrayApp::get()->getSettings();
    mSystrayMenu->clear();

    // Show and hide action
    mMenuShowHideThunderbird = new QAction( tr("Hide Thunderbird"), this );
    connect( mMenuShowHideThunderbird, &QAction::triggered, this, &TrayIcon::actionActivate );

    // We start with disabled action, and enable it once the window is detected
    mMenuShowHideThunderbird->setEnabled( false );

    mSystrayMenu->addAction( mMenuShowHideThunderbird );
    mSystrayMenu->addSeparator();

    // New email could be either a single action, or menu depending on settings
    if (settings->mNewEmailMenuEnabled) {
        if (!settings->mNewEmailData.isEmpty()) {
            // A submenu
            auto* newEmails = new QMenu(tr("New Email"));
            auto* action = new QAction(tr("Blank"), this);
            connect(action, &QAction::triggered, this, &TrayIcon::actionNewEmail);
            newEmails->addAction(action);
            newEmails->addSeparator();
            for (int index = 0; index < settings->mNewEmailData.size(); index++) {
                action = new QAction(settings->mNewEmailData[index].menuentry(), this);
                connect(action, &QAction::triggered, this, &TrayIcon::actionNewEmail);
                // Remember the delay in the action itself
                action->setData(index);
                newEmails->addAction(action);
            }
            mSystrayMenu->addMenu(newEmails);
        } else {
            // A single action
            mSystrayMenu->addAction(tr("New Email Message"), this, SLOT(actionNewEmail()));
        }
        mSystrayMenu->addSeparator();
    }

    // Snoozer times map, for easy editing. The first parameter is in seconds
    QMap< unsigned int, QString > snoozingTimes;

    snoozingTimes.insert( 360, tr( "5 minutes") );
    snoozingTimes.insert( 600, tr( "10 minutes") );
    snoozingTimes.insert( 1800, tr( "30 minutes") );
    snoozingTimes.insert( 3600, tr( "1 hour") );
    snoozingTimes.insert( 3600 * 4, tr( "4 hours") );

    // And add them all
    QMenu * snooze = new QMenu( tr("Snooze for ...") );

    for ( unsigned int snoozingseconds : snoozingTimes.keys() )
    {
        QAction * a = new QAction( snoozingTimes[snoozingseconds], this );
        connect( a, &QAction::triggered, this, &TrayIcon::actionSnoozeFor );

        // Remember the delay in the action itself
        a->setData( snoozingseconds );
        snooze->addAction( a );
    }

    // And add snoozing menu itself
    mSystrayMenu->addMenu( snooze );

    // Unsnooze menu item is unvisible by default
    mMenuUnsnooze = new QAction( tr("Unsnooze"), this );
    connect( mMenuUnsnooze, &QAction::triggered, this, &TrayIcon::actionUnsnooze );

    mSystrayMenu->addAction( mMenuUnsnooze );
    mMenuUnsnooze->setVisible( false );

    // Add the ignore action
    if (settings->mAllowSuppressingUnreads) {
        mMenuIgnoreUnreads = new QAction( tr("Ignore unread emails"), this );
        connect( mMenuIgnoreUnreads, &QAction::triggered, this, &TrayIcon::actionIgnoreEmails );
        mSystrayMenu->addAction( mMenuIgnoreUnreads );
        // Force an update of the ignore unread mails menu entry text.
        setIgnoredUnreadMails(ignoredUnreadEmails++, false);
    }
    else {
        mMenuIgnoreUnreads = nullptr;
    }

    mSystrayMenu->addSeparator();

    // Some generic actions
    mSystrayMenu->addAction( tr("Settings..."), this, SLOT(showSettings()) );

    mSystrayMenu->addSeparator();

    // And exit
    mSystrayMenu->addAction( tr("Quit"), this, SLOT(actionQuit()) );
}

void TrayIcon::createUnreadCounterThread()
{
    mUnreadMonitor = new UnreadMonitor( this );

    connect( mUnreadMonitor, &UnreadMonitor::unreadUpdated, this, &TrayIcon::unreadCounterUpdate );
    connect(mUnreadMonitor, &UnreadMonitor::warningChanged, this,
            &TrayIcon::unreadMonitorWarningChanged);

    mUnreadMonitor->start();
}

void TrayIcon::startThunderbird()
{
    if ( mThunderbirdProcess ) {
        Log::debug("Not starting Thunderbird because we already started it and it is still running" );
        return;
    }

    QString executable;
    QStringList args;

    if ( !BirdtrayApp::get()->getSettings()->getStartThunderbirdCmdline( executable, args ) )
    {
        Log::debug("Failed to get Thunderbird command-line" );
        return;
    }

    Log::debug("Starting Thunderbird as '%s %s'", qPrintable(executable), qPrintable(args.join(' ')));

    mThunderbirdProcess = new QProcess();
    connect( mThunderbirdProcess, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(tbProcessFinished(int,QProcess::ExitStatus)) );

#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
    connect( mThunderbirdProcess, &QProcess::errorOccurred, this, &TrayIcon::tbProcessError );
#endif

    mThunderbirdProcess->start(executable, args);
}

void TrayIcon::tbProcessError(QProcess::ProcessError )
{
#ifdef Q_OS_WIN
    if (mThunderbirdUpdaterProcess->attach() == AttachResult::SUCCESS)
    {
        return;
    }
#endif /* Q_OS_WIN */

    QMessageBox::critical(nullptr,
            tr("Cannot start Thunderbird"),
            tr("Error starting Thunderbird as '%1 %2':\n\n%3")
                    .arg( mThunderbirdProcess->program() )
                    .arg( mThunderbirdProcess->arguments().join(' ') )
                    .arg( mThunderbirdProcess->errorString()) );

    // We keep the mThunderbirdProcess pointer, so the process is not restarted again
}

void TrayIcon::tbProcessFinished(int, QProcess::ExitStatus)
{
    // If we are here this could mean that either Thunderbird was quit manually,
    // in which case it is restarted in updateState(), or that we started TB
    // and the active instance was activated (and our instance exited).
    // Thus we just destroy the process later, to let updateState() make decision
#ifdef Q_OS_WIN
    if (mThunderbirdUpdaterProcess->attach() == AttachResult::SUCCESS) {
        return;
    }
#endif /* Q_OS_WIN */
    mThunderbirdProcess->deleteLater();
    mThunderbirdProcess = nullptr;
}

#ifdef Q_OS_WIN
void TrayIcon::tbUpdaterProcessFinished(const ProcessHandle::ExitReason& exitReason)
{
    if (exitReason.isError()) {
        QMessageBox::critical(
                nullptr, tr("Cannot start Thunderbird"),
                tr("Error starting Thunderbird, because we could not attach to the updater:\n\n%1")
                .arg( exitReason.getErrorDescription() ) );
        return;
    }
    // The updater will start Thunderbird automatically
    mThunderbirdProcess->deleteLater();
    mThunderbirdProcess = nullptr;
}
#endif /* Q_OS_WIN */

void TrayIcon::onQuit() {
    if (mWinTools && mWinTools->isHidden()) {
        mWinTools->show();
    }
    if (BirdtrayApp::get()->getSettings()->mExitThunderbirdWhenQuit) {
        if (mWinTools) {
            mWinTools->closeWindow();
        }
    }
}

void TrayIcon::onAutoUpdateCheckFinished(bool foundUpdate, const QString &errorMessage) {
    Q_UNUSED(foundUpdate)
    AutoUpdater* autoUpdater = BirdtrayApp::get()->getAutoUpdater();
    if (errorMessage.isNull()) {
        disconnect(autoUpdater, &AutoUpdater::onCheckUpdateFinished,
                   this, &TrayIcon::onAutoUpdateCheckFinished);
    }
}

void TrayIcon::onThunderbirdWindowShown() {
    mMenuShowHideThunderbird->setText( tr("Hide Thunderbird") );
    if (haveUnreadMailsData && BirdtrayApp::get()->getSettings()->ignoreUnreadCountOnShow) {
        setIgnoredUnreadMails(mUnreadCounter);
    }
}

void TrayIcon::onThunderbirdWindowHidden() {
    mMenuShowHideThunderbird->setText( tr("Show Thunderbird") );
    if (haveUnreadMailsData && BirdtrayApp::get()->getSettings()->ignoreUnreadCountOnHide) {
        setIgnoredUnreadMails(mUnreadCounter);
    }
}

void TrayIcon::hideThunderbird()
{
    mWinTools->hide();
}

void TrayIcon::showThunderbird()
{
    mWinTools->show();
}

void TrayIcon::setIgnoredUnreadMails(unsigned int ignoredMails, bool updateIcon) {
    if (ignoredMails == ignoredUnreadEmails) {
        return;
    }
    Log::debug("Setting ignored unread mails to %u", ignoredMails);
    ignoredUnreadEmails = ignoredMails;
    if (mMenuIgnoreUnreads) {
        if (ignoredUnreadEmails > 0) {
            mMenuIgnoreUnreads->setText(
                    tr("Ignore unread emails (now %1)").arg(ignoredUnreadEmails));
        } else {
            mMenuIgnoreUnreads->setText(tr("Ignore unread emails"));
        }
    }
    if (updateIcon) {
        this->updateIcon();
    }
}

void TrayIcon::doAutoUpdateCheck() {
    AutoUpdater* autoUpdater = BirdtrayApp::get()->getAutoUpdater();
    connect(autoUpdater, &AutoUpdater::onCheckUpdateFinished,
            this, &TrayIcon::onAutoUpdateCheckFinished);
    autoUpdater->checkForUpdates();
}

void TrayIcon::drawWarningIndicator(QPainter &painter, const QSize &iconSize) {
    painter.setOpacity(1.0);
    int width = iconSize.width() / 4;
    QPen pen(QColor(255, 200, 0, 255));
    pen.setWidth(width);
    painter.setPen(pen);
    int x = iconSize.width() - static_cast<int>(iconSize.width() * 0.125) - pen.width() / 2;
    painter.drawLine(x, static_cast<int>(iconSize.height() * 0.33),
            x, iconSize.height() - width / 2);
    pen.setColor(QColor(255, 120, 0, 255));
    pen.setWidthF(std::max(pen.width() - 16, 1));
    painter.setPen(pen);
    painter.drawLine(x, static_cast<int>(iconSize.height() * 0.33),
            x, iconSize.height() - 20 - width);
    painter.drawPoint(x, iconSize.height() - width / 2);
}
