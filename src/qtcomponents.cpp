#include "qtcomponents.h"

QTreeViewWithKeyEvents::QTreeViewWithKeyEvents( QWidget *parent )
    : QTreeView(parent)
{

}

void QTreeViewWithKeyEvents::keyPressEvent( QKeyEvent *event )
{
    emit onKeyPressed(event);
    QTreeView::keyPressEvent(event);
}