// Công cụ dữ liệu (Spec 27) — tách cột, xóa hàng trùng. Logic thuần, test được.
#pragma once
#include <QString>
#include <QStringList>
#include <QVector>
#include <vector>

namespace datatools {

// Tách `text` theo chuỗi phân cách `delim`. Nếu mergeConsecutive=true thì bỏ token rỗng
// (coi nhiều dấu phân cách liền nhau như một).
QStringList splitDelimited(const QString &text, const QString &delim, bool mergeConsecutive = false);

// Trả về chỉ số các HÀNG trùng cần xóa (giữ lần xuất hiện đầu), dựa trên giá trị ở `keyCols`.
// Nếu hasHeader=true thì bỏ qua hàng 0. Kết quả tăng dần.
QVector<int> duplicateRowIndices(const std::vector<std::vector<QString>> &rows,
                                 const QVector<int> &keyCols, bool hasHeader);

} // namespace datatools
