#include <QMenu>
#include <QTimer>
#include <QPixmap>
#include <QPainter>
#include <QMessageBox>
#include <QFontMetrics>

#include "settings.h"
#include "trayicon.h"
#include "unreadcounter.h"
#include "dialogsettings.h"
#include "windowtools.h"

TrayIcon::TrayIcon()
{
    updateIcon();

    mBlinkingIconOpacity = 1.0;
    mBlinkingDelta = 0.0;
    mBlinkingTimeout = 0;

    mUnreadCounter = 0;
    mUnreadMonitor = 0;
    mMenuShowHideThunderbird = 0;

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
    updateIcon();

    connect( &mBlinkingTimer, &QTimer::timeout, this, &TrayIcon::updateIcon );

    connect( this, &TrayIcon::activated, this, &TrayIcon::actionSystrayIconActivated );

    // State timer
    connect( &mStateTimer, &QTimer::timeout, this, &TrayIcon::updateState );
    mStateTimer.setInterval( 1000 );
    mStateTimer.start();

    // Start Thunderbird
    if ( pSettings->mLaunchThunderbird )
    {
        //TODO: error handling and status check
        mThunderbirdProcess.start( pSettings->mThunderbirdCmdLine );
    }
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
        if ( unread > 0 && pSettings->mBlinkSpeed > 0 && mBlinkingDelta == 0.0 )
            setBlinking( 100, pSettings->mBlinkSpeed );

        if ( unread == 0 && mBlinkingDelta > 0.0 )
            setBlinking( 0, 0 );
    }

    // Apply blinking, if needed
    if ( mBlinkingTimeout )
    {
        if ( mBlinkingIconOpacity + mBlinkingDelta > 1.0 || mBlinkingIconOpacity + mBlinkingDelta < 0.0 )
            mBlinkingDelta = -mBlinkingDelta;

        mBlinkingIconOpacity += mBlinkingDelta;
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
    else if ( mBlinkingTimeout == 0 )
        p.setOpacity( 0.75 );
    else
        p.setOpacity( mBlinkingIconOpacity );

    p.drawPixmap( pSettings->mNotificationIcon.rect(), pSettings->mNotificationIcon );
    p.setFont( pSettings->mNotificationFont );

    // Do we need to draw error sign?
    if ( mUnreadMonitor == 0 )
    {
        p.setOpacity( 1.0 );
        QPen pen( Qt::red );
        pen.setWidth( (temp.width() * 10) / 100 );
        p.setPen( pen );
        p.drawLine( 2, 2, temp.width() - 3, temp.height() - 3 );
        p.drawLine( temp.width() - 3, 2, 2, temp.height() - 3 );
    }


    // Do we need to draw the unread counter?
    if ( unread > 0 )
    {
        // Find the suitable font size, starting from 4
        QString countvalue = QString::number( unread );
        int size = 4;

        for ( ; size < 256; size++ )
        {
            pSettings->mNotificationFont.setPointSize( size );
            pSettings->mNotificationFont.setWeight( pSettings->mNotificationFontWeight );
            QFontMetrics fm( pSettings->mNotificationFont );

            if ( fm.width( countvalue ) > temp.width() - 2 || fm.height() > temp.height() - 2 )
                break;
        }

        pSettings->mNotificationFont.setPointSize( size - 1 );
        QFontMetrics fm( pSettings->mNotificationFont );
        p.setOpacity( mBlinkingTimeout ? 1.0 - mBlinkingIconOpacity : 1.0 );
        p.setPen( mUnreadColor );
        p.drawText( (temp.width() - fm.width( countvalue )) / 2, (temp.height() - fm.height()) / 2 + fm.ascent(), countvalue );
    }

    p.end();

    setIcon( temp );
}

void TrayIcon::setBlinking(int timeoutms, int percentagechange)
{
    if ( timeoutms > 0 )
    {
        mBlinkingIconOpacity = 1.0;
        mBlinkingDelta = percentagechange / 100.0;
        mBlinkingTimeout = timeoutms;
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

    if ( !mMenuShowHideThunderbird->isEnabled() )
    {
        if ( mWinTools && mWinTools->lookup() )
            mMenuShowHideThunderbird->setEnabled( true );
    }
}


void TrayIcon::actionQuit()
{
    if ( mWinTools && mWinTools->isHidden() )
        mWinTools->show();

    if ( pSettings->mExitThunderbirdWhenQuit )
    {
        if ( mWinTools )
            mWinTools->closeWindow();
        else
            mThunderbirdProcess.terminate();
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
        setBlinking( 0, 0 );

        updateIcon();
    }
}

void TrayIcon::actionActivate()
{
    qDebug("activate");

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
    setBlinking( 0, 0 );

    updateIcon();
}

void TrayIcon::actionUnsnooze()
{
    mSnoozedUntil = QDateTime();

    // Hide the snooze menu
    mMenuUnsnooze->setVisible( false );

    updateIcon();
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

    menu->addAction( mMenuShowHideThunderbird );
    menu->addSeparator();

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
    mUnreadMonitor = new UnreadMonitor();

    connect( mUnreadMonitor, &UnreadMonitor::unreadUpdated, this, &TrayIcon::unreadCounterUpdate );
    connect( mUnreadMonitor, &UnreadMonitor::error, this, &TrayIcon::unreadCounterError );

    mUnreadMonitor->start();
}
