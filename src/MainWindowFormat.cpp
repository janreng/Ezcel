// Áp định dạng cho vùng chọn (đậm/nghiêng/màu/căn lề/định dạng số…).
// Các điều khiển (font, cỡ, màu, định dạng số, kiểu ô) nay nằm trên dải lệnh
// Ribbon (buildRibbon ở MainWindow.cpp); file này giữ phần áp định dạng dùng chung.
#include "MainWindow.h"
#include "model/SpreadsheetModel.h"
#include "ui/Theme.h"

#include <QTableView>
#include <QColorDialog>
#include <QColor>

using Format = SpreadsheetModel::Format;

// ---------------------------------------------------------------- áp định dạng
void MainWindow::applyFormatAttr(const QString &key, const QVariant &value)
{
    int t, l, b, r;
    if (!selectionBox(t, l, b, r)) return;
    Format attrs;
    attrs.insert(key, value); // value null -> model xóa key
    m_model->setFormat(t, l, b, r, attrs);
}

void MainWindow::toggleFormatAttr(const QString &key)
{
    QModelIndex cur = m_view->currentIndex();
    bool on = cur.isValid() && m_model->formatAt(cur.row(), cur.column()).value(key).toBool();
    applyFormatAttr(key, on ? QVariant() : QVariant(true)); // null = tắt
}

void MainWindow::pickColor(const QString &key)
{
    int t, l, b, r;
    if (!selectionBox(t, l, b, r)) return;
    QString cur = m_model->formatAt(t, l).value(key).toString();
    QColor init = cur.isEmpty() ? QColor(key == QLatin1String("bg") ? "#ffffff" : "#000000")
                                : QColor(cur);
    QColor c = QColorDialog::getColor(init, this,
        key == QLatin1String("bg") ? QStringLiteral("Màu nền") : QStringLiteral("Màu chữ"));
    if (c.isValid()) applyFormatAttr(key, c.name());
}
