#include "dialoglogoutput.h"


DialogLogOutput::DialogLogOutput(QWidget *parent) :
    QDialog(parent)
{
    setupUi(this);

    // Auto-delete this object when closed
    setAttribute( Qt::WA_DeleteOnClose );
}

DialogLogOutput::~DialogLogOutput()
{
}

void DialogLogOutput::add(const QString &line)
{
    logViewer->appendPlainText( line );
    logViewer->ensureCursorVisible();
}

void DialogLogOutput::add(const QStringList &lines)
{
    logViewer->appendPlainText( lines.join("\r\n") );
    logViewer->ensureCursorVisible();
}
