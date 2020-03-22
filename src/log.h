#ifndef LOG_H
#define LOG_H

#include <QMutex>
#include <QPointer>
#include <QStringList>

class DialogLogOutput;

// Logger
class Log
{
    public:
        // Adds a log entry and terminates an app, writing the log buffer to log.txt
        static void    fatal( const QString& str );

        // Adds a debug log entry
        static void    debug( const char * fmt, ... );

        // Shows the logging widget
        static void    showLogger();

    private:
        // Only keep this number of last log lines in buffer
        static const int MAX_LOG_LINES = 500;

        static Log *    mLog;

        // Singleton getter
        static Log *g();

        // Add a log entry
        void    add( const QString& text );

        // Internal stuff
        Log();
        ~Log();
        Log( const Log& l) = delete;
        Log operator= ( const Log& l) = delete;

        // All historic log entries are stored here, guarded by mMutex
        QStringList     mEntries;
        QMutex          mMutex;

        // Auto-set to null when log dialog is closed
        QPointer<DialogLogOutput>   mDialog;
};

#endif // LOG_H
