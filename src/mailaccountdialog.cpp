#include <utility>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QFileDialog>
#include <QtCore/QDirIterator>
#include <QtGui/QPainter>
#include <QtGui/QKeyEvent>
#include "mailaccountdialog.h"
#include "ui_mailaccountdialog.h"
#include "utils.h"
#include "colorbutton.h"

MailAccountDialog::MailAccountDialog(QWidget* parent, QColor defaultColor) :
        QDialog(parent),
        ui(new Ui::MailAccountDialog),
        defaultColor(std::move(defaultColor)) {
    ui->setupUi(this);
    connect(ui->tbProfilesBrowseButton, &QAbstractButton::clicked,
            this, &MailAccountDialog::onProfilesDirBrowseButtonClicked);
    connect(ui->tbProfilesPathEdit, &QLineEdit::editingFinished,
            this, &MailAccountDialog::loadProfiles);
    connect(ui->accountsList, &QTreeWidget::itemChanged, &MailAccountDialog::onAccountItemChanged);
    for (const QString &path : Utils::getThunderbirdProfilesPaths()) {
        if (QDir(Utils::expandPath(path)).exists()) {
            ui->tbProfilesPathEdit->setText(path);
            loadProfiles();
            break;
        }
    }
}

MailAccountDialog::~MailAccountDialog() {
    delete ui;
}

void MailAccountDialog::getSelectedAccounts(QList<std::tuple<QString, QColor>> &outList) const {
    QTreeWidgetItemIterator iterator = iterateCheckedAccountItems();
    for (; *iterator != nullptr; ++iterator) {
        QTreeWidgetItem* selectedItem = *iterator;
        QVariant color = selectedItem->data(1, Qt::UserRole);
        outList.append(std::make_tuple(
                selectedItem->data(0, Qt::UserRole).toString(),
                color.isNull() ? defaultColor : color.value<QColor>()));
    }
}

void MailAccountDialog::accept() {
    if (*iterateCheckedAccountItems() != nullptr || QMessageBox::warning(
            this, tr("No folder selected"),
            tr("No mail folder was selected to monitor.\nDo you want to continue?"),
            QMessageBox::StandardButton::Ok | QMessageBox::StandardButton::Cancel
    ) == QMessageBox::StandardButton::Ok) {
        QDialog::accept();
    }
}

void MailAccountDialog::keyPressEvent(QKeyEvent* event) {
    switch (event->key()) {
    case Qt::Key_Return:
    case Qt::Key_Enter:
        break; // Prevent enter from closing the dialog
    default:
        QDialog::keyPressEvent(event);
    }
}

void MailAccountDialog::onProfilesDirBrowseButtonClicked() {
    QString directory = QFileDialog::getExistingDirectory(
            nullptr, tr("Choose the Thunderbird profiles path"),
            Utils::expandPath(ui->tbProfilesPathEdit->text()), QFileDialog::ShowDirsOnly);
    if (directory.isEmpty()) {
        return;
    }
    ui->tbProfilesPathEdit->setText(QDir::toNativeSeparators(directory));
    loadProfiles();
}

void MailAccountDialog::onAccountItemChanged(QTreeWidgetItem* item, int column) {
    if (column != 0) {
        return;
    }
    Qt::CheckState checkState = item->checkState(column);
    if (checkState != Qt::PartiallyChecked) {
        for (int i = 0; i < item->childCount(); i++) {
            item->child(i)->setCheckState(column, checkState);
        }
    }
    QTreeWidgetItem* parent = item->parent();
    if (parent == nullptr) {
        return;
    }
    bool isPartiallySelected = checkState == Qt::PartiallyChecked;
    if (!isPartiallySelected) {
        for (int i = 0; i < parent->childCount(); i++) {
            if (parent->child(i)->checkState(column) != checkState) {
                isPartiallySelected = true;
                break;
            }
        }
    }
    parent->setCheckState(column, isPartiallySelected ? Qt::PartiallyChecked : checkState);
}

