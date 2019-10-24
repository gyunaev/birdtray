#include <QBrush>
#include <QPainter>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>

#include "settings.h"
#include "modelaccounttree.h"
#include "utils.h"

ModelAccountTree::ModelAccountTree(QObject *parent, QTreeView* treeView)
        : QAbstractItemModel( parent ), QItemDelegate(parent)
{
    // Get the current settings in proper(stored) order
    for ( QString uri : pSettings->mFolderNotificationList )
    {
        mAccounts.push_back( uri );
        mColors.push_back( pSettings->mFolderNotificationColors[uri] );
    }
    treeView->setModel(this);
    treeView->setItemDelegateForColumn(1, this);
}

int ModelAccountTree::columnCount(const QModelIndex &) const
{
    // We have two columns
    return 2;
}

QVariant ModelAccountTree::data(const QModelIndex &index, int role) const
{
    if ( index.row() >= 0 && index.row() < mAccounts.size() && index.column() < 2 )
    {
        if ( role == Qt::DisplayRole && index.column() == 0) {
            QString account = mAccounts[index.row()];
            if (!account.endsWith(".msf")) {
                return Utils::decodeIMAPutf7(account);
            }
            QFileInfo fileInfo(account);
            QString folderName = fileInfo.baseName();
            if (folderName == "INBOX") {
                folderName = QObject::tr("Inbox");
            } else {
                folderName = QObject::tr(folderName.toUtf8().constData());
            }
            QString accountName = fileInfo.dir().dirName();
            return accountName + " [" + folderName + "]";
        } else if ( role == Qt::ToolTipRole && index.column() == 0 ) {
            return mAccounts[index.row()];
        }
    }

    return QVariant();
}

QModelIndex ModelAccountTree::index(int row, int column, const QModelIndex &) const
{
    if ( row < 0 || row >= mAccounts.size() || column > 2 )
        return QModelIndex();

    return createIndex( row, column );
}

QModelIndex ModelAccountTree::parent(const QModelIndex &) const
{
    // No item has parent
    return QModelIndex();
}

int ModelAccountTree::rowCount(const QModelIndex &) const
{
    return mAccounts.size();
}

Qt::ItemFlags ModelAccountTree::flags(const QModelIndex &) const
{
    // Same for all items
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant ModelAccountTree::headerData(int section, Qt::Orientation , int role) const
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

void ModelAccountTree::paint(QPainter* painter, const QStyleOptionViewItem &option,
                             const QModelIndex &index) const {
    if (index.column() == 1) {
        painter->fillRect(option.rect.marginsRemoved(QMargins(1, 1, 1, 1)), mColors[index.row()]);
    } else {
        QItemDelegate::paint(painter, option, index);
    }
}

void ModelAccountTree::addAccount(const QString &uri, const QColor &color)
{
    if (uri.isEmpty()) {
        return;
    }
    // Only this line changed
    beginInsertRows( QModelIndex(), mColors.size(), mColors.size() + 1 );

    mAccounts.push_back( uri );
    mColors.push_back( color );

    endInsertRows();
}

void ModelAccountTree::editAccount(const QModelIndex &idx, const QString &uri, const QColor &color)
{
    if (uri.isEmpty()) {
        return;
    }
    mAccounts[ idx.row() ] = uri;
    mColors[ idx.row() ] = color;

    emit dataChanged( createIndex( idx.row(), 0 ),  createIndex( idx.row(), 1 ) );
}

void ModelAccountTree::getAccount(const QModelIndex &idx, QString &uri, QColor &color)
{
    uri = mAccounts[ idx.row() ];
    color = mColors[ idx.row() ];
}

void ModelAccountTree::removeAccount(const QModelIndex &idx)
{
    beginRemoveRows( QModelIndex(), idx.row(), idx.row() );
    mAccounts.removeAt( idx.row() );
    mColors.removeAt( idx.row() );
    endRemoveRows();
}

void ModelAccountTree::clear()
{
    if (mColors.isEmpty()) {
        return;
    }
    beginRemoveRows( QModelIndex(), 0, mColors.size() - 1 );
    mAccounts.clear();
    mColors.clear();
    endRemoveRows();
}

void ModelAccountTree::applySettings()
{
    pSettings->mFolderNotificationColors.clear();
    pSettings->mFolderNotificationList.clear();

    for ( int i = 0; i < mAccounts.size(); i++ )
    {
        pSettings->mFolderNotificationList.push_back( mAccounts[i] );
        pSettings->mFolderNotificationColors[ mAccounts[i] ] = mColors[i];
    }
}
