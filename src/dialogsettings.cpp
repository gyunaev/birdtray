#include <QFileDialog>
#include <QColorDialog>
#include <QMessageBox>
#include <QFile>
#include <QtWidgets/QListView>
#include <QtCore/QStandardPaths>

#include "dialogsettings.h"
#include "modelaccounttree.h"
#include "modelnewemails.h"
#include "utils.h"
#include "autoupdater.h"
#include "mailaccountdialog.h"
#include "birdtrayapp.h"

DialogSettings::DialogSettings( QWidget *parent)
    : QDialog(parent), Ui::DialogSettings()
{
    setupUi(this);
    BirdtrayApp* app = BirdtrayApp::get();
    Settings* settings = app->getSettings();

    // Show the first tab
    tabWidget->setCurrentIndex( 0 );

    connect( buttonBox, &QDialogButtonBox::accepted, this, &DialogSettings::accept );
    connect( buttonBox, &QDialogButtonBox::rejected, this, &DialogSettings::reject );

    connect( btnNotificationIcon, &QPushButton::clicked, this, &DialogSettings::buttonChangeIcon );
    connect( btnNotificationIconUnread, &QPushButton::clicked, this, &DialogSettings::buttonChangeUnreadIcon );
    connect(borderWidthSlider, &QSlider::valueChanged, this, &DialogSettings::onBorderWidthChanged);

    connect( treeAccounts, &QTreeView::doubleClicked, this, &DialogSettings::accountEditIndex  );
    connect( btnAccountAdd, &QPushButton::clicked, this, &DialogSettings::accountAdd );
    connect( btnAccountEdit, &QPushButton::clicked, this, &DialogSettings::accountEdit );
    connect( btnAccountRemove, &QPushButton::clicked, this, &DialogSettings::accountRemove );

    connect( treeNewEmails, &QTreeView::doubleClicked, this, &DialogSettings::newEmailEditIndex  );
    connect( btnNewEmailAdd, &QPushButton::clicked, this, &DialogSettings::newEmailAdd );
    connect( btnNewEmailEdit, &QPushButton::clicked, this, &DialogSettings::newEmailEdit );
    connect( btnNewEmailDelete, &QPushButton::clicked, this, &DialogSettings::newEmailRemove );
    
    connect( thunderbirdCommandEditButton, &QToolButton::clicked,
            this, &DialogSettings::editThunderbirdCommand );
    connect( checkUpdateButton, &QPushButton::clicked,
            this, &DialogSettings::onCheckUpdateButton );
    connect( app->getAutoUpdater(), &AutoUpdater::onCheckUpdateFinished,
            this, &DialogSettings::onCheckUpdateFinished );

    // Setup parameters
    btnNotificationColor->setColor( settings->mNotificationDefaultColor );
    borderColorButton->setColor(settings->mNotificationBorderColor);
    borderWidthSlider->setValue(static_cast<int>(settings->mNotificationBorderWidth) * 2);
    notificationFont->setCurrentFont( settings->mNotificationFont );
    notificationFontWeight->setValue( settings->mNotificationFontWeight * 2 );
    sliderBlinkingSpeed->setValue( settings->mBlinkSpeed );
    boxLaunchThunderbirdAtStart->setChecked( settings->mLaunchThunderbird );
    boxShowHideThunderbird->setChecked( settings->mShowHideThunderbird );
    boxHideWhenMinimized->setChecked( settings->mHideWhenMinimized );
    boxMonitorThunderbirdWindow->setChecked( settings->mMonitorThunderbirdWindow );
    boxRestartThunderbird->setChecked( settings->mRestartThunderbird );
    thunderbirdCommandLabel->setText(settings->mThunderbirdCmdLine.join(' '));
    thunderbirdCommandLabel->setToolTip(settings->mThunderbirdCmdLine.join('\n'));
    leThunderbirdWindowMatch->setText( settings->mThunderbirdWindowMatch  );
    spinMinimumFontSize->setValue( settings->mNotificationMinimumFontSize );
    spinMinimumFontSize->setMaximum( settings->mNotificationMaximumFontSize - 1 );
    boxHideWindowAtStart->setChecked( settings->mHideWhenStarted );
    boxHideWindowAtRestart->setChecked( settings->mHideWhenRestarted );
    boxEnableNewEmail->setChecked( settings->mNewEmailMenuEnabled );
    boxBlinkingUsesAlpha->setChecked( settings->mBlinkingUseAlphaTransition );
    checkUpdateOnStartup->setChecked( settings->mUpdateOnStartup );
    boxAllowSuppression->setChecked( settings->mAllowSuppressingUnreads );
    spinUnreadOpacityLevel->setValue( settings->mUnreadOpacityLevel * 100 );
    spinThunderbirdStartDelay->setValue( settings->mLaunchThunderbirdDelay );
    boxShowUnreadCount->setChecked( settings->mShowUnreadEmailCount );

    if ( settings->mLaunchThunderbird )
        boxStopThunderbirdOnExit->setChecked( settings->mExitThunderbirdWhenQuit );

    // Accounts tab
    mAccountModel = new ModelAccountTree(this, treeAccounts);
    treeAccounts->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    treeAccounts->setCurrentIndex(mAccountModel->index(0, 0));

    // New emails tab
    mModelNewEmails = new ModelNewEmails( this );
    treeNewEmails->setModel( mModelNewEmails );
    treeNewEmails->setCurrentIndex(mModelNewEmails->index(0, 0));
    
    // Advanced tab
    QStringList thunderbirdCommandLine = settings->mThunderbirdCmdLine;
    thunderbirdCmdModel = new QStringListModel(thunderbirdCommandLine << "", this);
    connect(thunderbirdCmdModel, &QAbstractItemModel::dataChanged,
            this, &DialogSettings::onThunderbirdCommandModelChanged);

    // Create the "About" box
    QString origabout = browserAbout->toHtml();
    origabout.replace( "[VERSION]", Utils::getBirdtrayVersion() );
    origabout.replace( "[DATE]", QString("%1 %2").arg(__DATE__) .arg(__TIME__) );
    browserAbout->setText( origabout );

    // Icon
    btnNotificationIcon->setIcon( settings->getNotificationIcon() );

    if ( !settings->mNotificationIconUnread.isNull() )
    {
        boxNotificationIconUnread->setChecked( true );
        btnNotificationIconUnread->setIcon( settings->mNotificationIconUnread );
    }
    else
        boxNotificationIconUnread->setChecked( false );
}

