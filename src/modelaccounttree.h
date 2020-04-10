#ifndef ACCOUNTTREEITEM_H
#define ACCOUNTTREEITEM_H

#include <QList>
#include <QColor>
#include <QAbstractItemModel>
#include <QtWidgets/QTreeView>
#include <QtWidgets/QStyledItemDelegate>

class ModelAccountTree : public QAbstractItemModel, public QStyledItemDelegate
{
    public:
        /**
         * A model that contains information about mail accounts registered to watch.
         * @param parent The parent of this widget.
         * @param treeView The widget that displays this model.
         */
        explicit ModelAccountTree(QObject *parent = nullptr, QTreeView* treeView = nullptr);

        virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;
        virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
        virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
        virtual QModelIndex parent(const QModelIndex &index) const override;
        virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
        virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
        virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
        void paint(QPainter* painter, const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;

        void    addAccount(const QString& path, const QColor& color );
        void    editAccount(const QModelIndex& idx, const QString& path, const QColor& color );
        void    getAccount(const QModelIndex& idx, QString& path, QColor& color );
        void    removeAccount( const QModelIndex& idx );
        void    clear();

        // Moves the current accounts/colors to settings
        void    applySettings();

    private:
        QList< QString> mAccounts;
        QList< QColor > mColors;
};

#endif // ACCOUNTTREEITEM_H
