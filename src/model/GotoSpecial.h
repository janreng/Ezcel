#pragma once
#include <QString>
#include <QVector>
#include <QPair>

// "Đi tới đặc biệt" (Go To Special, Spec 32): tìm các ô theo loại. Logic thuần, test được.
namespace gotospecial {

enum class Kind { Blanks, Formulas, Numbers, Text, Constants };

// Trả danh sách (row,col) các ô khớp loại trong lưới thô.
QVector<QPair<int, int>> find(const QVector<QVector<QString>> &grid, Kind kind);

} // namespace gotospecial
