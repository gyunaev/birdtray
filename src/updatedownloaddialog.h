#ifndef UPDATE_DOWNLOAD_DIALOG_H
#define UPDATE_DOWNLOAD_DIALOG_H

#include <QDialog>
#include <QPushButton>

#ifdef Q_OS_WIN
//#  include <QtWinExtras/QWinTaskbarButton>
#endif

namespace Ui {
    class UpdateDownloadDialog;
}

/**
 * A dialog that shows the user the progress while downloading the Birdtray installer
 * and allows the user to update and restart once the download is complete.
 */
class UpdateDownloadDialog : public QDialog {
Q_OBJECT

public:
    explicit UpdateDownloadDialog(QWidget* parent = nullptr);
    
    ~UpdateDownloadDialog() override;
    
    /**
     * Reset the dialog to the download process UI.
     */
    void reset();
    
    /**
     * Switches the dialog to the download completed UI.
     */
    void onDownloadComplete();
    
    /**
     * Update the download process.
     *
     * @param bytesReceived The number of bytes received so far.
     * @param bytesTotal The total number of bytes to receive.
     */
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    
    /**
     * @return Whether or not the user has cancelled the download or aborted the installation.
     */
    bool wasCanceled() const;

private Q_SLOTS:
    
    /**
     * Called if the user presses the action button.
     * During download, the dialog is minimized.
     * Once the download is finished, the button closes the dialog.
     */
    void onActionPressed();

protected:
    void showEvent(QShowEvent* event) override;

private:
    Ui::UpdateDownloadDialog* ui;
    
    /**
     * Whether we are in download-complete or downloading UI mode.
     */
    bool downloadCompleted = false;
    
    /**
     * The first action button.
     */
    QPushButton* actionButton;

#ifdef Q_OS_WIN
    /**
     * The windows task-bar button.
     */
//    QWinTaskbarButton* taskBarButton = nullptr;
#endif
};

#endif // UPDATE_DOWNLOAD_DIALOG_H
