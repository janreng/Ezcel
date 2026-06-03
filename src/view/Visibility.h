#pragma once
class QTableView;

// Ẩn/hiện hàng-cột trên QTableView (thuần view; test headless được).
namespace viewutil {

void hideRows(QTableView *view, int top, int bottom);
void hideCols(QTableView *view, int left, int right);
// Hiện lại mọi hàng/cột bị ẩn nằm trong vùng [top,left]..[bottom,right].
void unhideRange(QTableView *view, int top, int left, int bottom, int right);

} // namespace viewutil
