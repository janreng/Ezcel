#include "model/Sparkline.h"
#include <algorithm>

namespace sparkline {

namespace {
void minMax(const QVector<double> &v, double &mn, double &mx) {
    mn = v[0]; mx = v[0];
    for (double x : v) { mn = std::min(mn, x); mx = std::max(mx, x); }
}
} // namespace

QVector<QPointF> linePoints(const QVector<double> &values, double w, double h, double margin) {
    QVector<QPointF> pts;
    const int n = values.size();
    if (n == 0) return pts;
    double mn, mx; minMax(values, mn, mx);
    const double range = mx - mn;
    const double innerW = w - 2 * margin, innerH = h - 2 * margin;
    for (int i = 0; i < n; ++i) {
        const double x = margin + (n == 1 ? innerW / 2 : innerW * i / (n - 1));
        const double frac = range > 0 ? (values[i] - mn) / range : 0.5;
        const double y = (h - margin) - frac * innerH; // giá trị lớn -> y nhỏ (trên)
        pts.push_back(QPointF(x, y));
    }
    return pts;
}

QVector<double> columnHeights(const QVector<double> &values, double h, double margin) {
    QVector<double> out;
    const int n = values.size();
    if (n == 0) return out;
    double mn, mx; minMax(values, mn, mx);
    const double range = mx - mn;
    const double innerH = h - 2 * margin;
    out.reserve(n);
    for (double v : values) {
        const double frac = range > 0 ? (v - mn) / range : 1.0;
        out.push_back(frac * innerH);
    }
    return out;
}

} // namespace sparkline
