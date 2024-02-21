#ifndef QTCOMPONENTS_H
#define QTCOMPONENTS_H

#include <QKeyEvent>
#include <QTreeView>

typedef void ( * TreeViewKeyPressedEvent)(void * handle, QKeyEvent * event);

/**
 * A QTreeView that allow to define listener of keyEvent received
 */
class QTreeViewWithKeyEvents : public QTreeView {
    Q_OBJECT

    public:
        explicit QTreeViewWithKeyEvents(QWidget *parent = NULL);

    signals:
        void onKeyPressed(QKeyEvent* event);

    protected:
        void keyPressEvent(QKeyEvent * event) override;

};

#endif // QTCOMPONENTS_H