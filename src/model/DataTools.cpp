// Công cụ dữ liệu (Spec 27).
#include "model/DataTools.h"
#include <QSet>
#include <limits>

namespace datatools {

QStringList splitDelimited(const QString &text, const QString &delim, bool mergeConsecutive)
{
    if (delim.isEmpty()) return {text};
    QStringList parts = text.split(delim, mergeConsecutive ? Qt::SkipEmptyParts : Qt::KeepEmptyParts);
    return parts;
}

QVector<int> duplicateRowIndices(const std::vector<std::vector<QString>> &rows,
                                 const QVector<int> &keyCols, bool hasHeader)
{
    QVector<int> dup;
    QSet<QString> seen;
    int start = hasHeader ? 1 : 0;
    for (int i = start; i < int(rows.size()); ++i) {
        // Khóa = ghép giá trị các cột chọn với dấu ngăn không xuất hiện trong dữ liệu.
        QString key;
        for (int c : keyCols) {
            key += (c >= 0 && c < int(rows[i].size())) ? rows[i][c] : QString();
            key += QChar(0x1f); // dấu ngăn (unit separator)
        }
        if (seen.contains(key)) dup.push_back(i);
        else seen.insert(key);
    }
    return dup;
}

namespace {
// Tổng hợp một cột giá trị (dạng chuỗi) theo hàm `fn`. Chỉ tính ô là số;
// Count đếm ô không rỗng (kể cả text). Trả chuỗi kết quả.
QString aggregate(const QVector<QString> &vals, Agg fn)
{
    if (fn == Agg::Count) {
        int n = 0;
        for (const QString &v : vals) if (!v.trimmed().isEmpty()) ++n;
        return QString::number(n);
    }
    double sum = 0.0;
    double mx = -std::numeric_limits<double>::infinity();
    double mn = std::numeric_limits<double>::infinity();
    int cnt = 0;
    for (const QString &v : vals) {
        bool ok = false;
        double d = v.trimmed().toDouble(&ok);
        if (!ok) continue;
        sum += d; mx = qMax(mx, d); mn = qMin(mn, d); ++cnt;
    }
    double res = 0.0;
    switch (fn) {
    case Agg::Sum:     res = sum; break;
    case Agg::Average: res = cnt ? sum / cnt : 0.0; break;
    case Agg::Max:     res = cnt ? mx : 0.0; break;
    case Agg::Min:     res = cnt ? mn : 0.0; break;
    default:           res = sum; break;
    }
    return QString::number(res, 'g', 15);
}
} // namespace

QVector<QVector<QString>> subtotal(const QVector<QVector<QString>> &rows,
                                   int groupCol, const QVector<int> &aggCols, Agg fn,
                                   const QString &totalLabel, const QString &grandLabel)
{
    QVector<QVector<QString>> out;
    if (rows.isEmpty()) return out;

    // Số cột rộng nhất để dựng dòng tổng đúng kích thước.
    int width = 0;
    for (const auto &r : rows) width = qMax(width, int(r.size()));

    auto valueAt = [](const QVector<QString> &r, int c) -> QString {
        return (c >= 0 && c < r.size()) ? r[c] : QString();
    };

    // Dựng một dòng tổng cho khoảng [from, to) với nhãn đặt ở groupCol.
    auto makeTotalRow = [&](int from, int to, const QString &label) {
        QVector<QString> tr(width);
        if (groupCol >= 0 && groupCol < width) tr[groupCol] = label;
        for (int c : aggCols) {
            if (c < 0 || c >= width) continue;
            QVector<QString> col;
            for (int i = from; i < to; ++i) col.push_back(valueAt(rows[i], c));
            tr[c] = aggregate(col, fn);
        }
        return tr;
    };

    int runStart = 0;
    QString curKey = valueAt(rows[0], groupCol);
    for (int i = 0; i <= rows.size(); ++i) {
        const bool end = (i == rows.size());
        const QString key = end ? QString() : valueAt(rows[i], groupCol);
        if (end || key != curKey) {
            for (int j = runStart; j < i; ++j) out.push_back(rows[j]);
            out.push_back(makeTotalRow(runStart, i, curKey + QStringLiteral(" ") + totalLabel));
            runStart = i;
            curKey = key;
        }
    }
    // Tổng cộng trên toàn bộ dữ liệu gốc.
    out.push_back(makeTotalRow(0, rows.size(), grandLabel));
    return out;
}

} // namespace datatools
