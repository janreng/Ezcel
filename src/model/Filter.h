#pragma once
#include <QString>
#include <QVector>

// Lọc dữ liệu — logic thuần (test headless được).
namespace filterutil {

// Trả về index các HÀNG cần ẩn: giá trị cột không chứa `text` (không phân biệt
// hoa/thường). Hàng 0 coi là tiêu đề -> luôn giữ. text rỗng -> không ẩn hàng nào.
QVector<int> rowsToHide(const QVector<QString> &colValues, const QString &text);

} // namespace filterutil
