#pragma once
#include <QString>
#include <QStringList>
#include <QVector>

// Biểu đồ tự vẽ (Spec 19): tách dữ liệu vùng chọn thành chuỗi (nhãn + giá trị)
// và tính trục. Logic thuần để kiểm thử; phần vẽ ở view/ChartWidget.
namespace chart {

struct Series {
    QStringList labels;
    QVector<double> values;
};

// Rút chuỗi từ vùng [t..b]×[l..r] của lưới:
//  - Nếu >=2 cột: cột TRÁI làm nhãn, cột PHẢI làm giá trị số.
//  - Nếu 1 cột: giá trị là cột đó, nhãn là số thứ tự 1,2,3…
// Ô không phải số -> 0.
inline Series extractSeries(const QVector<QVector<QString>> &grid, int t, int l, int b, int r)
{
    Series s;
    auto at = [&](int row, int c) -> QString {
        return (row >= 0 && row < grid.size() && c >= 0 && c < grid[row].size())
                   ? grid[row][c].trimmed() : QString();
    };
    const bool twoCol = (r > l);
    int idx = 1;
    for (int row = t; row <= b; ++row) {
        bool ok = false;
        const double v = at(row, r).toDouble(&ok);
        s.values << (ok ? v : 0.0);
        if (twoCol) {
            const QString lab = at(row, l);
            s.labels << (lab.isEmpty() ? QString::number(idx) : lab);
        } else {
            s.labels << QString::number(idx);
        }
        ++idx;
    }
    return s;
}

// Góc quạt (độ) cho biểu đồ tròn: value/tổng × 360. Giá trị âm coi như 0.
// Tổng <= 0 -> vector rỗng. Thuần để kiểm thử.
inline QVector<double> pieAngles(const QVector<double> &values)
{
    double sum = 0;
    for (double v : values) sum += (v > 0 ? v : 0);
    QVector<double> out;
    if (sum <= 0) return out;
    for (double v : values) out << (v > 0 ? v : 0) / sum * 360.0;
    return out;
}

// Cận trên "đẹp" cho trục: làm tròn lên bội của 1/2/5 × lũy thừa 10.
inline double niceMax(double maxVal)
{
    if (maxVal <= 0) return 1.0;
    double pow10 = 1.0;
    while (pow10 * 10 <= maxVal) pow10 *= 10;
    while (pow10 > maxVal && pow10 > 1e-9) pow10 /= 10; // pow10 <= maxVal < pow10*10
    for (double f : {1.0, 2.0, 5.0, 10.0})
        if (maxVal <= f * pow10) return f * pow10;
    return 10 * pow10;
}

} // namespace chart
