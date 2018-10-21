#include <QMenu>
#include <QTimer>
#include <QPixmap>
#include <QPainter>
#include <QProcess>
#include <QMessageBox>
#include <QFontMetrics>

#include "settings.h"
#include "trayicon.h"
#include "unreadcounter.h"
#include "dialogsettings.h"
#include "windowtools.h"

TrayIcon::TrayIcon()
{
    mBlinkingIconOpacity = 1.0;
    mBlinkingDelta = 0.0;
    mBlinkingTimeout = 0;

    mUnreadCounter = 0;
    mUnreadMonitor = 0;
    mMenuShowHideThunderbird = 0;
    mThunderbirdWindowExists = false;
    mThunderbirdWindowExisted = false;
    mThunderbirdWindowHide = false;

    mWinTools = WindowTools::create();

    // If the settings are not yet configure, pop up the message
    if ( pSettings->mThunderbirdFolderPath.isEmpty() )
    {
        if ( QMessageBox::question( 0,
            tr( "Would you like to set up Birdtray?" ),
            tr( "You have not yet configured the Thunderbird profile path. Would you like to do it now?") ) == QMessageBox::Yes )
        {
            actionSettings();
        }
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
}

void TrayIcon::unreadCounterUpdate( unsigned int total, QColor color )
{
    qDebug("unreadCounterUpdate %d", total );
    mUnreadCounter = total;
    mUnreadColor = color;

    updateIcon();
}

void TrayIcon::unreadCounterError(QString message)
{
    qWarning("UnreadCounter generated an error: %s", qPrintable(message) );

    mCurrentStatus = message;
    mUnreadMonitor->deleteLater();
    mUnreadMonitor = 0;

    mUnreadCounter = 0;
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
        // Are we blinking, and if not, should we be?
        if ( unread > 0 && pSettings->mBlinkSpeed > 0 && mBlinkingTimeout == 0 )
            enableBlinking( true );

        if ( unread == 0 && mBlinkingTimeout != 0 )
            enableBlinking( false );
    }

    QPixmap temp( pSettings->mNotificationIcon.size() );
    QPainter p;

    temp.fill( Qt::transparent );
    p.begin( &temp );

    // We use 0.75 opacity if we have unread count and non-blinking.
    // We use blinking opacity if we have unread count and blinking.
    // And we use 1.0 if we have zero unread count
    if ( unread == 0 )
        p.setOpacity( 1.0 );
    else
        p.setOpacity( mBlinkingIconOpacity );

    p.drawPixmap( pSettings->mNotificationIcon.rect(), pSettings->mNotificationIcon );
    p.setFont( pSettings->mNotificationFont );

    // Do we need to draw error sign?
    if ( mUnreadMonitor == 0 || (pSettings->mMonitorThunderbirdWindow && !mThunderbirdWindowExists ) )
    {
        p.setOpacity( 1.0 );
        QPen pen( Qt::red );
        pen.setWidth( (temp.width() * 10) / 100 );
        p.setPen( pen );
        p.drawLine( 2, 2, temp.width() - 3, temp.height() - 3 );
        p.drawLine( temp.width() - 3, 2, 2, temp.height() - 3 );
        unread = 0;
    }

    // Do we need to draw the unread counter?
    if ( unread > 0 )
    {
        // Find the suitable font size, starting from 4
        QString countvalue = QString::number( unread );

        int fontsize = largestFontSize( pSettings->mNotificationFont,
                                        pSettings->mNotificationMinimumFontSize,
                                        pSettings->mNotificationMaximumFontSize,
                                        countvalue,
                                        temp.size() - QSize( 2, 2 ) );

        pSettings->mNotificationFont.setPointSize( fontsize );
        QFontMetrics fm( pSettings->mNotificationFont );
        p.setOpacity( mBlinkingTimeout ? 1.0 - mBlinkingIconOpacity : 1.0 );
        p.setPen( mUnreadColor );
        p.drawText( (temp.width() - fm.width( countvalue )) / 2, (temp.height() - fm.height()) / 2 + fm.ascent(), countvalue );
    }

    p.end();

    // FIXME: this is not very efficient, although at our icon sizes (128x128) is probably ok.
    if ( mLastDrawnIcon != temp.toImage() )
    {
        mLastDrawnIcon = temp.toImage();
        setIcon( temp );
    }
}

void TrayIcon::enableBlinking(bool enabled)
{
    if ( enabled )
    {
        mBlinkingIconOpacity = 1.0;

        // If we are using the alpha transition, we have to update icon more often
        if ( pSettings->mBlinkingUseAlphaTransition )
        {
            mBlinkingDelta = pSettings->mBlinkSpeed / 100.0;
            mBlinkingTimeout = 100;
        }
        else
        {
            // The blinking speed slider is a value from 0 to 30, so we make it 50x
            mBlinkingDelta = 0;
            mBlinkingTimeout = pSettings->mBlinkSpeed * 50;
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
            {
                mThunderbirdWindowExisted = true;

                if ( !mMenuShowHideThunderbird->isEnabled() )
                    mMenuShowHideThunderbird->setEnabled( true );
            }

            // Hide the window if requested
            if ( mThunderbirdWindowHide )
            {
                mThunderbirdWindowHide = false;
                mWinTools->hide();
            }
        }
        else
        {
            // Thunderbird is not running. Has it run before?
            if ( !mThunderbirdWindowExisted )
            {
                // No. Shall we start it?
                if ( pSettings->mLaunchThunderbird && !hasThunderbirdRecentlyStarted() )
                {
                    startThunderbird();

                    // Hide after?
                    if ( pSettings->mHideWhenStarted )
                        mThunderbirdWindowHide = true;
                }
            }
            else
            {
                // It has run before, but not running now. Should we restart?
                if ( pSettings->mRestartThunderbird && !hasThunderbirdRecentlyStarted() )
                {
                    startThunderbird();

                    // Hide after?
                    if ( pSettings->mHideWhenRestarted )
                        mThunderbirdWindowHide = true;
                }
            }
        }

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
        // We are either not blinking at all, or doing no transition
        // this depends on nonzero pSettings->mBlinkSpeed
        if ( pSettings->mBlinkSpeed != 0 )
        {
            // Flip the opacity
            if ( mBlinkingIconOpacity == 0.75 )
                mBlinkingIconOpacity = 0.15;
            else
                mBlinkingIconOpacity = 0.75;
        }
        else
            mBlinkingIconOpacity = 0.75;
    }

    updateIcon();
}


