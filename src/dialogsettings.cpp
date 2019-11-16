#include <QFileDialog>
#include <QColorDialog>
#include <QMessageBox>
#include <QFile>
#include <QDir>
#include <QtWidgets/QListView>

#include "dialogsettings.h"
#include "modelaccounttree.h"
#include "modelnewemails.h"
#include "databaseaccounts.h"
#include "dialogaddeditaccount.h"
#include "databaseunreadfixer.h"
#include "utils.h"
#include "autoupdater.h"
#include "mailaccountdialog.h"
#include "birdtrayapp.h"

DialogSettings::DialogSettings( QWidget *parent)
    : QDialog(parent), Ui::DialogSettings()
{
    setupUi(this);
    mProgressFixer = 0;
    BirdtrayApp* app = BirdtrayApp::get();
    Settings* settings = app->getSettings();

    // Show the first tab
    tabWidget->setCurrentIndex( 0 );

    connect( buttonBox, &QDialogButtonBox::accepted, this, &DialogSettings::accept );
    connect( buttonBox, &QDialogButtonBox::rejected, this, &DialogSettings::reject );
    connect( btnBrowse, &QPushButton::clicked, this, &DialogSettings::browsePath );
    connect( leProfilePath, &QLineEdit::textChanged, this, &DialogSettings::profilePathChanged );
    connect( btnFixUnreadCount, &QPushButton::clicked, this, &DialogSettings::fixDatabaseUnreads );
    connect( tabWidget, &QTabWidget::currentChanged, this, &DialogSettings::activateTab );

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
    leProfilePath->setText( QDir::toNativeSeparators(settings->mThunderbirdFolderPath) );
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

    // Prepare the error palette
    mPaletteErrror = mPaletteOk = leProfilePath->palette();
    mPaletteErrror.setColor( QPalette::Text, Qt::red );

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

    // Parsers
    boxParserSelection->addItem( tr("using global search database (wont work with 68+)"), false );
    boxParserSelection->addItem( tr("using Mork index files (recommended)"), true );

    connect( boxParserSelection, SIGNAL(currentIndexChanged(int)), this, SLOT(unreadParserChanged(int)) );

    if ( settings->mUseMorkParser )
        boxParserSelection->setCurrentIndex( 1 );
    else
        boxParserSelection->setCurrentIndex( 0 );

    profilePathChanged();
}

