#ifndef MAIL_ACCOUNT_DIALOG_H
#define MAIL_ACCOUNT_DIALOG_H

#include <tuple>
#include <QWizard>
#include <QtCore/QDir>
#include <QtWidgets/QTreeWidgetItem>

namespace Ui {
    class MailAccountDialog;
}

class MailAccountDialog : public QWizard {
public:
    enum PageId : int {
        noPage = -1,
        profilesDirPage,
        profilePage,
        accountsPage,
    };

Q_OBJECT
protected:
    void initializePage(int id) override;

public:
    explicit MailAccountDialog(QWidget* parent = nullptr, QColor defaultColor = Qt::black);
    
    ~MailAccountDialog() override;
    
    /**
     * @return True if the values selected on the current page are valid.
     */
    bool validateCurrentPage() override;
    
    /**
     * Returns the list of paths to the selected msf files as well as the optional selected color.
     *
     * @param outList: The list of paths to the selected msf files and the selected color.
     */
    void getSelectedAccounts(QList<std::tuple<QString, QColor>> &outList) const;

private Q_SLOTS:
    
    /**
     * Called when the current page id of the wizard changes.
     * @param id The new page id.
     */
    void onCurrentIdChanged(int id);
    
    /**
     * Called when the user clicks on th button to browse for the Thunderbird profiles directory.
     */
    void onProfilesDirBrowseButtonClicked();
    
    /**
     * Called when the user presses enter on the Thunderbird profiles directory path text editor.
     */
    void onProfilesDirEditCommit();
    
    /**
     * Called when the profile selection changes.
     * @param newProfileIndex The index of the new selected profile.
     */
    void onProfileSelectionChanged(int newProfileIndex);
    
    /**
     * Called when an account item in the accounts list changed.
     * @param item The account item.
     * @param column The changed column.
     */
    static void onAccountItemChanged(QTreeWidgetItem* item, int column);

private:
    /**
     * Set up the profiles directory page.
     */
    void initializeProfilesDirPage();
    
    /**
     * @return Whether or not the selected profiles directory is valid.
     */
    bool validateProfilesDirPage();
    
    /**
     * Set up the Thunderbird profiles page.
     */
    void initializeTbProfilesPage();
    
    /**
     * @return Whether or not the selected Thunderbird profile is valid.
     */
    bool validateTbProfilesPage();
    
    /**
     * Set up the accounts selection page.
     */
    void initializeAccountsPage();
    
    /**
     * @return Whether or not the account selection is valid.
     */
    bool validateAccountsPage();
    
    /**
     * Propagates the changes to the parent account item to it's children.
     *
     * @param parent The parent of the account item.
     * @param checkState The new check state of the children.
     */
    static void propagateChangesToAccountChildren(
            QTreeWidgetItem* parent, Qt::CheckState checkState);
    
    /**
     * @return All checked account items from the accountList.
     */
    QList<QTreeWidgetItem*> getCheckedAccountItems() const;
    
    Ui::MailAccountDialog* ui;
    
    /**
     * The default notification color.
     */
    const QColor defaultColor;
    
    /**
     * The id of the furthest page we have reached;
     */
    PageId furthestPage = noPage;
    
    /**
     * The directory containing the Thunderbird profiles.
     */
    QDir* profilesDir = nullptr;
    
    /**
     * The directories of the selected Thunderbird profile that contain the mail folder msf files.
     */
    QList<QDir> thunderbirdProfileMailDirs;
};

#endif // MAIL_ACCOUNT_DIALOG_H
