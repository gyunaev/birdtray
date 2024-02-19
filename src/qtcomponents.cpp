#include "qtcomponents.h"

QTreeViewWithKeyEvents::QTreeViewWithKeyEvents( QWidget *parent )
    : QTreeView(parent)
{

}

void QTreeViewWithKeyEvents::onKeyPressed( void * handle, TreeViewKeyPressedEvent callback)
{
    mHandle = handle;
    mCallback = callback;
}

void QTreeViewWithKeyEvents::keyPressEvent( QKeyEvent *event )
{
    if (mCallback != NULL && mHandle != NULL)
    {
        mCallback(mHandle, event);
    }
    QTreeView::keyPressEvent(event);
}