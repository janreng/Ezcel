#include "model/Forecast.h"
#include <cmath>
#include <limits>

namespace forecast {

Line linearFit(const QVector<double> &x, const QVector<double> &y)
{
    Line out;
    const int n = qMin(x.size(), y.size());
    if (n < 2) return out;
    double sx = 0, sy = 0, sxx = 0, sxy = 0;
    for (int i = 0; i < n; ++i) {
        sx += x[i]; sy += y[i];
        sxx += x[i] * x[i]; sxy += x[i] * y[i];
    }
    const double denom = n * sxx - sx * sx;
    if (denom == 0.0) return out; // x trùng nhau -> không có độ dốc
    out.slope = (n * sxy - sx * sy) / denom;
    out.intercept = (sy - out.slope * sx) / n;
    out.ok = true;
    return out;
}

QVector<QPair<double, double>> linearForecast(const QVector<double> &x, const QVector<double> &y, int periods)
{
    QVector<QPair<double, double>> out;
    if (periods <= 0) return out;
    const Line fit = linearFit(x, y);
    if (!fit.ok) return out;
    const int n = qMin(x.size(), y.size());
    // Bước đều trung bình của x lịch sử.
    const double step = (n >= 2) ? (x[n - 1] - x[0]) / (n - 1) : 1.0;
    const double lastX = x[n - 1];
    for (int k = 1; k <= periods; ++k) {
        const double fx = lastX + step * k;
        out.push_back({fx, fit.slope * fx + fit.intercept});
    }
    return out;
}

QVector<double> movingAverage(const QVector<double> &y, int window)
{
    QVector<double> out;
    if (window <= 0) return out;
    out.reserve(y.size());
    const double nan = std::numeric_limits<double>::quiet_NaN();
    double running = 0.0;
    for (int i = 0; i < y.size(); ++i) {
        running += y[i];
        if (i >= window) running -= y[i - window];
        out.push_back(i >= window - 1 ? running / window : nan);
    }
    return out;
}

} // namespace forecast
