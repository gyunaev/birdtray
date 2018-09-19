#include "dialogaddeditaccount.h"

DialogAddEditAccount::DialogAddEditAccount( QWidget *parent )
    : QDialog(parent), Ui::DialogAddEditAccount()
{
    setupUi(this);
}

DialogAddEditAccount::~DialogAddEditAccount()
{
}

void DialogAddEditAccount::setCurrent(const QList<DatabaseAccounts::Account> &accounts, const QString &account, const QColor &color)
{
    for ( int i = 0; i < accounts.size(); i++ )
        boxAccounts->addItem( accounts[i].uri );

    if ( !account.isEmpty() )
        boxAccounts->setCurrentText( account );

    btnColor->setColor( color );
}

QString DialogAddEditAccount::account() const
{
    return boxAccounts->currentText();
}

QColor DialogAddEditAccount::color() const
{
    return btnColor->color();
}
