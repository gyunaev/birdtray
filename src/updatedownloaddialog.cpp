#include "updatedownloaddialog.h"
#include "ui_updatedownloaddialog.h"

#ifdef Q_OS_WIN
#  include <QtWinExtras/QWinTaskbarProgress>
#endif

UpdateDownloadDialog::UpdateDownloadDialog(QWidget* parent) :
        QDialog(parent),
#ifdef Q_OS_WIN
        taskBarButton(this),
#endif
        ui(new Ui::UpdateDownloadDialog) {
    ui->setupUi(this);
    actionButton = new QPushButton(tr("Background"), this);
    ui->buttonBox->addButton(actionButton, QDialogButtonBox::ButtonRole::ActionRole);
    connect(actionButton, &QPushButton::clicked, this, &UpdateDownloadDialog::onActionPressed);
    QPushButton* cancelButton = ui->buttonBox->addButton(QDialogButtonBox::StandardButton::Cancel);
    connect(cancelButton, &QPushButton::clicked, this, &UpdateDownloadDialog::onCancelPressed);
}

UpdateDownloadDialog::~UpdateDownloadDialog() {
    delete ui;
    actionButton->deleteLater();
}

void UpdateDownloadDialog::reset() {
    downloadCompleted = false;
    canceled = false;
    ui->label->setText(tr("Downloading Birdtray installer..."));
    ui->progressBar->setValue(0);
    ui->progressBar->show();
#ifdef Q_OS_WIN
    taskBarButton.progress()->reset();
    taskBarButton.progress()->show();
#endif
}

void UpdateDownloadDialog::onDownloadComplete() {
    downloadCompleted = true;
    ui->progressBar->hide();
#ifdef Q_OS_WIN
    taskBarButton.progress()->setRange(0, 100);
    taskBarButton.progress()->setValue(100);
#endif
    actionButton->setText(tr("Update and Restart"));
    ui->label->setText(tr("Download finished. Restart and update Birdtray?"));
    show();
    raise();
    activateWindow();
}

void UpdateDownloadDialog::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal) {
    if (bytesTotal <= 0) {
        ui->label->setText(tr("Downloading Birdtray installer..."));
        ui->progressBar->setRange(0, 0);
#ifdef Q_OS_WIN
        taskBarButton.progress()->setRange(0, 0);
#endif
        return;
    }
    ui->label->setText(
            tr("Downloading Birdtray installer... (%1 Mb/ %2 Mb).")
                    .arg(qRound(bytesReceived / 1000000.0))
                    .arg(qRound(bytesTotal / 1000000.0)));
    int percent = qRound((bytesReceived / (double) bytesTotal) * 100.0);
    ui->progressBar->setRange(0, 100);
    ui->progressBar->setValue(percent);
#ifdef Q_OS_WIN
    taskBarButton.progress()->setRange(0, 100);
    taskBarButton.progress()->setValue(percent);
#endif
}

bool UpdateDownloadDialog::wasCanceled() const {
    return canceled;
}

void UpdateDownloadDialog::onActionPressed() {
    if (downloadCompleted) {
        canceled = false;
        close();
    } else {
        hide();
    }
}

void UpdateDownloadDialog::onCancelPressed() {
    canceled = true;
}

void UpdateDownloadDialog::showEvent(QShowEvent* event) {
    QDialog::showEvent(event);
#ifdef Q_OS_WIN
    taskBarButton.setWindow(windowHandle());
#endif
}
