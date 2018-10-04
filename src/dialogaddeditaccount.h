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
        explicit DialogAddEditAccount( QWidget *parent = 0 );
        ~DialogAddEditAccount();

        // Sets the list of accounts (fills up combobox), sets the current account (if not empty) and color
        void setCurrent( const QList<DatabaseAccounts::Account>& accounts, const QString& account, const QColor& color );

        // Returns the dialog selection
        QString account() const;
        QColor color() const;

    public slots:
        void    browse();
        void    accept();
};

#endif // DIALOGADDEDITACCOUNT_H
