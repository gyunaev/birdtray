#ifndef DIALOGADDEDITACCOUNT_H
#define DIALOGADDEDITACCOUNT_H

#include <QDialog>
#include <QColor>

#include "databaseaccounts.h"
#include "ui_dialogaddeditaccount.h"

class DialogAddEditAccount : public QDialog, public Ui::DialogAddEditAccount
{
    Q_OBJECT

    public:
        explicit DialogAddEditAccount( bool usemork, QWidget *parent = 0 );
        ~DialogAddEditAccount();

        // Sets the list of accounts (fills up combobox), sets the current account (if not empty) and color
        void setCurrent( const QList<DatabaseAccounts::Account>& accounts, const QString& account, const QColor& color );

        // Returns the dialog selection
        QString account() const;
        QColor color() const;

    public slots:
        void    browse();
        void    accept();
        
        /**
         * Called when an account is selected in the combobox.
         *
         * @param accountUri The uri of the account.
         */
        void    onAccountSelected(const QString& accountUri);

    private:
        bool    mMorkParser;
};

#endif // DIALOGADDEDITACCOUNT_H
