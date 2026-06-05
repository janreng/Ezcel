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

// Nhãn có khớp từ khóa lọc không (từ khóa rỗng -> luôn khớp; không phân biệt hoa thường).
inline bool matchesFilter(const QString &label, const QString &keyword)
{
    return keyword.isEmpty() || label.contains(keyword.trimmed(), Qt::CaseInsensitive);
}

// Gom nhóm vùng [t..b]×[l..r] của grid: HÀNG ĐẦU là tiêu đề.
// rowCol = cột nhãn (gom nhóm), valCol = cột giá trị. Cả hai theo chỉ số cột tuyệt đối.
// Nhãn rỗng -> "(trống)". Ô không phải số bị bỏ qua khi tính Tổng/TB/Max/Min (vẫn đếm cho Đếm).
// Nhãn được sắp xếp tăng dần (số thì theo số, còn lại theo chuỗi).
// rowFilter: chỉ giữ nhãn hàng chứa chuỗi này (rỗng = giữ hết). Tổng cộng tính theo nhãn còn lại.
inline Result aggregate(const QVector<QVector<QString>> &grid,
                        int t, int l, int b, int r, int rowCol, int valCol, Agg agg,
                        const QString &rowFilter = QString())
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
        if (!matchesFilter(key, rowFilter)) continue;
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

// Nạp 1 bản ghi vào bucket (v = giá trị, ok = có phải số).
inline void feedBucket(Bucket &g, double v, bool ok) {
    ++g.rows;
    if (ok) {
        if (!g.hasNum) { g.maxV = g.minV = v; g.hasNum = true; }
        else { g.maxV = qMax(g.maxV, v); g.minV = qMin(g.minV, v); }
        g.sum += v; ++g.numCount;
    }
}

// Sắp xếp danh sách nhãn tăng dần (toàn số -> theo số, còn lại -> theo chuỗi).
inline void sortLabels(QStringList &labels) {
    bool allNum = true;
    for (const QString &k : labels) { bool ok = false; k.toDouble(&ok); if (!ok) { allNum = false; break; } }
    std::sort(labels.begin(), labels.end(), [allNum](const QString &a, const QString &c) {
        if (allNum) return a.toDouble() < c.toDouble();
        return a.compare(c, Qt::CaseInsensitive) < 0;
    });
}

// Bảng chéo 2 chiều.
struct CrossResult {
    QString rowField, colField, valueField;
    Agg agg = Agg::Sum;
    QStringList rowLabels, colLabels;
    QVector<QVector<double>> values; // [hàng][cột]
    QVector<double> rowTotals;       // tổng hợp theo từng hàng (qua mọi cột)
    QVector<double> colTotals;       // tổng hợp theo từng cột (qua mọi hàng)
    double grandTotal = 0.0;
    bool valid = false;
};

// Gom nhóm 2 chiều: rowCol = trường hàng, colCol = trường cột, valCol = trường giá trị.
// Ô [i][j] = giá trị tổng hợp của các bản ghi có (hàng=i, cột=j). Tổng hàng/cột tính trên
// bucket của cả hàng/cả cột (đúng ngữ nghĩa TB/Max/Min, không phải cộng dồn các ô).
inline CrossResult crosstab(const QVector<QVector<QString>> &grid,
                            int t, int l, int b, int r,
                            int rowCol, int colCol, int valCol, Agg agg,
                            const QString &rowFilter = QString())
{
    CrossResult res;
    res.agg = agg;
    if (t < 0 || t >= grid.size()) return res;
    if (rowCol < l || rowCol > r || colCol < l || colCol > r || valCol < l || valCol > r) return res;

    auto cell = [&](int row, int col) -> QString {
        if (row < 0 || row >= grid.size()) return QString();
        if (col < 0 || col >= grid[row].size()) return QString();
        return grid[row][col].trimmed();
    };
    res.rowField = cell(t, rowCol);
    res.colField = cell(t, colCol);
    res.valueField = cell(t, valCol);

    QHash<QString, QHash<QString, Bucket>> cells; // [rowKey][colKey]
    QHash<QString, Bucket> rowB, colB;
    Bucket grand;
    QStringList rowOrder, colOrder;
    for (int row = t + 1; row <= b && row < grid.size(); ++row) {
        QString rk = cell(row, rowCol); if (rk.isEmpty()) rk = QStringLiteral("(trống)");
        if (!matchesFilter(rk, rowFilter)) continue;
        QString ck = cell(row, colCol); if (ck.isEmpty()) ck = QStringLiteral("(trống)");
        if (!rowB.contains(rk)) rowOrder << rk;
        if (!colB.contains(ck)) colOrder << ck;
        bool ok = false; const double v = cell(row, valCol).toDouble(&ok);
        feedBucket(cells[rk][ck], v, ok);
        feedBucket(rowB[rk], v, ok);
        feedBucket(colB[ck], v, ok);
        feedBucket(grand, v, ok);
    }
    if (rowOrder.isEmpty() || colOrder.isEmpty()) return res;
    sortLabels(rowOrder);
    sortLabels(colOrder);
    res.rowLabels = rowOrder;
    res.colLabels = colOrder;

    for (const QString &rk : rowOrder) {
        QVector<double> line;
        for (const QString &ck : colOrder) {
            const auto &colMap = cells.value(rk);
            line << (colMap.contains(ck) ? bucketValue(colMap.value(ck), agg) : 0.0);
        }
        res.values << line;
        res.rowTotals << bucketValue(rowB.value(rk), agg);
    }
    for (const QString &ck : colOrder)
        res.colTotals << bucketValue(colB.value(ck), agg);
    res.grandTotal = bucketValue(grand, agg);
    res.valid = true;
    return res;
}

} // namespace pivot
