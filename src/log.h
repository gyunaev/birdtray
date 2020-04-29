#ifndef LOG_H
#define LOG_H

#include <QMutex>
#include <QPointer>
#include <QStringList>
#include <QFile>

class DialogLogOutput;

// Logger
class Log final
{
    public:
        // Adds a log entry and terminates an app, writing the log buffer to log.txt
        Q_NORETURN static void    fatal( const QString& str );

        // Adds a debug log entry
        static void    debug( const char * fmt, ... ) Q_ATTRIBUTE_FORMAT_PRINTF(1, 2);

        // Shows the logging widget
        static void    showLogger();

        // Initializes log with/without a file output
        static void    initialize( const QString& path = "" );

    private:
        // Only keep this number of last log lines in buffer
        static const int MAX_LOG_LINES = 500;

        static Log *    mLog;

        // Singleton getter
        static Log *g();

        // Add a log entry
        void    add( const QString& text );

        // Internal stuff
        Log()  = default;
        ~Log() = default;
        Log( const Log& l) = delete;
        Log operator= ( const Log& l) = delete;

        // All historic log entries are stored here, guarded by mMutex
        QStringList     mEntries;
        QMutex          mMutex;
        QFile           mOutputFile;

        // Auto-set to null when log dialog is closed
        QPointer<DialogLogOutput>   mDialog;
};

#endif // LOG_H
