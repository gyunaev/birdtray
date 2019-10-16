#include "updatedialog.h"
#include "ui_updatedialog.h"
#include <utils.h>
#include <QPushButton>

UpdateDialog::UpdateDialog(QWidget* parent) :
        QDialog(parent),
        ui(new Ui::UpdateDialog) {
    ui->setupUi(this);
    ui->currentVersionLabel->setText(Utils::getBirdtrayVersion());
    
    downloadButton = new QPushButton(tr("Download"), ui->buttonBox);
    downloadButton->setDefault(true);
    downloadButton->setAutoDefault(true);
    connect(downloadButton, &QPushButton::clicked, this, &UpdateDialog::onDownloadButtonClicked);
    ui->buttonBox->addButton(downloadButton, QDialogButtonBox::ButtonRole::AcceptRole);
    ui->buttonBox->addButton(QDialogButtonBox::StandardButton::Cancel);
}

UpdateDialog::~UpdateDialog() {
    delete ui;
    downloadButton->deleteLater();
}

void UpdateDialog::show(const QString &newVersion, const QString &changelog,
                        qulonglong estimatedSize) {
    ui->newVersionLabel->setText(newVersion);
    ui->changelogLabel->setText(changelog);
    if (estimatedSize == (qulonglong) -1) {
        downloadButton->setText(tr("Download"));
    } else {
        downloadButton->setText(tr("Update and Restart"));
    }
    if (estimatedSize == 0 || estimatedSize == (qulonglong) -1) {
        ui->estimatedSizeDescLabel->hide();
        ui->estimatedSizeLabel->hide();
    } else {
        ui->estimatedSizeLabel->setText(tr("ca. %1 Mb").arg(qRound(estimatedSize / 1000000.0)));
        ui->estimatedSizeDescLabel->show();
        ui->estimatedSizeLabel->show();
    }
    QDialog::show();
}
