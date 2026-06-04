#pragma once
#include <QVector>

// Sinh chuỗi số để "Điền chuỗi" (Fill Series, Spec 05). Logic thuần, test được.
namespace seriesgen {

enum class Type { Linear, Growth };

// Sinh `count` giá trị bắt đầu từ `start`:
//   Linear: start, start+step, start+2*step, ...
//   Growth: start, start*step, start*step^2, ...
// count <= 0 -> rỗng.
QVector<double> generate(double start, double step, int count, Type type);

} // namespace seriesgen
