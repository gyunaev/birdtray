#ifndef ACCOUNTTREEITEM_H
#define ACCOUNTTREEITEM_H

#include <QList>
#include <QColor>
#include <QAbstractItemModel>

#include "databaseaccounts.h"

class ModelAccountTree : public QAbstractItemModel
{
    public:
        ModelAccountTree( QObject *parent = 0 );

        virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;
        virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
        virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
        virtual QModelIndex parent(const QModelIndex &index) const override;
        virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
        virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
        virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

        void    addAccount( const QString& uri, const QColor& color );
        void    editAccount( const QModelIndex& idx, const QString& uri, const QColor& color );
        void    getAccount( const QModelIndex& idx, QString& uri, QColor& color );
        void    removeAccount( const QModelIndex& idx );
        void    clear();

        // Moves the current accounts/colors to settings
        void    applySettings();

    private:
        QList< QString> mAccounts;
        QList< QColor > mColors;
};

#endif // ACCOUNTTREEITEM_H
