#pragma once
#include <QString>
#include <QStringList>
#include <QVector>
#include <QHash>
#include <algorithm>

// PivotTable / Bảng tổng hợp (Spec 41) — logic thuần để kiểm thử.
// Bản 1: gom nhóm theo 1 trường (hàng) + tính TỔNG một trường số.
namespace pivot {

struct Result {
    QString rowField;              // tên trường hàng (tiêu đề cột nhãn)
    QString valueField;            // tên trường giá trị (tiêu đề cột số)
    QStringList rowLabels;         // các nhãn nhóm (đã sắp xếp)
    QVector<double> values;        // tổng tương ứng từng nhãn
    double grandTotal = 0.0;       // tổng cộng
    bool valid = false;            // dữ liệu hợp lệ không
};

// Gom nhóm vùng [t..b]×[l..r] của grid: HÀNG ĐẦU là tiêu đề.
// rowCol = cột nhãn (gom nhóm), valCol = cột số (tính tổng). Cả hai theo chỉ số cột tuyệt đối.
// Nhãn rỗng -> "(trống)". Giá trị không phải số -> coi như 0.
// Nhãn được sắp xếp tăng dần (số thì theo số, còn lại theo chuỗi).
inline Result sum(const QVector<QVector<QString>> &grid,
                  int t, int l, int b, int r, int rowCol, int valCol)
{
    Result res;
    if (t < 0 || rowCol < l || rowCol > r || valCol < l || valCol > r) return res;
    if (t >= grid.size()) return res;

    auto cell = [&](int row, int col) -> QString {
        if (row < 0 || row >= grid.size()) return QString();
        if (col < 0 || col >= grid[row].size()) return QString();
        return grid[row][col].trimmed();
    };

    res.rowField = cell(t, rowCol);
    res.valueField = cell(t, valCol);

    QHash<QString, double> acc;
    QStringList order; // giữ thứ tự xuất hiện trước khi sắp xếp
    for (int row = t + 1; row <= b && row < grid.size(); ++row) {
        QString key = cell(row, rowCol);
        if (key.isEmpty()) key = QStringLiteral("(trống)");
        bool ok = false;
        double v = cell(row, valCol).toDouble(&ok);
        if (!ok) v = 0.0;
        if (!acc.contains(key)) order << key;
        acc[key] += v;
        res.grandTotal += v;
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
        res.values << acc.value(k);
    }
    res.valid = true;
    return res;
}

} // namespace pivot
