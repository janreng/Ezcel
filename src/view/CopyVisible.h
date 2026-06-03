#pragma once
#include <QString>
#include <QVector>
#include <QSet>

// Sao chép "chỉ ô hiện" (Visible cells only, Spec 32) — logic thuần, test được.
namespace copyutil {

// Ghép TSV từ `block` (mảng hàng × cột chuỗi), BỎ QUA các hàng có chỉ số (offset
// trong block) nằm trong `hiddenRows`. Cột phân tách bằng Tab, hàng bằng '\n'.
// Dùng khi dữ liệu đã lọc: chỉ chép các hàng đang hiển thị.
QString toTsvSkipHidden(const QVector<QVector<QString>> &block, const QSet<int> &hiddenRows);

} // namespace copyutil
