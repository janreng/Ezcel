// Công cụ dữ liệu (Spec 27).
#include "model/DataTools.h"
#include <QSet>
#include <QHash>
#include <algorithm>
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

QVector<QVector<QString>> reverseRows(const QVector<QVector<QString>> &rows) {
    QVector<QVector<QString>> out;
    out.reserve(rows.size());
    for (int i = rows.size() - 1; i >= 0; --i) out.push_back(rows[i]);
    return out;
}

QVector<QVector<QString>> reverseCols(const QVector<QVector<QString>> &rows) {
    QVector<QVector<QString>> out;
    out.reserve(rows.size());
    for (const auto &row : rows) {
        QVector<QString> r;
        r.reserve(row.size());
        for (int c = row.size() - 1; c >= 0; --c) r.push_back(row[c]);
        out.push_back(r);
    }
    return out;
}

QStringList joinColumns(const QVector<QVector<QString>> &rows, const QString &sep, bool skipEmpty) {
    QStringList out;
    out.reserve(rows.size());
    for (const auto &row : rows) {
        QStringList parts;
        for (const QString &cell : row) {
            if (skipEmpty && cell.trimmed().isEmpty()) continue;
            parts << cell;
        }
        out << parts.join(sep);
    }
    return out;
}

QVector<QPair<QString, double>> pivotSummary(const QVector<QVector<QString>> &rows,
                                             int groupCol, int valueCol, Agg fn) {
    // Gom giá trị cột valueCol theo từng nhóm (giữ thứ tự xuất hiện của nhãn nhóm).
    QHash<QString, QVector<QString>> buckets; // khóa thường-hóa -> các giá trị
    QHash<QString, QString> labelOf;          // khóa thường -> nhãn gốc đầu tiên
    for (const auto &row : rows) {
        if (groupCol < 0 || groupCol >= row.size()) continue;
        const QString g = row[groupCol].trimmed();
        if (g.isEmpty()) continue;
        const QString key = g.toLower();
        if (!labelOf.contains(key)) labelOf.insert(key, g);
        buckets[key].push_back(valueCol >= 0 && valueCol < row.size() ? row[valueCol] : QString());
    }
    QVector<QPair<QString, double>> out;
    for (auto it = buckets.constBegin(); it != buckets.constEnd(); ++it)
        out.push_back({labelOf.value(it.key()), aggregate(it.value(), fn).toDouble()});
    std::sort(out.begin(), out.end(), [](const auto &a, const auto &b) {
        return a.first.compare(b.first, Qt::CaseInsensitive) < 0;
    });
    return out;
}

QVector<QVector<QString>> pivotCrosstab(const QVector<QVector<QString>> &rows,
                                        int rowCol, int colCol, int valueCol, Agg fn,
                                        const QString &cornerLabel, const QString &totalLabel) {
    // Gom giá trị theo cặp (nhãn hàng, nhãn cột); giữ nhãn gốc đầu tiên cho mỗi khóa.
    QHash<QString, QHash<QString, QVector<QString>>> cells; // rowKey -> colKey -> values
    QHash<QString, QString> rowLabel, colLabel;             // key thường -> nhãn gốc
    for (const auto &row : rows) {
        if (rowCol < 0 || rowCol >= row.size() || colCol < 0 || colCol >= row.size()) continue;
        const QString r = row[rowCol].trimmed();
        const QString c = row[colCol].trimmed();
        if (r.isEmpty() || c.isEmpty()) continue;
        const QString rk = r.toLower(), ck = c.toLower();
        if (!rowLabel.contains(rk)) rowLabel.insert(rk, r);
        if (!colLabel.contains(ck)) colLabel.insert(ck, c);
        const QString v = (valueCol >= 0 && valueCol < row.size()) ? row[valueCol] : QString();
        cells[rk][ck].push_back(v);
    }
    // Danh sách nhãn hàng/cột sắp xếp tăng dần (không phân biệt hoa/thường).
    auto sortedLabels = [](const QHash<QString, QString> &m) {
        QVector<QString> keys;
        for (auto it = m.constBegin(); it != m.constEnd(); ++it) keys.push_back(it.key());
        std::sort(keys.begin(), keys.end(), [&](const QString &a, const QString &b) {
            return m.value(a).compare(m.value(b), Qt::CaseInsensitive) < 0;
        });
        return keys;
    };
    const QVector<QString> rk = sortedLabels(rowLabel);
    const QVector<QString> ck = sortedLabels(colLabel);

    QVector<QVector<QString>> out;
    // Hàng tiêu đề.
    QVector<QString> header;
    header.push_back(cornerLabel);
    for (const QString &c : ck) header.push_back(colLabel.value(c));
    header.push_back(totalLabel);
    out.push_back(header);

    // Thân bảng + tổng theo hàng.
    for (const QString &r : rk) {
        QVector<QString> line;
        line.push_back(rowLabel.value(r));
        QVector<QString> rowVals; // gom toàn bộ giá trị của hàng để tính tổng hàng
        for (const QString &c : ck) {
            const auto &vals = cells[r][c];
            line.push_back(vals.isEmpty() ? QString() : aggregate(vals, fn));
            rowVals += vals;
        }
        line.push_back(aggregate(rowVals, fn));
        out.push_back(line);
    }

    // Hàng tổng theo cột + tổng toàn bộ.
    QVector<QString> totalRow;
    totalRow.push_back(totalLabel);
    QVector<QString> allVals;
    for (const QString &c : ck) {
        QVector<QString> colVals;
        for (const QString &r : rk) colVals += cells[r][c];
        totalRow.push_back(aggregate(colVals, fn));
        allVals += colVals;
    }
    totalRow.push_back(aggregate(allVals, fn));
    out.push_back(totalRow);
    return out;
}

QVector<int> duplicateValueIndices(const QVector<QString> &values) {
    QHash<QString, int> counts;
    for (const QString &v : values) {
        const QString k = v.trimmed().toLower();
        if (!k.isEmpty()) ++counts[k];
    }
    QVector<int> out;
    for (int i = 0; i < values.size(); ++i) {
        const QString k = values[i].trimmed().toLower();
        if (!k.isEmpty() && counts.value(k) > 1) out.push_back(i);
    }
    return out;
}

QVector<QVector<QString>> fillBlanksDown(const QVector<QVector<QString>> &rows) {
    QVector<QVector<QString>> out = rows;
    int width = 0;
    for (const auto &r : out) width = qMax(width, int(r.size()));
    QVector<QString> last(width); // giá trị gần nhất theo từng cột
    for (auto &row : out) {
        for (int c = 0; c < width; ++c) {
            if (c >= row.size()) continue;
            if (row[c].trimmed().isEmpty()) { if (!last[c].isEmpty()) row[c] = last[c]; }
            else last[c] = row[c];
        }
    }
    return out;
}

} // namespace datatools
