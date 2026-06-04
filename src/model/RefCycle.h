#pragma once
#include <QString>

// Đảo trạng thái khóa tuyệt đối ($) của tham chiếu ô — phím F4 (Spec 04/12).
// Logic thuần, test headless được.
namespace refcycle {

// Đảo một tham chiếu ô đơn theo vòng Excel:
//   A1 -> $A$1 -> A$1 -> $A1 -> A1
// Giữ nguyên tên cột + số hàng. Chuỗi không phải tham chiếu hợp lệ -> trả nguyên.
QString cycle(const QString &ref);

// Tìm token tham chiếu (A1, $B$2, AB12...) bao quanh vị trí con trỏ `pos` trong `text`
// rồi đảo trạng thái $ của nó. Trả về text mới; nếu không có ref tại đó -> trả nguyên.
QString cycleAt(const QString &text, int pos);

} // namespace refcycle