void DialogSettings::accept()
{
    // Validate the profile path if we use database parser
    if ( !isMorkParserSelected() )
    {
        QString profilePath = leProfilePath->text();

        if ( profilePath.isEmpty() )
        {
            QMessageBox::critical(nullptr, tr("Empty Thunderbird directory"),
                                  tr("You must specify a Thunderbird directory."));
            return;
        }
        if (!reportIfProfilePathValid(profilePath)) {
            return;
        }
    }

    if (boxEnableNewEmail->isChecked() && thunderbirdCmdModel->rowCount() <= 1) {
        QMessageBox::critical(nullptr, tr("Empty Thunderbird command"),
                              tr("You have enabled New Email menu, "
                                 "but you did not specify a Thunderbird command."));
        tabWidget->setCurrentIndex( 4 );
        thunderbirdCommandEditButton->setFocus();
        return;
    }
    
    BirdtrayApp* app = BirdtrayApp::get();
    Settings* settings = app->getSettings();
    settings->mThunderbirdFolderPath = leProfilePath->text();
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
    settings->mUseMorkParser = isMorkParserSelected();
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

void DialogSettings::browsePath()
{
    QString path = leProfilePath->text();
    if (path.isNull() || path.isEmpty()) {
        path = Utils::expandPath(THUNDERBIRD_PROFILES_PATH);
    }
    QString directory = QFileDialog::getExistingDirectory(
            nullptr, tr("Choose the Thunderbird profile path"),
            path, QFileDialog::ShowDirsOnly );

    if (directory.isEmpty() || !reportIfProfilePathValid(directory)) {
        return;
    }

    leProfilePath->setText( QDir::toNativeSeparators(directory) );
}


void DialogSettings::profilePathChanged()
{
    bool valid = isProfilePathValid(leProfilePath->text());

    if ( !isMorkParserSelected() )
    {
        groupAccounts->setEnabled( valid );
        btnFixUnreadCount->setEnabled( valid );
    }

    if ( valid ) {
        leProfilePath->setPalette( mPaletteOk );
        updateAccountList();
    } else {
        leProfilePath->setPalette( mPaletteErrror );
        mAccounts.clear();
    }

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

void DialogSettings::fixDatabaseUnreads()
{
    if ( QMessageBox::question( 0,
                           tr("Fix the unread messages?"),
                           tr("<html>This option should be used if you have no unread messages, but still see the new email counter.<br>"
                              "To use this option, it is mandatory to shut down Thunderbird.<br>"
                              "Fixing may take up to five minutes.<br><br>"
                              "Please confirm that Thunderbird is shut down, and you want to proceed?</html>") ) ==  QMessageBox::No )
        return;

    mProgressFixer = new QProgressDialog();
    mProgressFixer->setLabelText( tr("Updating the database...") );
    mProgressFixer->setCancelButton( 0 );
    mProgressFixer->setMinimum( 0 );
    mProgressFixer->setMaximum( 100 );
    mProgressFixer->setWindowModality(Qt::WindowModal);
    mProgressFixer->show();

    DatabaseUnreadFixer * fixer = new DatabaseUnreadFixer(
            DatabaseAccounts::getDatabasePath(leProfilePath->text()));
    connect( fixer, &DatabaseUnreadFixer::done, this, &DialogSettings::databaseUnreadsFixed );
    connect( fixer, &DatabaseUnreadFixer::progress, this, &DialogSettings::databaseUnreadsUpdate );

    fixer->start();
}

void DialogSettings::databaseUnreadsUpdate(int progresspercentage)
{
    mProgressFixer->setValue( progresspercentage );
}

void DialogSettings::databaseUnreadsFixed( QString errorMsg )
{
    Utils::debug("Done updating the database, error: '%s'", qPrintable( errorMsg ) );

    mProgressFixer->close();
    delete mProgressFixer;
    mProgressFixer = 0;

    if (errorMsg.isEmpty()) {
        QMessageBox::information(nullptr, tr("Database updated"),
                                 tr("Successfully updated the database."));
    } else {
        QMessageBox::critical(nullptr, tr("Error updating database"),
                              tr("Error updating the database:\n%1").arg(errorMsg));
    }
}

void DialogSettings::accountsAvailable( QString errorMsg )
{
    if ( !errorMsg.isEmpty() )
    {
        QMessageBox::critical(nullptr, tr("Error retrieving accounts"),
                              tr("Error retrieving accounts:\n%1").arg(errorMsg));
        return;
    }

    DatabaseAccounts * dba = (DatabaseAccounts *) sender();
    mAccounts = dba->accounts();
}

void DialogSettings::accountAdd()
{
    if (!isMorkParserSelected()) {
        DialogAddEditAccount dlg(isMorkParserSelected());
        dlg.setCurrent(mAccounts, "", btnNotificationColor->color());
        if (dlg.exec() != QDialog::Accepted) {
            return;
        }
        mAccountModel->addAccount(dlg.account(), dlg.color());
    } else {
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
    QString uri;
    QColor color;
    mAccountModel->getAccount(index, uri, color);
    color = QColorDialog::getColor(color, this);
    if (color.isValid()) {
        mAccountModel->editAccount(index, uri, color);
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
    layout.addWidget(&dialogButtonBox);
    connect(&dialogButtonBox, &QDialogButtonBox::accepted, &commandDialog, &QDialog::accept);
    connect(&dialogButtonBox, &QDialogButtonBox::rejected, &commandDialog, &QDialog::reject);
    commandDialog.setLayout(&layout);
    if (commandDialog.exec() == QDialog::Accepted) {
        QStringList thunderbirdCommand = thunderbirdCmdModel->stringList();
        thunderbirdCommand.removeLast();
        thunderbirdCommandLabel->setText(thunderbirdCommand.join(' '));
        thunderbirdCommandLabel->setToolTip(thunderbirdCommand.join('\n'));
    }
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

void DialogSettings::unreadParserChanged(int curr)
{
    if ( curr == 1 )
    {
        // Mork parser

        // Fix Unread is useless in this mode
        btnFixUnreadCount->setEnabled( false );

        // Enable account group
        groupAccounts->setEnabled( true );

        // Hide the thunderbird path as well
        groupThunderbirdProfilePath->hide();
    }
    else
    {
        btnFixUnreadCount->setEnabled( true );

        // Show the thunderbird path
        groupThunderbirdProfilePath->show();

        // Trigger hiding/showing the account group
        profilePathChanged();
    }

    // Did we change comparing to settings?
    if (mAccountModel->rowCount() != 0 &&
        isMorkParserSelected() != BirdtrayApp::get()->getSettings()->mUseMorkParser) {
        if ( QMessageBox::question( 0,
                               tr("WARNING: Parser changed"),
                               tr("You have changed the parser, but the account format is not compatible "
                                  "between parsers, and you need to re-set them up.\n\nDo you want to clear the accounts?") )
                == QMessageBox::Yes )
        {
            mAccountModel->clear();
        }
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


void DialogSettings::updateAccountList() {
    if (isMorkParserSelected()) {
        return;
    }
    DatabaseAccounts * dba = new DatabaseAccounts(
            DatabaseAccounts::getDatabasePath(leProfilePath->text()));
    connect( dba, &DatabaseAccounts::done, this, &DialogSettings::accountsAvailable );
    dba->start();
}

void DialogSettings::activateTab(int tabIndex) {
    // #1 is the Accounts tab
    if (tabIndex == 1) {
        updateAccountList();
    }
}

bool DialogSettings::isProfilePathValid(const QString& profilePath) const
{
    if ( profilePath.isEmpty() )
        return false;
    
    QString databasePath = DatabaseAccounts::getDatabasePath(profilePath);
    return QFile::exists(databasePath);
}

bool DialogSettings::reportIfProfilePathValid(const QString &profilePath) const {
    if (isProfilePathValid(profilePath)) {
        return true;
    }
    if (!profilePath.isEmpty()) {
        const QString name = QFileInfo(DatabaseAccounts::getDatabasePath(profilePath)).fileName();
        QMessageBox::critical(nullptr, tr("Invalid Thunderbird directory"),
                tr("Valid Thunderbird directory must contain the file %1.").arg(name));
    }
    return false;
}

bool DialogSettings::isMorkParserSelected() const
{
    return boxParserSelection->currentIndex() == 1;
}

void DialogSettings::onThunderbirdCommandModelChanged(
        const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles) {
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
