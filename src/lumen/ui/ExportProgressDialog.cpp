#include "ExportProgressDialog.h"

#include "export/ExportTask.h"

#include <QLabel>
#include <QMessageBox>
#include <QProgressBar>
#include <QPushButton>
#include <QVBoxLayout>

namespace lumen::ui {

ExportProgressDialog::ExportProgressDialog(exp::ExportTask* task,
                                             QWidget* parent)
    : QDialog(parent)
    , task_(task)
{
    setWindowTitle(tr("Exporting..."));
    setMinimumWidth(350);
    setModal(true);

    auto* layout = new QVBoxLayout(this);

    stepLabel_ = new QLabel(tr("Preparing export..."), this);
    layout->addWidget(stepLabel_);

    progressBar_ = new QProgressBar(this);
    progressBar_->setRange(0, 100);
    progressBar_->setValue(0);
    layout->addWidget(progressBar_);

    cancelBtn_ = new QPushButton(tr("Cancel"), this);
    layout->addWidget(cancelBtn_);

    connect(task_, &exp::ExportTask::progress,
            this, &ExportProgressDialog::onProgress);
    connect(task_, &exp::ExportTask::finished,
            this, &ExportProgressDialog::onFinished);
    connect(task_, &exp::ExportTask::error,
            this, &ExportProgressDialog::onError);
    connect(cancelBtn_, &QPushButton::clicked,
            this, &ExportProgressDialog::onCancel);
}

void ExportProgressDialog::onProgress(int percent)
{
    progressBar_->setValue(percent);
    if (percent < 30)
        stepLabel_->setText(tr("Rendering plot..."));
    else if (percent < 80)
        stepLabel_->setText(tr("Writing file..."));
    else
        stepLabel_->setText(tr("Finalizing..."));
}

void ExportProgressDialog::onFinished(bool success, const QString& /*path*/)
{
    if (success) {
        stepLabel_->setText(tr("Export complete."));
        progressBar_->setValue(100);
    }
    accept();
}

void ExportProgressDialog::onError(const QString& message)
{
    QMessageBox::warning(this, tr("Export Error"), message);
}

void ExportProgressDialog::onCancel()
{
    if (task_)
        task_->cancel();
    reject();
}

}  // namespace lumen::ui
