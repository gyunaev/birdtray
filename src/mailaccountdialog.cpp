#include <utility>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QFileDialog>
#include <QtCore/QDirIterator>
#include "mailaccountdialog.h"
#include "ui_mailaccountdialog.h"
#include "utils.h"
#include "colorbutton.h"

MailAccountDialog::MailAccountDialog(QWidget* parent, QColor defaultColor) :
        QWizard(parent),
        ui(new Ui::MailAccountDialog),
        defaultColor(std::move(defaultColor)) {
    ui->setupUi(this);
    connect(this, &QWizard::currentIdChanged, this, &MailAccountDialog::onCurrentIdChanged);
    connect(ui->tbProfilesBrowseButton, &QAbstractButton::clicked,
            this, &MailAccountDialog::onProfilesDirBrowseButtonClicked);
    connect(ui->tbProfilesPathEdit, &QLineEdit::editingFinished,
            this, &MailAccountDialog::onProfilesDirEditCommit);
    connect(ui->profileSelector,
            static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &MailAccountDialog::onProfileSelectionChanged);
    connect(ui->accountsList, &QTreeWidget::itemChanged, &MailAccountDialog::onAccountItemChanged);
}

MailAccountDialog::~MailAccountDialog() {
    delete ui;
    delete profilesDir;
}

void MailAccountDialog::initializePage(int id) {
    QWizard::initializePage(id);
    switch (static_cast<PageId>(id)) {
    case profilesDirPage:
        initializeProfilesDirPage();
        break;
    case profilePage:
        initializeTbProfilesPage();
        break;
    case accountsPage:
        initializeAccountsPage();
        break;
    case noPage: break;
    }
}

bool MailAccountDialog::validateCurrentPage() {
    switch (static_cast<PageId>(currentId())) {
    case profilesDirPage: return validateProfilesDirPage();
    case profilePage: return validateTbProfilesPage();
    case accountsPage: return validateAccountsPage();
    case noPage:break;
    }
    return QWizard::validateCurrentPage();
}

void MailAccountDialog::getSelectedAccounts(QList<std::tuple<QString, QColor>> &outList) const {
    for (QTreeWidgetItem* selectedItem : getCheckedAccountItems()) {
        QVariant color = selectedItem->data(1, Qt::UserRole);
        outList.append(std::make_tuple(
                selectedItem->data(0, Qt::UserRole).toString(),
                color.isNull() ? defaultColor : color.value<QColor>()));
    }
}

void MailAccountDialog::onCurrentIdChanged(int id) {
    bool skipPage = false;
    switch (static_cast<PageId>(id)) {
    case profilesDirPage:
        skipPage = furthestPage < PageId::profilesDirPage && profilesDir != nullptr;
        break;
    case profilePage:
        skipPage = furthestPage < PageId::profilePage && !thunderbirdProfileMailDirs.isEmpty();
        break;
    case accountsPage:
    case noPage: break;
    }
    if (furthestPage < id) {
        furthestPage = static_cast<PageId>(id);
    }
    if (skipPage) {
        next();
    }
}

void MailAccountDialog::onProfilesDirBrowseButtonClicked() {
    QString directory = QFileDialog::getExistingDirectory(
            nullptr, tr("Choose the Thunderbird profiles path"),
            Utils::expandPath(ui->tbProfilesPathEdit->text()), QFileDialog::ShowDirsOnly);
    if (directory.isEmpty()) {
        return;
    }
    QDir* newProfilesDir = new QDir(directory);
    if (!newProfilesDir->exists()) {
        delete newProfilesDir;
        return;
    } else {
        delete profilesDir;
        profilesDir = newProfilesDir;
    }
    ui->tbProfilesPathEdit->setText(QDir::toNativeSeparators(directory));
}

void MailAccountDialog::onProfilesDirEditCommit() {
    QDir* newProfilesDir = new QDir(Utils::expandPath(ui->tbProfilesPathEdit->text()));
    if (!newProfilesDir->exists()) {
        delete newProfilesDir;
    } else {
        delete profilesDir;
        profilesDir = newProfilesDir;
    }
}

void MailAccountDialog::onProfileSelectionChanged(int newProfileIndex) {
    Q_UNUSED(newProfileIndex)
    thunderbirdProfileMailDirs.clear();
}

void MailAccountDialog::onAccountItemChanged(QTreeWidgetItem* item, int column) {
    if (column != 0) {
        return;
    }
    propagateChangesToAccountChildren(item, item->checkState(column));
}

void MailAccountDialog::initializeProfilesDirPage() {
    if (profilesDir != nullptr) {
        return;
    }
    QString profilesPath;
    for (const QString &path : Utils::getThunderbirdProfilesPaths()) {
        profilesDir = new QDir(Utils::expandPath(path));
        if (profilesDir->exists()) {
            profilesPath = path;
            break;
        } else {
            delete profilesDir;
            profilesDir = nullptr;
        }
    }
    if (profilesDir != nullptr) {
        ui->tbProfilesPathEdit->setText(profilesPath);
    }
}

