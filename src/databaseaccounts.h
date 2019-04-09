#ifndef DATABASEACCOUNTS_H
#define DATABASEACCOUNTS_H

#include <QThread>


class DatabaseAccounts : public QThread
{
    Q_OBJECT

    public:
        struct Account
        {
            QString uri;
            qint64  id;
        };

        DatabaseAccounts(const QString &databasePath);
        
        /**
         * Get the path to the database file in the given directory.
         *
         * @param directory The directory containing the database file.
         * @return The path to the database file.
         */
        static const QString getDatabasePath(const QString &directory);

        const QList<Account>& accounts() const;

        void run() override;

    signals:
        void    done( QString errorMsg );

    private:
        void    queryAccounts();

        QString         mDbPath;
        QList<Account>  mAccounts;
};

#endif // DATABASEACCOUNTS_H
