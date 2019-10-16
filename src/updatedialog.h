#ifndef UPDATE_DIALOG_H
#define UPDATE_DIALOG_H

#include <QDialog>

namespace Ui {
    class UpdateDialog;
}

/**
 * A dialog that informs the user about a new update, shows the changelog
 * and allows to download and potentially install the update.
 */
class UpdateDialog : public QDialog {
Q_OBJECT

public:
    explicit UpdateDialog(QWidget* parent = nullptr);
    
    ~UpdateDialog() override;
    
    /**
     * Show the dialog.
     *
     * @param newVersion The new version that is available.
     * @param changelog The changelog for the new version.
     * @param estimatedSize The estimated download size or 0 if the size is unknown,
     *                      or -1 if the new version can't be downloaded automatically.
     */
    void show(const QString &newVersion, const QString &changelog, qulonglong estimatedSize);

Q_SIGNALS:
    
    /**
     * Signalled when the user clicks on the download button.
     */
    void onDownloadButtonClicked();

private:
    Ui::UpdateDialog* ui;
    
    /**
     * The download or download & update button.
     */
    QPushButton* downloadButton;
};

#endif // UPDATE_DIALOG_H
