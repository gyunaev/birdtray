#ifndef DIALOG_ADD_EDIT_NEW_EMAIL_H
#define DIALOG_ADD_EDIT_NEW_EMAIL_H

#include <QDialog>
#include "ui_dialogaddeditnewemail.h"

/**
 * A dialog that allows editing of the "New Email" entries.
 */
class DialogAddEditNewEmail : public QDialog, public Ui::DialogAddEditNewEmail {
Q_OBJECT

public:
    DialogAddEditNewEmail();

public slots:
    
    void accept() override;
};

#endif // DIALOG_ADD_EDIT_NEW_EMAIL_H
