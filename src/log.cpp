#include <stdarg.h>

#include <QDir>
#include <QDateTime>
#include <QStandardPaths>
#include <QMessageBox>
#include <QTextStream>

#include "log.h"
#include "dialoglogoutput.h"

Log * Log::mLog;

void Log::initialize(const QString& path )
{
    if (mLog != nullptr)
        Log::fatal("Attempt to initialize initialized log");

    Log * self = g();

    if ( path.isEmpty() )
        return;

    QMutexLocker l( &(self->mMutex) );
    bool opened = false;

    if ( path == "stderr" )
    {
        opened = self->mOutputFile.open( stderr, QIODevice::OpenModeFlag::WriteOnly );
    }
    else
    {
        self->mOutputFile.setFileName( path );
        opened = self->mOutputFile.open(QIODevice::OpenModeFlag::WriteOnly);
    }

    if ( !opened )
    {
        l.unlock();
        Log::fatal( QObject::tr("Failed to open log file %s: %s").arg(path).arg(self->mOutputFile.errorString()) );
    }
}

void Log::fatal( const QString& str )
{
    Log * self = g();

    // Add the log line
    self->add( str );

    // Write the whole log buffer into a file log.txt
    QFile file( QStandardPaths::writableLocation( QStandardPaths::TempLocation ) + QDir::separator() + "birdtray-log.txt" );

    // Show the dialog
    QMessageBox::critical(nullptr, QApplication::tr("Fatal"), QObject::tr("Fatal error: %1\n\nLog file is written into file %2") .arg(str) .arg(file.fileName()));

    self->mMutex.lock();

    if ( file.open(QFile::WriteOnly) )
    {
        QByteArray writedata = self->mEntries.join("\r\n").toUtf8();
        file.write( writedata );
        file.close();
    }

    // We're done, do not unlock self->mMutex
    exit( 1 );
}

void Log::debug(const char *fmt, ... )
{
    va_list vl;
    char buf[4096];

    va_start( vl, fmt );
    vsnprintf( buf, sizeof(buf) - 1, fmt, vl );
    va_end( vl );

    // Add the log line
    g()->add( buf );
}

void Log::showLogger()
{
    Log * self = g();

    QMutexLocker l( &self->mMutex );

    // Create a dialog if not exist
    if ( self->mDialog.isNull() )
    {
        self->mDialog = new DialogLogOutput();
        self->mDialog->add( self->mEntries );
        self->mDialog->show();
    }

    self->mDialog->activateWindow();
}

Log *Log::g()
{
    if ( mLog == nullptr )
        mLog = new Log();

    return mLog;
}

void Log::add(const QString &text)
{
    // Prepend a timestamp
    QDateTime now = QDateTime::currentDateTime();
    QString logline = QString("%1 %2 ") .arg( now.toString( "yyyy-MM-dd hh:mm:ss" )) .arg(text);

    QMutexLocker l( &mMutex );

    // Adding it to the log buffer
    if ( mEntries.count() >= MAX_LOG_LINES )
        mEntries.pop_front();

    mEntries.push_back( logline );

    // Add it to the open log window, if any
    if ( !mDialog.isNull() )
        mDialog.data()->add( logline );

    // if log file is open, append to log file
    if (mOutputFile.isOpen()) {
        mOutputFile.write( (logline + "\r\n").toUtf8() );
        mOutputFile.flush();
    }
}
