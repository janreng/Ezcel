// Sparkline — biểu đồ mini trong một ô (đường/cột). Toạ độ tính THUẦN, test được.
#pragma once
#include <QVector>
#include <QPointF>

namespace sparkline {

enum class Type { Line, Column };

// Một sparkline: vẽ trong ô đích (targetRow,targetCol), lấy số liệu từ vùng nguồn.
struct Spark {
    int targetRow, targetCol;
    int srcTop, srcLeft, srcBottom, srcRight;
    Type type;
    bool isTarget(int r, int c) const { return r == targetRow && c == targetCol; }
};

// Toạ độ điểm cho sparkline ĐƯỜNG trong khung w×h (chừa lề `margin` mỗi phía).
// x trải đều; y đảo trục màn hình (giá trị lớn -> y nhỏ, nằm trên). Rỗng nếu values rỗng.
QVector<QPointF> linePoints(const QVector<double> &values, double w, double h, double margin);

// Chiều cao cột (0..h-2*margin) cho sparkline CỘT theo min..max của values.
// Giá trị nhỏ nhất -> 0; lớn nhất -> cao tối đa. Rỗng nếu values rỗng.
QVector<double> columnHeights(const QVector<double> &values, double h, double margin);

} // namespace sparkline
