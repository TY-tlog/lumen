#pragma once

#include <QDialog>

class QLabel;
class QProgressBar;
class QPushButton;

namespace lumen::exp {
class ExportTask;
}

namespace lumen::ui {

/// Progress dialog for async export (Phase 9.6).
class ExportProgressDialog : public QDialog {
    Q_OBJECT

public:
    explicit ExportProgressDialog(exp::ExportTask* task,
                                   QWidget* parent = nullptr);

private:
    void onProgress(int percent);
    void onFinished(bool success, const QString& path);
    void onError(const QString& message);
    void onCancel();

    exp::ExportTask* task_ = nullptr;
    QProgressBar* progressBar_ = nullptr;
    QLabel* stepLabel_ = nullptr;
    QPushButton* cancelBtn_ = nullptr;
};

}  // namespace lumen::ui
