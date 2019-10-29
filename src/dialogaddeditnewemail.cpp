#include <QMessageBox>
#include "dialogaddeditnewemail.h"

DialogAddEditNewEmail::DialogAddEditNewEmail()
        : QDialog(), Ui::DialogAddEditNewEmail() {
    setupUi(this);
}

void DialogAddEditNewEmail::accept() {
    if (leMenuEntry->text().isEmpty()) {
        QMessageBox::critical(nullptr,
                              tr("No name specified"),
                              tr("The name cannot be empty"));
        leMenuEntry->setFocus();
        return;
    }
    QDialog::accept();
}