void DialogSettings::accept()
{
    BirdtrayApp* app = BirdtrayApp::get();
    Settings* settings = app->getSettings();
    settings->mNotificationDefaultColor = btnNotificationColor->color();
    settings->mNotificationBorderColor = borderColorButton->color();
    // A width of 100 is way to much, nobody will want to go beyond 50.
    settings->mNotificationBorderWidth = qRound(borderWidthSlider->value() / 2.0);
    settings->mNotificationFont = notificationFont->currentFont();
    settings->mBlinkSpeed = sliderBlinkingSpeed->value();
    settings->mLaunchThunderbird = boxLaunchThunderbirdAtStart->isChecked();
    settings->mShowHideThunderbird = boxShowHideThunderbird->isChecked();
    QStringList thunderbirdCommand = thunderbirdCmdModel->stringList();
    thunderbirdCommand.removeLast();
    settings->mThunderbirdCmdLine = thunderbirdCommand;
    settings->mThunderbirdWindowMatch = leThunderbirdWindowMatch->text();
    settings->mHideWhenMinimized = boxHideWhenMinimized->isChecked();
    settings->mNotificationFontWeight = qMin(99, (int) (notificationFontWeight->value() / 2));
    settings->mExitThunderbirdWhenQuit = boxStopThunderbirdOnExit->isChecked();
    settings->mAllowSuppressingUnreads = boxAllowSuppression->isChecked();

    settings->mMonitorThunderbirdWindow = boxMonitorThunderbirdWindow->isChecked();    
    settings->mNotificationMinimumFontSize = spinMinimumFontSize->value();
    settings->mRestartThunderbird = boxRestartThunderbird->isChecked();
    settings->mHideWhenStarted = boxHideWindowAtStart->isChecked();
    settings->mHideWhenRestarted = boxHideWindowAtRestart->isChecked();
    settings->mNewEmailMenuEnabled = boxEnableNewEmail->isChecked();
    settings->mBlinkingUseAlphaTransition = boxBlinkingUsesAlpha->isChecked();
    settings->mUpdateOnStartup = checkUpdateOnStartup->isChecked();
    settings->mUnreadOpacityLevel = (double) spinUnreadOpacityLevel->value() / 100.0;
    settings->mLaunchThunderbirdDelay = spinThunderbirdStartDelay->value();
    settings->mShowUnreadEmailCount = boxShowUnreadCount->isChecked();

    settings->setNotificationIcon(btnNotificationIcon->icon().pixmap( settings->mIconSize ));

    if ( boxNotificationIconUnread->isChecked() )
        settings->mNotificationIconUnread = btnNotificationIconUnread->icon().pixmap( settings->mIconSize );
    else
        settings->mNotificationIconUnread = QPixmap();

    mModelNewEmails->applySettings();
    mAccountModel->applySettings();

    QDialog::accept();
}

