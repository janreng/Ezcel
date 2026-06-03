#pragma once
#include <QVector>

// Logic phụ trợ cho menu chuột phải trên ô đầu hàng/cột (Spec 06).
// Tách thuần (không phụ thuộc Widgets) để test headless.
namespace headermenu {

// Có section nào bị ẩn nằm trong vùng chọn [lo, hi] (đã kẹp về [0, n)) không.
// Excel bật "Hiện lại" khi trong vùng chọn có hàng/cột đang ẩn — kể cả khi
// chọn hai biên ôm lấy một hàng/cột ẩn ở giữa (hộp bao gồm section ẩn đó).
bool canUnhide(const QVector<bool> &hidden, int lo, int hi);

// Danh sách chỉ số section bị ẩn trong [lo, hi] (để hiện lại lần lượt).
QVector<int> hiddenSections(const QVector<bool> &hidden, int lo, int hi);

} // namespace headermenu
