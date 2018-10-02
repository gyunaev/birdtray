#include <QMessageBox>
#include "dialogaddeditnewemail.h"

DialogAddEditNewEmail::DialogAddEditNewEmail()
    : QDialog(), Ui::DialogAddEditNewEmail()
{
    setupUi( this );

    connect( buttonBox, &QDialogButtonBox::accepted, this, &DialogAddEditNewEmail::accept );
    connect( buttonBox, &QDialogButtonBox::rejected, this, &DialogAddEditNewEmail::reject );
}

void DialogAddEditNewEmail::accept()
{
    if ( leMenuEntry->text().isEmpty() )
    {
        QMessageBox::critical( 0,
                               tr("Empty menu entry"),
                               tr("Menu entry cannot be empty") );
        leMenuEntry->setFocus();
        return;
    }

    QDialog::accept();
}
