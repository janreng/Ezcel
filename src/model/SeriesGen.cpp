#include "model/SeriesGen.h"

namespace seriesgen {

QVector<double> generate(double start, double step, int count, Type type) {
    QVector<double> out;
    if (count <= 0) return out;
    double v = start;
    for (int i = 0; i < count; ++i) {
        out.push_back(v);
        v = (type == Type::Growth) ? v * step : v + step;
    }
    return out;
}

} // namespace seriesgen
