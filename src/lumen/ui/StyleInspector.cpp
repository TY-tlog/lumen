#include "StyleInspector.h"

#include <QHeaderView>
#include <QLabel>
#include <QTableWidget>
#include <QVBoxLayout>

namespace lumen::ui {

StyleInspector::StyleInspector(QWidget* parent)
    : QDockWidget(tr("Style Inspector"), parent)
{
    setObjectName(QStringLiteral("StyleInspector"));

    auto* widget = new QWidget(this);
    auto* layout = new QVBoxLayout(widget);

    table_ = new QTableWidget(widget);
    table_->setColumnCount(3);
    table_->setHorizontalHeaderLabels({tr("Property"), tr("Value"), tr("Source")});
    table_->horizontalHeader()->setStretchLastSection(true);
    table_->verticalHeader()->setVisible(false);
    table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setAlternatingRowColors(true);

    layout->addWidget(table_);
    setWidget(widget);
    setMinimumWidth(300);
}

void StyleInspector::showTrace(const QString& elementName,
                                const style::CascadeTrace& trace)
{
    setWindowTitle(tr("Style Inspector — %1").arg(elementName));
    table_->setRowCount(static_cast<int>(trace.size()));

    for (int i = 0; i < static_cast<int>(trace.size()); ++i) {
        const auto& entry = trace[i];
        table_->setItem(i, 0, new QTableWidgetItem(entry.property));
        table_->setItem(i, 1, new QTableWidgetItem(entry.value));
        table_->setItem(i, 2, new QTableWidgetItem(entry.sourceName));
    }

    table_->resizeColumnsToContents();
}

void StyleInspector::clearTrace()
{
    setWindowTitle(tr("Style Inspector"));
    table_->setRowCount(0);
}

}  // namespace lumen::ui
