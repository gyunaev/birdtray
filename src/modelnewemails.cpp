#include "modelnewemails.h"
#include "settings.h"
#include "dialogaddeditnewemail.h"

ModelNewEmails::ModelNewEmails(QObject *parent)
    : QAbstractItemModel( parent )
{
    mNewEmailData = pSettings->mNewEmailData;
}

int ModelNewEmails::columnCount(const QModelIndex &) const
{
    return 1;
}

QVariant ModelNewEmails::data(const QModelIndex &index, int role) const
{
    if ( index.row() >= 0 && index.row() < mNewEmailData.size() && index.column() == 0 )
    {
        if ( role == Qt::DisplayRole )
            return mNewEmailData[index.row()].menuentry();
    }

    return QVariant();
}

QModelIndex ModelNewEmails::index(int row, int column, const QModelIndex &) const
{
    if ( row < 0 || row >= mNewEmailData.size() || column > 1 )
        return QModelIndex();

    return createIndex( row, column );
}

QModelIndex ModelNewEmails::parent(const QModelIndex &) const
{
    return QModelIndex();
}

int ModelNewEmails::rowCount(const QModelIndex &) const
{
    return mNewEmailData.size();
}

Qt::ItemFlags ModelNewEmails::flags(const QModelIndex &) const
{
    // Same for all items
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant ModelNewEmails::headerData(int section, Qt::Orientation , int role) const
{
    if ( role == Qt::DisplayRole )
    {
        if ( section == 0 )
            return "Menu entry item";
    }

    return QVariant();
}

bool ModelNewEmails::add() {
    Setting_NewEmail item;
    if (!item.edit()) {
        return false;
    }
    // Only this line changed
    beginInsertRows(QModelIndex(), mNewEmailData.size(), mNewEmailData.size() + 1);
    mNewEmailData.push_back(item);
    endInsertRows();
    return true;
}

void ModelNewEmails::edit(const QModelIndex &idx)
{
    if ( idx.isValid() && mNewEmailData[ idx.row() ].edit() )
        emit dataChanged( createIndex( idx.row(), 0 ),  createIndex( idx.row(), 1 ) );
}

void ModelNewEmails::remove(const QModelIndex &idx)
{
    if ( !idx.isValid() )
        return;

    beginRemoveRows( QModelIndex(), idx.row(), idx.row() );
    mNewEmailData.removeAt( idx.row() );
    endRemoveRows();
}

void ModelNewEmails::applySettings()
{
    pSettings->mNewEmailData = mNewEmailData;
}
