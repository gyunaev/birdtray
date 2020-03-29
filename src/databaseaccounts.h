#ifndef DATABASEACCOUNTS_H
#define DATABASEACCOUNTS_H

#include <QString>

namespace DatabaseAccounts
{
    struct Account
    {
        QString uri;
        qint64  id;
    };
};

#endif // DATABASEACCOUNTS_H
