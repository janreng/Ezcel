#pragma once
#include <QString>
#include <QStringList>
#include <QVector>
#include <QHash>
#include <algorithm>

// PivotTable / Bảng tổng hợp (Spec 41) — logic thuần để kiểm thử.
// Bản 1: gom nhóm theo 1 trường (hàng) + tính TỔNG một trường số.
namespace pivot {

// Hàm tổng hợp.
enum class Agg { Sum, Count, Average, Max, Min };

inline QString aggName(Agg a) {
    switch (a) {
    case Agg::Sum:     return QStringLiteral("Tổng");
    case Agg::Count:   return QStringLiteral("Đếm");
    case Agg::Average: return QStringLiteral("Trung bình");
    case Agg::Max:     return QStringLiteral("Lớn nhất");
    case Agg::Min:     return QStringLiteral("Nhỏ nhất");
    }
    return QString();
}

struct Result {
    QString rowField;              // tên trường hàng (tiêu đề cột nhãn)
    QString valueField;            // tên trường giá trị (tiêu đề cột số)
    Agg agg = Agg::Sum;            // hàm tổng hợp đã dùng
    QStringList rowLabels;         // các nhãn nhóm (đã sắp xếp)
    QVector<double> values;        // giá trị tổng hợp tương ứng từng nhãn
    double grandTotal = 0.0;       // tổng hợp toàn vùng (cùng hàm)
    bool valid = false;            // dữ liệu hợp lệ không
};

// Số liệu tích lũy cho một nhóm (hoặc toàn vùng).
struct Bucket {
    double sum = 0.0;
    int rows = 0;        // số bản ghi (cho Đếm)
    int numCount = 0;    // số ô là số (cho Trung bình)
    double maxV = 0.0;
    double minV = 0.0;
    bool hasNum = false;
};

// Quy đổi 1 bucket -> giá trị theo hàm tổng hợp.
inline double bucketValue(const Bucket &g, Agg a) {
    switch (a) {
    case Agg::Sum:     return g.sum;
    case Agg::Count:   return double(g.rows);
    case Agg::Average: return g.numCount > 0 ? g.sum / g.numCount : 0.0;
    case Agg::Max:     return g.hasNum ? g.maxV : 0.0;
    case Agg::Min:     return g.hasNum ? g.minV : 0.0;
    }
    return 0.0;
}

// Gom nhóm vùng [t..b]×[l..r] của grid: HÀNG ĐẦU là tiêu đề.
// rowCol = cột nhãn (gom nhóm), valCol = cột giá trị. Cả hai theo chỉ số cột tuyệt đối.
// Nhãn rỗng -> "(trống)". Ô không phải số bị bỏ qua khi tính Tổng/TB/Max/Min (vẫn đếm cho Đếm).
// Nhãn được sắp xếp tăng dần (số thì theo số, còn lại theo chuỗi).
inline Result aggregate(const QVector<QVector<QString>> &grid,
                        int t, int l, int b, int r, int rowCol, int valCol, Agg agg)
{
    Result res;
    res.agg = agg;
    if (t < 0 || rowCol < l || rowCol > r || valCol < l || valCol > r) return res;
    if (t >= grid.size()) return res;

    auto cell = [&](int row, int col) -> QString {
        if (row < 0 || row >= grid.size()) return QString();
        if (col < 0 || col >= grid[row].size()) return QString();
        return grid[row][col].trimmed();
    };

    res.rowField = cell(t, rowCol);
    res.valueField = cell(t, valCol);

    QHash<QString, Bucket> acc;
    Bucket grand;
    QStringList order; // giữ thứ tự xuất hiện trước khi sắp xếp
    for (int row = t + 1; row <= b && row < grid.size(); ++row) {
        QString key = cell(row, rowCol);
        if (key.isEmpty()) key = QStringLiteral("(trống)");
        if (!acc.contains(key)) { order << key; }
        bool ok = false;
        const double v = cell(row, valCol).toDouble(&ok);
        auto feed = [&](Bucket &g) {
            ++g.rows;
            if (ok) {
                if (!g.hasNum) { g.maxV = g.minV = v; g.hasNum = true; }
                else { g.maxV = qMax(g.maxV, v); g.minV = qMin(g.minV, v); }
                g.sum += v; ++g.numCount;
            }
        };
        feed(acc[key]);
        feed(grand);
    }
    if (order.isEmpty()) return res;

    // Sắp xếp nhãn: nếu tất cả là số thì theo số, không thì theo chuỗi (không phân biệt hoa thường).
    bool allNum = true;
    for (const QString &k : order) {
        bool ok = false; k.toDouble(&ok);
        if (!ok) { allNum = false; break; }
    }
    std::sort(order.begin(), order.end(), [allNum](const QString &a, const QString &c) {
        if (allNum) return a.toDouble() < c.toDouble();
        return a.compare(c, Qt::CaseInsensitive) < 0;
    });

    for (const QString &k : order) {
        res.rowLabels << k;
        res.values << bucketValue(acc.value(k), agg);
    }
    res.grandTotal = bucketValue(grand, agg);
    res.valid = true;
    return res;
}

// Tương thích ngược: tính TỔNG.
inline Result sum(const QVector<QVector<QString>> &grid,
                  int t, int l, int b, int r, int rowCol, int valCol)
{
    return aggregate(grid, t, l, b, r, rowCol, valCol, Agg::Sum);
}

} // namespace pivot
