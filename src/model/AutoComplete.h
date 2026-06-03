// AutoComplete cột text (Spec 05) — gợi ý theo tiền tố + danh sách giá trị. Logic thuần, test được.
#pragma once
#include <QString>
#include <QStringList>
#include <vector>

namespace autocomplete {

// Gợi ý hoàn thành cho `prefix` dựa trên các giá trị text trong cột.
// Trả về giá trị đầy đủ nếu có ĐÚNG MỘT giá trị text bắt đầu bằng prefix (không phân biệt
// hoa thường) và khác chính prefix; ngược lại trả chuỗi rỗng.
QString suggest(const std::vector<QString> &colValues, const QString &prefix);

// Danh sách giá trị text duy nhất (bỏ rỗng/số/công thức), đã sắp xếp — cho "Chọn từ danh sách".
QStringList uniqueTexts(const std::vector<QString> &colValues);

} // namespace autocomplete
