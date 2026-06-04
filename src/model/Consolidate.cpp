#include "model/Consolidate.h"
#include <QHash>
#include <QStringList>
#include <algorithm>
#include <limits>

namespace consolidate {
namespace {

// Tổng hợp một cột giá trị (chuỗi) theo hàm fn — giống datatools nhưng cục bộ.
QString aggregate(const QVector<QString> &vals, datatools::Agg fn)
{
    using datatools::Agg;
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
        bool okv = false;
        double d = v.trimmed().toDouble(&okv);
        if (!okv) continue;
        sum += d; mx = qMax(mx, d); mn = qMin(mn, d); ++cnt;
    }
    if (cnt == 0) return QString();
    double res = sum;
    switch (fn) {
    case Agg::Sum:     res = sum; break;
    case Agg::Average: res = sum / cnt; break;
    case Agg::Max:     res = mx; break;
    case Agg::Min:     res = mn; break;
    default:           res = sum; break;
    }
    return QString::number(res, 'g', 15);
}

} // namespace

QVector<QVector<QString>> byLabels(const QVector<QVector<QVector<QString>>> &tables,
                                   datatools::Agg fn, const QString &corner)
{
    QStringList rowKeys, colKeys;            // khóa thường-hóa, theo thứ tự xuất hiện
    QHash<QString, QString> rowCase, colCase; // khóa thường -> nhãn gốc đầu tiên
    QHash<QString, QVector<QString>> cells;   // "rowk\x1fcolk" -> các giá trị

    for (const auto &t : tables) {
        if (t.size() < 2 || t[0].size() < 2) continue; // cần ít nhất 1 nhãn hàng + 1 nhãn cột
        const int w = t[0].size();
        for (int c = 1; c < w; ++c) {
            const QString cl = t[0][c].trimmed();
            if (cl.isEmpty()) continue;
            const QString ck = cl.toLower();
            if (!colCase.contains(ck)) { colCase.insert(ck, cl); colKeys << ck; }
        }
        for (int r = 1; r < t.size(); ++r) {
            const QString rl = t[r][0].trimmed();
            if (rl.isEmpty()) continue;
            const QString rk = rl.toLower();
            if (!rowCase.contains(rk)) { rowCase.insert(rk, rl); rowKeys << rk; }
            for (int c = 1; c < t[r].size() && c < w; ++c) {
                const QString cl = t[0][c].trimmed();
                if (cl.isEmpty()) continue;
                cells[rk + QChar(0x1f) + cl.toLower()].push_back(t[r][c]);
            }
        }
    }

    std::sort(rowKeys.begin(), rowKeys.end());
    std::sort(colKeys.begin(), colKeys.end());

    QVector<QVector<QString>> out;
    QVector<QString> header;
    header << corner;
    for (const QString &ck : colKeys) header << colCase.value(ck);
    out << header;
    for (const QString &rk : rowKeys) {
        QVector<QString> line;
        line << rowCase.value(rk);
        for (const QString &ck : colKeys) {
            auto it = cells.constFind(rk + QChar(0x1f) + ck);
            line << (it == cells.constEnd() ? QString() : aggregate(*it, fn));
        }
        out << line;
    }
    return out;
}

} // namespace consolidate
