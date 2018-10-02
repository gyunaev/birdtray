#ifndef DIALOGADDEDITNEWEMAIL_H
#define DIALOGADDEDITNEWEMAIL_H

#include <QDialog>
#include "ui_dialogaddeditnewemail.h"

class DialogAddEditNewEmail : public QDialog, public Ui::DialogAddEditNewEmail
{
    Q_OBJECT

    public:
        DialogAddEditNewEmail();

    public slots:
        void    accept();
};

#endif // DIALOGADDEDITNEWEMAIL_H