void TrayIcon::actionQuit()
{
    if ( mWinTools && mWinTools->isHidden() )
        mWinTools->show();

    if ( pSettings->mExitThunderbirdWhenQuit )
    {
        if ( mWinTools )
            mWinTools->closeWindow();
    }

    exit( 0 );
}

void TrayIcon::actionSettings()
{
    DialogSettings dlg;

    if ( dlg.exec() == QDialog::Accepted )
    {
        pSettings->save();

        if ( !mUnreadMonitor )
            createUnreadCounterThread();

        // Recalculate the delta
        enableBlinking( false );

        updateIcon();

        emit settingsChanged();
    }
}

void TrayIcon::actionActivate()
{
    if ( !mWinTools )
        return;

    if ( mWinTools->isHidden() )
    {
        mMenuShowHideThunderbird->setText( tr("Hide Thunderbird") );
        mWinTools->show();
    }
    else
    {
        mMenuShowHideThunderbird->setText( tr("Show Thunderbird") );
        mWinTools->hide();
    }
}

void TrayIcon::actionSnoozeFor()
{
    // Snoozing time is added as QAction's userdata
    QAction * action = (QAction *) sender();
    mSnoozedUntil = QDateTime::currentDateTimeUtc().addSecs( action->data().toInt() );

    qDebug( "Snoozed until %s UTC", qPrintable(mSnoozedUntil.toString() ) );

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

void TrayIcon::actionNewEmail()
{
    QStringList args;
    args << "-compose";

    if ( !pSettings->mNewEmailData.isEmpty() )
    {
        QAction * action = (QAction *) sender();
        int index = ( action->data().toInt() );

        if ( index <  0 || index > pSettings->mNewEmailData.size() - 1 )
            return;

        args << pSettings->mNewEmailData[index].asArgs();
    }

    QProcess::startDetached( pSettings->mThunderbirdCmdLine, args );
}

void TrayIcon::actionSystrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if ( reason == QSystemTrayIcon::Trigger )
        actionActivate();
}

void TrayIcon::createMenu()
{
    QMenu * menu = new QMenu();

    // Show and hide action
    mMenuShowHideThunderbird = new QAction( tr("Hide Thunderbird"), this );
    connect( mMenuShowHideThunderbird, &QAction::triggered, this, &TrayIcon::actionActivate );

    // We start with disabled action, and enable it once the window is detected
    mMenuShowHideThunderbird->setEnabled( false );

    menu->addAction( mMenuShowHideThunderbird );
    menu->addSeparator();

    // New email could be either a single action, or menu depending on settings
    if ( pSettings->mNewEmailMenuEnabled )
    {
        if ( !pSettings->mNewEmailData.isEmpty() )
        {
            // A submenu
            QMenu * newemails = new QMenu( tr("New Email") );

            for ( int index = 0; index < pSettings->mNewEmailData.size(); index++ )
            {
                QAction * a = new QAction( pSettings->mNewEmailData[index].menuentry(), this );
                connect( a, &QAction::triggered, this, &TrayIcon::actionNewEmail );

                // Remember the delay in the action itself
                a->setData( index );
                newemails->addAction( a );
            }

            menu->addMenu( newemails );
        }
        else
        {
            // A single action
            menu->addAction( tr("New Email Message"), this, &TrayIcon::actionNewEmail );
        }

        menu->addSeparator();
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
    menu->addMenu( snooze );

    // Unsnooze menu item is unvisible by default
    mMenuUnsnooze = new QAction( tr("Unsnooze"), this );
    connect( mMenuUnsnooze, &QAction::triggered, this, &TrayIcon::actionUnsnooze );

    menu->addAction( mMenuUnsnooze );
    mMenuUnsnooze->setVisible( false );

    menu->addSeparator();

    // Some generic actions
    menu->addAction( tr("Settings..."), this, &TrayIcon::actionSettings );

    menu->addSeparator();

    // And exit
    menu->addAction( tr("Quit"), this, &TrayIcon::actionQuit );

    setContextMenu( menu );
}

void TrayIcon::createUnreadCounterThread()
{
    mUnreadMonitor = new UnreadMonitor( this );

    connect( mUnreadMonitor, &UnreadMonitor::unreadUpdated, this, &TrayIcon::unreadCounterUpdate );
    connect( mUnreadMonitor, &UnreadMonitor::error, this, &TrayIcon::unreadCounterError );

    mUnreadMonitor->start();
}

void TrayIcon::startThunderbird()
{
    qDebug("Starting Thunderbird as '%s'", qPrintable( pSettings->mThunderbirdCmdLine ) );

    mThunderbirdRestartTime = QDateTime::currentDateTimeUtc().addSecs( 5 );
    QProcess::startDetached( pSettings->mThunderbirdCmdLine );
}

bool TrayIcon::hasThunderbirdRecentlyStarted() const
{
    return !mThunderbirdRestartTime.isNull() && mThunderbirdRestartTime > QDateTime::currentDateTimeUtc();
}
