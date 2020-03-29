#include <QFileDialog>
#include <QColorDialog>
#include <QMessageBox>
#include <QtWidgets/QListView>
#include <QtCore/QStandardPaths>

#include "dialogsettings.h"
#include "modelaccounttree.h"
#include "modelnewemails.h"
#include "utils.h"
#include "autoupdater.h"
#include "mailaccountdialog.h"
#include "birdtrayapp.h"
#include "log.h"

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
    
    connect( checkUpdateButton, &QPushButton::clicked,
            this, &DialogSettings::onCheckUpdateButton );
    connect( app->getAutoUpdater(), &AutoUpdater::onCheckUpdateFinished,
            this, &DialogSettings::onCheckUpdateFinished );

    connect( btnShowLogWindow, &QPushButton::clicked, this, &DialogSettings::onShowLogWindow );
    connect( translatorsButton, &QPushButton::clicked, &DialogSettings::onTranslatorsDialog );

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
    ignoreStartupMailCountBox->setChecked(settings->ignoreStartUnreadCount);
    onlyShowIconOnNewMail->setChecked(settings->onlyShowIconOnUnreadMessages);

    if ( settings->mIndexFilesRereadIntervalSec > 0 )
    {
        boxForceReread->setChecked( true );
        spinForceRereadSeconds->setValue( settings->mIndexFilesRereadIntervalSec );
    }
    else
        boxForceReread->setChecked( false );

    // Form the proper command-line (with escaped arguments if they contain spaces
    QString tbcmdline;
    for ( QString a : settings->mThunderbirdCmdLine )
    {
        if ( !tbcmdline.isEmpty() )
            tbcmdline += ' ';

        if ( a.contains( ' ') && a[0] != '"' )
            tbcmdline += '"' + a + '"';
        else
            tbcmdline += a;
    }

    leThunderbirdCmdLine->setText( tbcmdline );

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
    settings->mThunderbirdCmdLine = Utils::splitCommandLine( leThunderbirdCmdLine->text() );
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
    settings->ignoreStartUnreadCount = ignoreStartupMailCountBox->isChecked();
    settings->onlyShowIconOnUnreadMessages = onlyShowIconOnNewMail->isChecked();

    settings->setNotificationIcon(btnNotificationIcon->icon().pixmap( settings->mIconSize ));

    if ( boxNotificationIconUnread->isChecked() )
        settings->mNotificationIconUnread = btnNotificationIconUnread->icon().pixmap( settings->mIconSize );
    else
        settings->mNotificationIconUnread = QPixmap();

    if ( boxForceReread->isChecked() )
        settings->mIndexFilesRereadIntervalSec = spinForceRereadSeconds->value();
    else
        settings->mIndexFilesRereadIntervalSec = 0;

    mModelNewEmails->applySettings();
    mAccountModel->applySettings();

    QDialog::accept();
}

void DialogSettings::onCheckUpdateFinished(bool foundUpdate, const QString &errorString) {
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
    } else {
        noUpdateIndicator->setVisible(!foundUpdate);
    }
}

void DialogSettings::accountAdd()
{
    if ( QGuiApplication::keyboardModifiers() & Qt::ShiftModifier &&
               QGuiApplication::keyboardModifiers() & Qt::ControlModifier )
    {
        QStringList files = QFileDialog::getOpenFileNames(
                nullptr, tr("Choose one or more MSF files"), "", tr("Mail Index (*.msf)"));
        if (files.isEmpty()) {
            return;
        }
        for (const QString &file : files) {
            mAccountModel->addAccount(file, btnNotificationColor->color());
        }
    }
    else
    {
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

void DialogSettings::onCheckUpdateButton() {
    checkUpdateButton->setText(tr("Checking..."));
    checkUpdateButton->setEnabled(false);
    BirdtrayApp::get()->getAutoUpdater()->checkForUpdates();
}

void DialogSettings::onShowLogWindow()
{
    Log::showLogger();
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

void DialogSettings::onTranslatorsDialog() {
    QFile translatorsListFile(":/res/translators.md");
    translatorsListFile.open(QFile::ReadOnly);
    QString translatorsList = QTextStream(&translatorsListFile).readAll();
    translatorsListFile.close();
    translatorsList.replace("# Active maintainers", "# " + tr("Active maintainers"))
                   .replace("# Contributors", "# " + tr("Contributors"));
    QDialog translatorsDialog;
    translatorsDialog.setWindowTitle(tr("Translators"));
    QHBoxLayout layout(&translatorsDialog);
    QTextBrowser content(&translatorsDialog);
    content.setOpenExternalLinks(true);
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    content.setMarkdown(Utils::formatGithubMarkdown(translatorsList));
#else
    content.setText(Utils::addGithubMentionLinksToMarkdown(translatorsList));
#endif
    layout.addWidget(&content);
    translatorsDialog.setLayout(&layout);
    translatorsDialog.exec();
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

    Settings* settings = BirdtrayApp::get()->getSettings();

    // Force scale icon to the expected size
    button->setIcon( test.scaled(
            settings->mIconSize.width(), settings->mIconSize.height(),
            Qt::KeepAspectRatio, Qt::SmoothTransformation) );
}

void DialogSettings::activateTab(int tabIndex)
{
    // #1 is the Accounts tab
    if (tabIndex == 4) {
        noUpdateIndicator->hide();
    }
}
