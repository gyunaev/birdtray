#include <QBrush>

#include "settings.h"
#include "accounttreemodel.h"

AccountTreeModel::AccountTreeModel( QObject *parent )
    : QAbstractItemModel( parent )
{
    // Get the current settings
    mAccounts = pSettings->mFolderNotificationColors.keys();
    mColors = pSettings->mFolderNotificationColors.values();
}

int AccountTreeModel::columnCount(const QModelIndex &) const
{
    // We have two columns
    return 2;
}

QVariant AccountTreeModel::data(const QModelIndex &index, int role) const
{
    if ( index.row() >= 0 && index.row() < mAccounts.size() && index.column() < 2 )
    {
        if ( role == Qt::DisplayRole )
        {
            if ( index.column() == 0 )
                return mAccounts[index.row()];
            else
                return "uses this color";
        }

        if ( role == Qt::BackgroundRole && index.column() == 1 )
            return QBrush( mColors[index.row()] );
    }

    return QVariant();
}

QModelIndex AccountTreeModel::index(int row, int column, const QModelIndex &) const
{
    if ( row < 0 || row >= mAccounts.size() || column > 2 )
        return QModelIndex();

    return createIndex( row, column );
}

QModelIndex AccountTreeModel::parent(const QModelIndex &) const
{
    // No item has parent
    return QModelIndex();
}

int AccountTreeModel::rowCount(const QModelIndex &) const
{
    return mAccounts.size();
}

Qt::ItemFlags AccountTreeModel::flags(const QModelIndex &) const
{
    // Same for all items
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant AccountTreeModel::headerData(int section, Qt::Orientation , int role) const
{
    if ( role == Qt::DisplayRole )
    {
        if ( section == 0 )
            return "Account";
        else
            return "Notification color";
    }

    return QVariant();
}

void AccountTreeModel::addAccount(const QString &uri, const QColor &color)
{
    // Only this line changed
    beginInsertRows( QModelIndex(), mColors.size(), mColors.size() + 1 );

    mAccounts.push_back( uri );
    mColors.push_back( color );

    endInsertRows();

    // For edit: dataChanged( createIndex( 0, 0 ),  createIndex( mColors.size(), 1 ) );
}

void AccountTreeModel::removeAccount(const QModelIndex &idx)
{
    beginRemoveRows( QModelIndex(), idx.row(), idx.row() );
    mAccounts.removeAt( idx.row() );
    mColors.removeAt( idx.row() );
    endRemoveRows();
}

void AccountTreeModel::applySettings()
{
    pSettings->mFolderNotificationColors.clear();

    for ( int i = 0; i < mAccounts.size(); i++ )
    {
        pSettings->mFolderNotificationColors[ mAccounts[i] ] = mColors[i];
    }
}
