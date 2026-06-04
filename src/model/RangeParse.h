#pragma once
#include "model/SpreadsheetModel.h"  // MergeRange
#include <QString>
#include <QVector>
#include <optional>

// Phân tích chuỗi địa chỉ vùng kiểu Excel (Spec 02) — logic thuần, test được.
// Hỗ trợ: ô đơn "A1", vùng "A1:B3", cả cột "A:A", cả hàng "1:1", và đa vùng
// ngăn bằng dấu phẩy "A1:B3,D5,F1:F10".
namespace rangeparse {

// Phân tích MỘT token thành MergeRange đã kẹp trong [0,rows) × [0,cols).
// Trả nullopt nếu token không hợp lệ. Cả cột/hàng dùng toàn bộ rows/cols.
std::optional<MergeRange> parseOne(const QString &token, int rows, int cols);

// Phân tích chuỗi đa vùng (ngăn bằng dấu phẩy). Bỏ qua token rỗng/không hợp lệ.
QVector<MergeRange> parseMulti(const QString &text, int rows, int cols);

} // namespace rangeparse
