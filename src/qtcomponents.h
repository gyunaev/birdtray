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

    public slots:
        void onKeyPressed(void * handleUsed, TreeViewKeyPressedEvent callback);

    protected:
        void keyPressEvent(QKeyEvent * event) override;

    private:
        TreeViewKeyPressedEvent mCallback;
        void * mHandle;
};

#endif // QTCOMPONENTS_H