#pragma once
#include <QHash>
#include <QString>
#include <QVariant>
#include <QStringList>

// Kiểu ô dựng sẵn (Cell Styles, Spec 30). Trả bộ thuộc tính định dạng cho 1 kiểu.
// Thuần dữ liệu, test được. Áp bằng SpreadsheetModel::setFormat.
namespace cellstyles {

// Danh sách tên kiểu (theo thứ tự hiển thị).
QStringList names();

// Bộ thuộc tính định dạng của kiểu (key giống Format của model). "Normal" -> trả
// các key = null để XÓA định dạng. Tên lạ -> rỗng.
QHash<QString, QVariant> style(const QString &name);

} // namespace cellstyles
