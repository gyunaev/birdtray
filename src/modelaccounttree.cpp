#include <QCoreApplication>
#include <QBrush>
#include <QPainter>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>

#include "modelaccounttree.h"
#include "utils.h"
#include "birdtrayapp.h"

ModelAccountTree::ModelAccountTree(QObject* parent, QTreeView* treeView)
        : QAbstractItemModel(parent), QStyledItemDelegate(parent) {
    Settings* settings = BirdtrayApp::get()->getSettings();
    // Get the current settings in proper(stored) order
    for (const QString& uri : settings->mFolderNotificationList) {
        mAccounts.push_back(uri);
        mColors.push_back(settings->mFolderNotificationColors[uri]);
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
            QString folderName = Utils::getMailFolderName(fileInfo);
            QString accountName = Utils::getMailAccountName(fileInfo);
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
        if (section == 0) {
            return QObject::tr("Account");
        } else {
            return QObject::tr("Notification color");
        }
    }

    return QVariant();
}

void ModelAccountTree::paint(QPainter* painter, const QStyleOptionViewItem &option,
                             const QModelIndex &index) const {
    QStyledItemDelegate::paint(painter, option, index);
    if (index.column() == 1) {
        painter->fillRect(option.rect.marginsRemoved(QMargins(1, 1, 1, 1)), mColors[index.row()]);
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

void ModelAccountTree::applySettings() {
    Settings* settings = BirdtrayApp::get()->getSettings();
    settings->mFolderNotificationColors.clear();
    settings->mFolderNotificationList.clear();

    for (int i = 0; i < mAccounts.size(); i++) {
        settings->mFolderNotificationList.push_back(mAccounts[i]);
        settings->mFolderNotificationColors[mAccounts[i]] = mColors[i];
    }
}
