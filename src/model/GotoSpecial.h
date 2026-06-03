#pragma once
#include <QString>
#include <QVector>
#include <QPair>

// "Đi tới đặc biệt" (Go To Special, Spec 32): tìm các ô theo loại. Logic thuần, test được.
namespace gotospecial {

enum class Kind { Blanks, Formulas, Numbers, Text, Constants };

// Trả danh sách (row,col) các ô khớp loại trong lưới thô.
QVector<QPair<int, int>> find(const QVector<QVector<QString>> &grid, Kind kind);

// Vùng dữ liệu liên tục bao quanh ô (row,col) — Ctrl+Shift+* (Current Region).
// Mở rộng hình chữ nhật ra 4 phía chừng nào còn ô không rỗng ở hàng/cột kế cận
// trong phạm vi cạnh hiện tại. Trả [top, left, bottom, right]; ô trống đơn độc -> chính nó.
struct Region { int top, left, bottom, right; };
Region currentRegion(const QVector<QVector<QString>> &grid, int row, int col);

// Ô cuối cùng có dữ liệu: (hàng lớn nhất có ô không rỗng, cột lớn nhất có ô không rỗng).
// Trả {-1, -1} nếu lưới không có dữ liệu nào.
QPair<int, int> lastCell(const QVector<QVector<QString>> &grid);

} // namespace gotospecial
