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
         */
        void    onCheckUpdateFinished(const QString &errorString);

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
        void    editThunderbirdCommand();
        void    onCheckUpdateButton();

        // Icon change
        void    buttonChangeIcon();
        void    buttonChangeUnreadIcon();
        
        /**
         * The unread count border width changed.
         * @param value The new border width.
         */
        void onBorderWidthChanged(int value);

        /**
         * Called when the user edited an entry of the Thunderbird command line model.
         *
         * @param topLeft The top left item that was changed.
         * @param bottomRight The bottom right item that was changed.
         * @param roles The data roles that have been modified.
         */
        void onThunderbirdCommandModelChanged(
                const QModelIndex &topLeft, const QModelIndex &bottomRight,
                const QVector<int> &roles = QVector<int>());

    private:
        void    changeIcon(QToolButton * button );

        /**
         * Try to find a command to start Thunderbird on the current system.
         *
         * @return The command that was found.
         */
        QStringList searchThunderbird() const;

        // Model to show the accounts
        ModelAccountTree    *   mAccountModel;

        // Model to show new emails
        ModelNewEmails      *   mModelNewEmails;
    
        /**
         * Model that contains the Thunderbird command line.
         */
        QStringListModel* thunderbirdCmdModel = nullptr;

};

#endif // SETTINGSDIALOG_H
