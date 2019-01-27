#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QProgressDialog>
#include <QPalette>

#include "databaseaccounts.h"
#include "ui_dialogsettings.h"

class ModelAccountTree;
class ModelNewEmails;

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
        void    accountAddMultiple();
        void    accountEdit();
        void    accountEditIndex( const QModelIndex& index );
        void    accountRemove();

        // New Email buttons
        void    newEmailAdd();
        void    newEmailEdit();
        void    newEmailEditIndex( const QModelIndex& index );
        void    newEmailRemove();

        // Icon change
        void    buttonChangeIcon();
        void    buttonChangeUnreadIcon();

        // Parser changed
        void    unreadParserChanged( int curr );

    private:
        void    changeIcon(QToolButton * button );
        bool    isProfilePathValid();
        bool    isMorkParserSelected() const;

        QPalette mPaletteOk;
        QPalette mPaletteErrror;

        // For database fixer
        QProgressDialog * mProgressFixer;

        // List of all accounts
        QList<DatabaseAccounts::Account>    mAccounts;

        // Model to show the accounts
        ModelAccountTree    *   mAccountModel;

        // Model to show new emails
        ModelNewEmails      *   mModelNewEmails;

};

#endif // SETTINGSDIALOG_H
