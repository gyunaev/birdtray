#include <QFileDialog>
#include <QMessageBox>

#include "settings.h"
#include "dialogaddeditaccount.h"

DialogAddEditAccount::DialogAddEditAccount(bool usemork, QWidget *parent )
    : QDialog(parent), Ui::DialogAddEditAccount()
{
    mMorkParser = usemork;
    setupUi(this);

    if ( !mMorkParser )
    {
        leFolderPath->hide();
        btnBrowse->hide();
    }
    else
    {
        boxAccounts->hide();
        connect( btnBrowse, &QPushButton::clicked, this, &DialogAddEditAccount::browse );
    }
}

DialogAddEditAccount::~DialogAddEditAccount()
{
}

void DialogAddEditAccount::setCurrent(const QList<DatabaseAccounts::Account> &accounts, const QString &account, const QColor &color)
{
    if ( !mMorkParser )
    {
        for ( int i = 0; i < accounts.size(); i++ )
            boxAccounts->addItem( accounts[i].uri );

        if ( !account.isEmpty() )
            boxAccounts->setCurrentText( account );
    }
    else
        leFolderPath->setText( account );

    btnColor->setColor( color );
}

QString DialogAddEditAccount::account() const
{
    if ( mMorkParser )
        return leFolderPath->text();
    else
        return boxAccounts->currentText();
}

QColor DialogAddEditAccount::color() const
{
    return btnColor->color();
}

void DialogAddEditAccount::browse()
{
    QString e = QFileDialog::getOpenFileName( 0,
                                              "Choose the MSF file",
                                              "",
                                              "Mail Index (*.msf)" );

    if ( e.isEmpty() )
        return;

    leFolderPath->setText( e );
}

void DialogAddEditAccount::accept()
{
    if ( mMorkParser )
    {
        if ( leFolderPath->text().isEmpty() || !QFile::exists( leFolderPath->text() ) )
        {
            QMessageBox::critical( 0, "Invalid MSF file", tr("You must specify valid, non-empty Thunderbird index file") );
            return;
        }
    }

    QDialog::accept();
}