bool MailAccountDialog::validateProfilesDirPage() {
    if (profilesDir == nullptr) {
        QMessageBox::warning(this, tr("Invalid Thunderbird profile directory"),
                             tr("Please enter the path to a valid directory."),
                             QMessageBox::StandardButton::Ok);
        return false;
    }
    if (profilesDir->entryList(QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot).isEmpty()) {
        QMessageBox::warning(this, tr("Invalid Thunderbird profile directory"),
                             tr("The Thunderbird profile directory contains no profiles."),
                             QMessageBox::StandardButton::Ok);
        return false;
    }
    return true;
}

void MailAccountDialog::initializeTbProfilesPage() {
    if (profilesDir == nullptr) {
        return;
    }
    thunderbirdProfileMailDirs.clear();
    QStringList profileDirs = profilesDir->entryList(
            QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot);
    // If we don't block the signals, Qt crashes on clear or addItem.
    bool oldState = ui->profileSelector->blockSignals(true);
    ui->profileSelector->clear();
    for (const QString &profileDirName : profileDirs) {
        QDir profileDir(profilesDir->absoluteFilePath(profileDirName));
        QStringList mailFolders = profileDir.entryList({"*Mail"}, QDir::Dirs);
        if (!mailFolders.isEmpty()) {
            std::transform(mailFolders.begin(), mailFolders.end(), mailFolders.begin(),
                    [=](const QString& folder) { return profileDir.absoluteFilePath(folder); });
            ui->profileSelector->addItem(
                    profileDirName.mid(profileDirName.indexOf('.') + 1), mailFolders);
        }
    }
    ui->profileSelector->blockSignals(oldState);
    if (ui->profileSelector->count() == 1) {
        for (const QString &selectedDir : ui->profileSelector->itemData(0).toStringList()) {
            thunderbirdProfileMailDirs.append(selectedDir);
        }
    }
}

bool MailAccountDialog::validateTbProfilesPage() {
    if (thunderbirdProfileMailDirs.isEmpty()) {
        QVariant selected = ui->profileSelector->itemData(ui->profileSelector->currentIndex());
        if (selected.isNull()) {
            QMessageBox::warning(this, tr("Invalid Thunderbird profile"),
                                 tr("Please select a valid Thunderbird profile."),
                                 QMessageBox::StandardButton::Ok);
            return false;
        }
        for (const QString &selectedDir : selected.toStringList()) {
            thunderbirdProfileMailDirs.append(selectedDir);
        }
    }
    QMutableListIterator<QDir> iterator(thunderbirdProfileMailDirs);
    while (iterator.hasNext()) {
        if (!iterator.next().exists()) {
            iterator.remove();
        }
    }
    if (thunderbirdProfileMailDirs.isEmpty()) {
        QMessageBox::warning(this, tr("Invalid Thunderbird profile"),
                             tr("The selected Thunderbird profile does not exist."),
                             QMessageBox::StandardButton::Ok);
        return false;
    }
    return true;
}

void MailAccountDialog::initializeAccountsPage() {
    ui->accountsList->clear();
    for (const QDir &mailDir : thunderbirdProfileMailDirs) {
        for (const QString &mailAccount : mailDir.entryList(
                QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot)) {
            QString accountDirectoryPath = mailDir.absoluteFilePath(mailAccount);
            QDirIterator msfFileIterator(accountDirectoryPath, {"*.msf"},
                    QDir::Files, QDirIterator::Subdirectories);
            if (!msfFileIterator.hasNext()) {
                continue;
            }
            auto* accountItem = new QTreeWidgetItem(ui->accountsList, {mailAccount});
            accountItem->setExpanded(true);
            accountItem->setCheckState(0, Qt::Unchecked);
            ui->accountsList->addTopLevelItem(accountItem);
            while (msfFileIterator.hasNext()) {
                (void) msfFileIterator.next();
                QFileInfo msfFile = msfFileIterator.fileInfo();
                QString name = Utils::getMailFolderName(msfFile);
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

bool MailAccountDialog::validateAccountsPage() {
    return !(getCheckedAccountItems().isEmpty() && QMessageBox::warning(
            this, tr("No folder selected"),
            tr("No mail folder was selected to monitor.\nDo you want to continue?"),
            QMessageBox::StandardButton::Ok | QMessageBox::StandardButton::Cancel
    ) == QMessageBox::StandardButton::Cancel);
}

QList<QTreeWidgetItem*> MailAccountDialog::getCheckedAccountItems() const {
    QList<QTreeWidgetItem*> checkedItems;
    for (int i = 0; i < ui->accountsList->topLevelItemCount(); i++) {
        QTreeWidgetItem* accountItem = ui->accountsList->topLevelItem(i);
        for (int j = 0; j < accountItem->childCount(); j++) {
            QTreeWidgetItem* folderItem = accountItem->child(j);
            if (folderItem->checkState(0) == Qt::Checked) {
                checkedItems.append(folderItem);
            }
        }
    }
    return checkedItems;
}

void MailAccountDialog::propagateChangesToAccountChildren(
        QTreeWidgetItem* parent, Qt::CheckState checkState) {
    for (int i = 0; i < parent->childCount(); i++) {
        QTreeWidgetItem* child = parent->child(i);
        child->setCheckState(0, checkState);
        propagateChangesToAccountChildren(child, checkState);
    }
}
