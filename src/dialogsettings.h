#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QProgressDialog>
#include <QPalette>

#include "databaseaccounts.h"
#include "ui_dialogsettings.h"

class AccountTreeModel;

class DialogSettings : public QDialog, public Ui::DialogSettings
{
    Q_OBJECT

    public:
        explicit DialogSettings(QWidget *parent = 0);

    public slots:
        void    accept() override;
        void    browsePath();
        void    profilePathChanged();

        // Calls the database fixer running in a DatabaseFixer thread
        // Receives databaseUnreadsFixed() once fixed
        void    fixDatabaseUnreads();
        void    databaseUnreadsUpdate( int progresspercentage );
        void    databaseUnreadsFixed(QString errorMsg);

        // Tab activation (to refresh accounts)
        // Calls the account query running in a DatabaseAccounts thread
        void    activateTab(int tab );

        // Receives accountsAvailable()
        void    accountsAvailable(QString errorMsg);

        // Account buttons
        void    accountAdd();
        void    accountEdit();
        void    accountEditIndex( const QModelIndex& index );
        void    accountRemove();

    private:
        bool    isProfilePathValid();

        QPalette mPaletteOk;
        QPalette mPaletteErrror;

        // For database fixer
        QProgressDialog * mProgressFixer;

        // List of all accounts
        QList<DatabaseAccounts::Account>    mAccounts;

        // Model to show the accounts
        AccountTreeModel    *   mAccountModel;
};

#endif // SETTINGSDIALOG_H
