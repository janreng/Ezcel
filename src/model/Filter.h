#pragma once
#include <QString>
#include <QVector>
#include <QSet>

// Lọc dữ liệu — logic thuần (test headless được).
namespace filterutil {

// Trả về index các HÀNG cần ẩn: giá trị cột không chứa `text` (không phân biệt
// hoa/thường). Hàng 0 coi là tiêu đề -> luôn giữ. text rỗng -> không ẩn hàng nào.
QVector<int> rowsToHide(const QVector<QString> &colValues, const QString &text);

// Danh sách giá trị duy nhất (bỏ rỗng) trong cột, đã sắp xếp — cho danh sách chọn lọc.
// Bỏ qua hàng 0 (tiêu đề).
QVector<QString> uniqueValues(const QVector<QString> &colValues);

// Ẩn các HÀNG có giá trị cột KHÔNG nằm trong tập `keep`. Hàng 0 (tiêu đề) luôn giữ.
QVector<int> rowsToHideByValues(const QVector<QString> &colValues, const QSet<QString> &keep);

} // namespace filterutil
