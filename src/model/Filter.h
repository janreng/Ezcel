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

// Điều kiện lọc theo SỐ (Number Filters, Spec 15).
enum class NumOp { Eq, Ne, Gt, Ge, Lt, Le, Between, NotBetween, AboveAvg, BelowAvg };

// Ẩn các HÀNG có giá trị cột KHÔNG thỏa điều kiện số. Hàng 0 (tiêu đề) luôn giữ.
// Ô không phải số luôn bị ẩn (không thỏa điều kiện số). Between/NotBetween dùng [v1, v2]
// (bao gồm hai đầu). AboveAvg/BelowAvg so với trung bình các ô SỐ trong cột (bỏ tiêu đề).
QVector<int> rowsToHideByNumber(const QVector<QString> &colValues, NumOp op,
                                double v1, double v2 = 0.0);

} // namespace filterutil
