#include <QCoreApplication>
#include <QBrush>
#include <QPainter>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>

#include "modelaccounttree.h"
#include "utils.h"
#include "birdtrayapp.h"
#include "unreadmonitor.h"

ModelAccountTree::ModelAccountTree(QObject* parent, QTreeView* treeView)
        : QAbstractItemModel(parent), QStyledItemDelegate(parent) {
    Settings* settings = BirdtrayApp::get()->getSettings();
    // Get the current settings in proper(stored) order
    for (const QString& path : settings->mFolderNotificationList) {
        mAccounts.push_back(path);
        mColors.push_back(settings->mFolderNotificationColors[path]);
    }
    treeView->setModel(this);
    treeView->setItemDelegateForColumn(1, this);
}

int ModelAccountTree::columnCount(const QModelIndex &) const
{
    // We have two columns
    return 2;
}

QVariant ModelAccountTree::data(const QModelIndex &index, int role) const {
    if (index.row() >= 0 && index.row() < mAccounts.size() && index.column() == 0) {
        switch (role) {
        case Qt::DisplayRole: {
            QFileInfo accountMorkFile(mAccounts[index.row()]);
            QString folderName = Utils::getMailFolderName(accountMorkFile);
            QString accountName = Utils::getMailAccountName(accountMorkFile);
            if (accountName.isNull()) {
                accountName = accountMorkFile.fileName();
            }
            if (folderName.isNull()) {
                return accountName;
            }
            return accountName + " [" + folderName + "]";
        }
        case Qt::ToolTipRole: {
            QString warning = BirdtrayApp::get()->getTrayIcon()->getUnreadMonitor()->getWarnings()
                                                .value(mAccounts[index.row()]);
            return warning.isNull() ? mAccounts[index.row()] : warning;
        }
        case Qt::ForegroundRole:
            if (BirdtrayApp::get()->getTrayIcon()->getUnreadMonitor()->getWarnings().contains(
                    mAccounts[index.row()])) {
                return QColor(255, 150, 0, 255);
            }
            return QVariant();
        default:
            break;
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
            return QCoreApplication::translate("ModelAccountTree", "Account");
        } else {
            return QCoreApplication::translate("ModelAccountTree", "Notification color");
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

void ModelAccountTree::addAccount(const QString &path, const QColor &color)
{
    if (path.isEmpty()) {
        return;
    }
    int existingIndex = mAccounts.indexOf(path);
    if (existingIndex != -1) {
        editAccount(createIndex(existingIndex, 0), path, color);
        return;
    }
    // Only this line changed
    beginInsertRows( QModelIndex(), mColors.size(), mColors.size() + 1 );

    mAccounts.push_back( path );
    mColors.push_back( color );

    endInsertRows();
}

void ModelAccountTree::editAccount(const QModelIndex &idx, const QString &path, const QColor &color)
{
    if (path.isEmpty()) {
        return;
    }
    mAccounts[ idx.row() ] = path;
    mColors[ idx.row() ] = color;

    emit dataChanged( createIndex( idx.row(), 0 ),  createIndex( idx.row(), 1 ) );
}

void ModelAccountTree::getAccount(const QModelIndex &idx, QString &path, QColor &color)
{
    path = mAccounts[ idx.row() ];
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