void DialogSettings::onCheckUpdateFinished(const QString &errorString) {
    if (checkUpdateButton->isEnabled()) {
        return; // We received an error that was not initiated by us. Ignore it.
    }
    checkUpdateButton->setText(tr("Check now"));
    checkUpdateButton->setEnabled(true);
    if (!errorString.isNull()) {
        QMessageBox::warning(
                nullptr, tr("Version check failed"),
                tr("Failed to check for a new Birdtray version:\n") + errorString,
                QMessageBox::StandardButton::Ok);
    }
}

void DialogSettings::accountAdd() {
    MailAccountDialog accountDialog(this, btnNotificationColor->color());
    if (accountDialog.exec() != QDialog::Accepted) {
        return;
    }
    QString path;
    QColor color;
    QList<std::tuple<QString, QColor>> accountInfoList;
    accountDialog.getSelectedAccounts(accountInfoList);
    for (const std::tuple<QString, QColor> &accountInfo : accountInfoList) {
        std::tie(path, color) = accountInfo;
        mAccountModel->addAccount(path, color);
    }
    treeAccounts->setCurrentIndex(mAccountModel->index(mAccountModel->rowCount() - 1, 0));
}

void DialogSettings::accountEdit()
{
    if ( treeAccounts->currentIndex().isValid() )
        accountEditIndex( treeAccounts->currentIndex() );
}

void DialogSettings::accountEditIndex(const QModelIndex &index)
{
    if (!index.isValid()) {
        return;
    }
    QString path;
    QColor color;
    mAccountModel->getAccount(index, path, color);
    color = QColorDialog::getColor(color, this);
    if (color.isValid()) {
        mAccountModel->editAccount(index, path, color);
    }
}

void DialogSettings::accountRemove()
{
    QModelIndex index = treeAccounts->currentIndex();

    if ( !index.isValid() )
        return;

    mAccountModel->removeAccount( index );
}

void DialogSettings::newEmailAdd() {
    if (mModelNewEmails->add()) {
        treeNewEmails->setCurrentIndex(mModelNewEmails->index(mModelNewEmails->rowCount() -1, 0));
    }
}

void DialogSettings::newEmailEdit()
{
    newEmailEditIndex( treeNewEmails->currentIndex() );
}

void DialogSettings::newEmailEditIndex(const QModelIndex &index)
{
    mModelNewEmails->edit( index );
}

void DialogSettings::newEmailRemove()
{
    mModelNewEmails->remove( treeNewEmails->currentIndex() );
}

