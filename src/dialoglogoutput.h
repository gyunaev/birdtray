#ifndef DIALOGLOGOUTPUT_H
#define DIALOGLOGOUTPUT_H

#include <QDialog>
#include "ui_dialoglogoutput.h"

class DialogLogOutput : public QDialog, public Ui::DialogLogOutput
{
    Q_OBJECT

    public:
        explicit DialogLogOutput(QWidget *parent = nullptr);
        ~DialogLogOutput();

    public slots:
        void    add( const QString& line );
        void    add( const QStringList& lines );

};

#endif // DIALOGLOGOUTPUT_H
