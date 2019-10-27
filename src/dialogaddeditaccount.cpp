#include <QFileDialog>
#include <QMessageBox>

#include "settings.h"
#include "dialogaddeditaccount.h"
#include "utils.h"
#include "dialogsettings.h"

DialogAddEditAccount::DialogAddEditAccount(bool usemork, QWidget *parent )
    : QDialog(parent), Ui::DialogAddEditAccount()
{
    mMorkParser = usemork;
    setupUi(this);

    if ( !mMorkParser )
    {
        leFolderPath->hide();
        btnBrowse->hide();
        connect(boxAccounts, &QComboBox::currentTextChanged,
                this, &DialogAddEditAccount::onAccountSelected);
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
        buttonBox->button(QDialogButtonBox::Ok)->setDisabled(accounts.isEmpty());
        if ( !account.isEmpty() ) {
            boxAccounts->setCurrentText( account );
        }
    } else {
        leFolderPath->setText( account );
    }

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
    QString path = leFolderPath->text();

    QString e = QFileDialog::getOpenFileName(
            nullptr, tr("Choose the MSF file"),
            path.isEmpty() ? Utils::expandPath(THUNDERBIRD_PROFILES_PATH) : path,
            tr("Mail Index (*.msf)"));

    if ( e.isEmpty() )
        return;

    leFolderPath->setText( QDir::toNativeSeparators(e) );
}

void DialogAddEditAccount::accept()
{
    if ( mMorkParser )
    {
        if ( leFolderPath->text().isEmpty() || !QFile::exists( leFolderPath->text() ) )
        {
            QMessageBox::critical(nullptr, tr("Invalid MSF file"),
                                  tr("You must specify a valid, non-empty Thunderbird index file"));
            return;
        }
    } else if (boxAccounts->currentText().isEmpty()) {
        QMessageBox::critical(nullptr, tr("No account selected"), tr("You must select an account"));
        return;
    }

    QDialog::accept();
}

void DialogAddEditAccount::onAccountSelected(const QString &accountUri) {
    buttonBox->button(QDialogButtonBox::Ok)->setDisabled(accountUri.isEmpty());
}