void DialogSettings::editThunderbirdCommand() {
    QDialog commandDialog(this);
    commandDialog.setWindowTitle(tr("Thunderbird Command"));
    commandDialog.resize(500, 300);
    QVBoxLayout layout(&commandDialog);
    QListView commandListView(&commandDialog);
    commandListView.setAlternatingRowColors(true);
    commandListView.setModel(thunderbirdCmdModel);
    layout.addWidget(&commandListView);
    QDialogButtonBox dialogButtonBox(
            QDialogButtonBox::Save | QDialogButtonBox::Cancel, &commandDialog);
    QPushButton* detectButton = dialogButtonBox.addButton(
            tr("Auto detect"), QDialogButtonBox::ActionRole);
    layout.addWidget(&dialogButtonBox);
    connect(&dialogButtonBox, &QDialogButtonBox::accepted, &commandDialog, &QDialog::accept);
    connect(&dialogButtonBox, &QDialogButtonBox::rejected, &commandDialog, &QDialog::reject);
    connect(detectButton, &QPushButton::clicked, this, [&]() {
        commandListView.setEnabled(false);
        QStringList command = searchThunderbird();
        if (command.isEmpty() || !QFileInfo(Utils::expandPath(command[0])).isExecutable()) {
            QMessageBox::warning(&commandDialog, tr("Thunderbird not found"),
                    tr("Unable to detect Thunderbird on your system."));
        } else {
            thunderbirdCmdModel->setStringList(command << "");
        }
        commandListView.setEnabled(true);
    });
    commandDialog.setLayout(&layout);
    QStringList thunderbirdCommand = thunderbirdCmdModel->stringList();
    if (commandDialog.exec() != QDialog::Accepted) {
        thunderbirdCmdModel->setStringList(thunderbirdCommand);
        return;
    }
    thunderbirdCommand = thunderbirdCmdModel->stringList();
    if (thunderbirdCommand.count() <= 1) {
        thunderbirdCommand = Utils::getDefaultThunderbirdCommand() << "";
        thunderbirdCmdModel->setStringList(thunderbirdCommand);
    }
    thunderbirdCommand.removeLast();
    thunderbirdCommandLabel->setText(thunderbirdCommand.join(' '));
    thunderbirdCommandLabel->setToolTip(thunderbirdCommand.join('\n'));
}

void DialogSettings::onCheckUpdateButton() {
    checkUpdateButton->setText(tr("Checking..."));
    checkUpdateButton->setEnabled(false);
    BirdtrayApp::get()->getAutoUpdater()->checkForUpdates();
}

void DialogSettings::buttonChangeIcon()
{
    if ( (QApplication::keyboardModifiers() & Qt::CTRL) != 0 )
    {
        // Reset default icon
        QPixmap temp;

        if ( temp.load( ":res/thunderbird.png" ) )
            btnNotificationIcon->setIcon( temp );

        return;
    }

    changeIcon( btnNotificationIcon );
}

void DialogSettings::buttonChangeUnreadIcon()
{
    changeIcon( btnNotificationIconUnread );
}

void DialogSettings::onBorderWidthChanged(int value) {
    borderWidthLabel->setText(value == 0 ? tr("None") : QString::number(value) + '%');
}

void DialogSettings::onThunderbirdCommandModelChanged(
        const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles) {
    Q_UNUSED(bottomRight)
    Q_UNUSED(roles)
    if (topLeft.row() != thunderbirdCmdModel->rowCount() - 1
        && thunderbirdCmdModel->data(topLeft, Qt::DisplayRole).toString().isEmpty()) {
        thunderbirdCmdModel->removeRow(topLeft.row());
    }
    if (!thunderbirdCmdModel->data(thunderbirdCmdModel->index(
            thunderbirdCmdModel->rowCount() - 1, 0), Qt::DisplayRole).toString().isEmpty()) {
        thunderbirdCmdModel->insertRow(thunderbirdCmdModel->rowCount());
        thunderbirdCmdModel->setData(
                thunderbirdCmdModel->index(thunderbirdCmdModel->rowCount() - 1, 0), "");
    }
}

void DialogSettings::changeIcon(QToolButton *button)
{
    QString e = QFileDialog::getOpenFileName( 0,
                                              tr("Choose the new icon"),
                                              "",
                                              tr("Images (*.png *.svg)") );

    if ( e.isEmpty() )
        return;

    QPixmap test;

    if ( !test.load( e ) )
    {
        QMessageBox::critical(nullptr, tr("Invalid icon"),
                              tr("Could not load the icon from this file."));
        return;
    }

    button->setIcon( test );
}

QStringList DialogSettings::searchThunderbird() const {
    QStringList defaultCommand = Utils::getDefaultThunderbirdCommand();
    if (defaultCommand.count() == 1
        && !QFileInfo(Utils::expandPath(defaultCommand[0])).isExecutable()) {
        return defaultCommand;
    }
    QString thunderbirdPath = QStandardPaths::findExecutable("thunderbird");
    if (!thunderbirdPath.isEmpty()) {
        return {thunderbirdPath};
    }
    return defaultCommand;
}
