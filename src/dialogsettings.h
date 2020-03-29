#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QProgressDialog>
#include <QPalette>
#include <QtCore/QStringListModel>

#include "ui_dialogsettings.h"

#ifdef Q_OS_WIN
#  define THUNDERBIRD_PROFILES_PATH "%AppData%\\Thunderbird\\Profiles"
#else
#  define THUNDERBIRD_PROFILES_PATH "~/Library/Thunderbird/Profiles"
#endif


class ModelAccountTree;
class ModelNewEmails;

class DialogSettings : public QDialog, public Ui::DialogSettings
{
    Q_OBJECT

    public:
        explicit DialogSettings(QWidget *parent = 0);

    public slots:
        void    accept() override;
        
        /**
         * Called once the update check finished.
         *
         * @param hasUpdate true if a new update was found.
         * @param errorMessage A message indicating an error during the check, or a null string.
         */
        void    onCheckUpdateFinished(bool foundUpdate, const QString &errorString);

        /**
         * Called when the currently viewed tab changes
         *
         * @param tabIndex The index of the tab that is now displayed.
         */
        void    activateTab(int tabIndex );

        // Account buttons
        /**
         * Add one or multiple accounts.
         */
        void    accountAdd();
        void    accountEdit();
        void    accountEditIndex( const QModelIndex& index );
        void    accountRemove();

        // New Email buttons
        void    newEmailAdd();
        void    newEmailEdit();
        void    newEmailEditIndex( const QModelIndex& index );
        void    newEmailRemove();
        
        // Advanced buttons
        void    onCheckUpdateButton();
        void    onShowLogWindow();

        // Icon change
        void    buttonChangeIcon();
        void    buttonChangeUnreadIcon();
        
        /**
         * The unread count border width changed.
         * @param value The new border width.
         */
        void onBorderWidthChanged(int value);

    private:
        void    changeIcon(QToolButton * button );

        // Model to show the accounts
        ModelAccountTree    *   mAccountModel;

        // Model to show new emails
        ModelNewEmails      *   mModelNewEmails;
};

#endif // SETTINGSDIALOG_H