void MailAccountDialog::loadProfiles() {
    if (profilesDirPath == ui->tbProfilesPathEdit->text()) {
        return;
    }
    profilesDirPath = ui->tbProfilesPathEdit->text();
    ui->accountsList->clear();
    QDir profilesDir(Utils::expandPath(profilesDirPath));
    bool foundAValidProfileDir = false;
    for (const QString &profileDirName: profilesDir.entryList(
            QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot | QDir::Filter::Hidden)) {
        const QString profileDir = profilesDir.absoluteFilePath(profileDirName);
        const QStringList mailFolders = getMailFoldersFor(profileDir);
        if (mailFolders.isEmpty()) {
            continue;
        }
        foundAValidProfileDir = true;
        auto* profileItem = new QTreeWidgetItem(
                ui->accountsList, {tr("%1 (Profile)").arg(getProfileName(profileDirName))});
        profileItem->setExpanded(true);
        ui->accountsList->addTopLevelItem(profileItem);
        loadAccounts(profileItem, mailFolders);
    }
    if (!foundAValidProfileDir) {
        if (!getMailFoldersFor(profilesDirPath).isEmpty()) {
            profilesDir.cdUp();
            ui->tbProfilesPathEdit->setText(QDir::toNativeSeparators(profilesDir.path()));
            loadProfiles();
        }
    }
}

void MailAccountDialog::loadAccounts(QTreeWidgetItem* profileTreeItem,
                                     const QStringList &mailFolders) {
    for (const QString &mailDirPath : mailFolders) {
        QDir mailDir(mailDirPath);
        for (const QString &mailAccount : mailDir.entryList(
                QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot | QDir::Filter::Hidden)) {
            QString accountDirectoryPath = mailDir.absoluteFilePath(mailAccount);
            QDirIterator msfFileIterator(accountDirectoryPath, {"*.msf"},
                    QDir::Files, QDirIterator::Subdirectories);
            if (!msfFileIterator.hasNext()) {
                continue;
            }
            auto* accountItem = new QTreeWidgetItem(profileTreeItem, {mailAccount});
            accountItem->setExpanded(true);
            if (msfFileIterator.hasNext()) {
                accountItem->setCheckState(0, Qt::Unchecked);
                profileTreeItem->setCheckState(0, Qt::Unchecked);
            }
            profileTreeItem->addChild(accountItem);
            while (msfFileIterator.hasNext()) {
                (void) msfFileIterator.next();
                QFileInfo msfFile = msfFileIterator.fileInfo();
                QString name = Utils::getMailFolderName(msfFile);
                if (name.isNull()) {
                    continue;
                }
                auto* folderItem = new QTreeWidgetItem(accountItem, {name});
                if (QString::compare(
                        msfFile.completeBaseName(), "INBOX", Qt::CaseInsensitive) == 0) {
                    QFont font = folderItem->font(0);
                    font.setBold(true);
                    folderItem->setFont(0, font);
                }
                folderItem->setCheckState(0, Qt::Unchecked);
                folderItem->setData(0, Qt::UserRole, msfFile.absoluteFilePath());
                auto* colorButton = new ColorButton(ui->accountsList, defaultColor);
                colorButton->setBorderlessMode(true);
                connect(colorButton, &ColorButton::onColorChanged, this, [=](const QColor &color) {
                    folderItem->setData(1, Qt::UserRole, color);
                });
                ui->accountsList->setItemWidget(folderItem, 1, colorButton);
                accountItem->addChild(folderItem);
            }
        }
    }
}

QTreeWidgetItemIterator MailAccountDialog::iterateCheckedAccountItems() const {
    return QTreeWidgetItemIterator(
            ui->accountsList,
            QTreeWidgetItemIterator::Checked | QTreeWidgetItemIterator::NoChildren);
}

QStringList MailAccountDialog::getMailFoldersFor(const QString &profileDirPath) {
    QDir profileDir(profileDirPath);
    QStringList mailFolders = profileDir.entryList(
            {"*Mail", "ExQuilla"}, QDir::Dirs | QDir::Hidden);
    std::transform(mailFolders.begin(), mailFolders.end(), mailFolders.begin(),
            [=](const QString &folder) { return profileDir.absoluteFilePath(folder); });
    return mailFolders;
}

QString MailAccountDialog::getProfileName(const QString &profileDirName) {
    return profileDirName.mid(profileDirName.indexOf('.') + 1);
}

void AccountsTreeWidget::paintEvent(QPaintEvent* event) {
    if (model() && model()->rowCount() > 0) {
        QTreeView::paintEvent(event);
        return;
    }
    QPainter painter(viewport());
    QFont font = painter.font();
    font.setPointSizeF(font.pointSizeF() * 1.5);
    painter.setFont(font);
    painter.setPen(Qt::gray);
    QRect textRect = painter.fontMetrics().boundingRect(viewport()->rect(),
            Qt::AlignCenter | Qt::TextWordWrap, _emptyText);
    textRect.moveCenter(viewport()->rect().center());
    painter.drawText(textRect, Qt::AlignCenter | Qt::TextWordWrap, _emptyText);
}
