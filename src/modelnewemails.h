#ifndef MODELNEWEMAILS_H
#define MODELNEWEMAILS_H

#include <QAbstractItemModel>
#include "setting_newemail.h"

class ModelNewEmails : public QAbstractItemModel
{
    public:
        ModelNewEmails( QObject * parent );

        virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;
        virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
        virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
        virtual QModelIndex parent(const QModelIndex &index) const override;
        virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
        virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
        virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

        /**
         * Add a new entry.
         *
         * @return True, if a new entry was added.
         */
        bool    add();
        void    edit( const QModelIndex &idx );
        void    remove( const QModelIndex& idx );

        // Moves the current data to settings
        void    applySettings();

    private:
        QList< Setting_NewEmail >   mNewEmailData;

};

#endif // MODELNEWEMAILS_H
