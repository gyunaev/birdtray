#ifndef DATABASEUNREADFIXER_H
#define DATABASEUNREADFIXER_H

#include <QThread>

class DatabaseUnreadFixer : public QThread
{
    Q_OBJECT

    public:
        DatabaseUnreadFixer( const QString& databasePath );

    protected:
        virtual void run() override;
        void    updateDatabase();

    signals:
        void    done( QString errorMsg );
        void    progress( int percentage );

    private:
        QString mDbPath;
};

#endif // DATABASEUNREADFIXER_H
