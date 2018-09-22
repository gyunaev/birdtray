#include <QMenu>
#include <QTimer>
#include <QPixmap>
#include <QPainter>
#include <QFontMetrics>

#include "settings.h"
#include "trayicon.h"
#include "unreadcounter.h"
#include "dialogsettings.h"


TrayIcon::TrayIcon()
{
    updateIcon();

    mBlinkingIconOpacity = 1.0;
    mBlinkingDelta = 0.0;
    mBlinkingTimeout = 0;

    mUnreadCounter = 0;
    mUnreadMonitor = 0;

    createMenu();
    createUnreadCounterThread();
    updateIcon();

    connect( &mBlinkingTimer, &QTimer::timeout, this, &TrayIcon::updateIcon );
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
    if ( mIconPixmap.isNull() )
    {
        mIconPixmap = QPixmap( pSettings->mIconSize );

        if ( !mIconPixmap.load( ":res/thunderbird.png" ) )
        {
            qFatal("cannot load icon");
        }
    }

    // How many unread messages are here?
    unsigned int unread = mUnreadCounter;

    // If we are snoozed, ignore the unread messages
    if ( !mSnoozedUntil.isNull() )
    {
        if ( mSnoozedUntil < QDateTime::currentDateTimeUtc() )
        {
            // We are unsnoozed now
            actionUnsnooze(); // this will call updateIcon again, but with empty mSnoozedUntil
            return;
        }

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

    QPixmap temp( mIconPixmap.size() );
    QPainter p;

    temp.fill( Qt::transparent );
    p.begin( &temp );
    p.setOpacity( mBlinkingIconOpacity );
    p.drawPixmap( mIconPixmap.rect(), mIconPixmap );
    p.setFont( pSettings->mTextFont );

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
            pSettings->mTextFont.setPointSize( size );
            QFontMetrics fm( pSettings->mTextFont );

            if ( fm.width( countvalue ) > temp.width() - 2 || fm.height() > temp.height() - 2 )
                break;
        }

        pSettings->mTextFont.setPointSize( size - 1 );
        QFontMetrics fm( pSettings->mTextFont );
        p.setOpacity( 1.0 - mBlinkingIconOpacity );
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

void TrayIcon::actionQuit()
{
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
        mBlinkingDelta = 0.0;

        updateIcon();
    }
}

void TrayIcon::actionActivate()
{
    qDebug("activate");
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

    // Keep the update timer on, but on 1sec interval
    mBlinkingTimer.setInterval( 1000 );
    mBlinkingTimer.start();

    updateIcon();
}

void TrayIcon::actionUnsnooze()
{
    mSnoozedUntil = QDateTime();

    // Hide the snooze menu
    mMenuUnsnooze->setVisible( false );

    updateIcon();
}

void TrayIcon::createMenu()
{
    QMenu * menu = new QMenu();

    // Show and hide action
    mMenuShowHideThunderbird = new QAction( tr("Show Thunderbird") );
    connect( mMenuShowHideThunderbird, &QAction::triggered, this, &TrayIcon::actionActivate );

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
        QAction * a = new QAction( snoozingTimes[snoozingseconds] );
        connect( a, &QAction::triggered, this, &TrayIcon::actionSnoozeFor );

        // Remember the delay in the action itself
        a->setData( snoozingseconds );
        snooze->addAction( a );
    }

    menu->addAction( mMenuShowHideThunderbird );

    // And add snoozing menu itself
    menu->addMenu( snooze );

    // Unsnooze menu item is unvisible by default
    mMenuUnsnooze = new QAction( tr("Unsnooze") );
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
