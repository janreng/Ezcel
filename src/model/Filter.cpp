#include "model/Filter.h"
#include <algorithm>

namespace filterutil {

QVector<int> rowsToHide(const QVector<QString> &colValues, const QString &text) {
    QVector<int> hide;
    if (text.isEmpty()) return hide;
    for (int i = 1; i < colValues.size(); ++i) // bỏ qua hàng 0 (tiêu đề)
        if (!colValues[i].contains(text, Qt::CaseInsensitive))
            hide.push_back(i);
    return hide;
}

QVector<QString> uniqueValues(const QVector<QString> &colValues) {
    QVector<QString> out;
    QSet<QString> seen;
    for (int i = 1; i < colValues.size(); ++i) { // bỏ qua tiêu đề
        const QString v = colValues[i].trimmed();
        if (v.isEmpty() || seen.contains(v)) continue;
        seen.insert(v);
        out.push_back(v);
    }
    std::sort(out.begin(), out.end(), [](const QString &a, const QString &b) {
        return a.compare(b, Qt::CaseInsensitive) < 0;
    });
    return out;
}

QVector<int> rowsToHideByValues(const QVector<QString> &colValues, const QSet<QString> &keep) {
    QVector<int> hide;
    for (int i = 1; i < colValues.size(); ++i) { // bỏ qua tiêu đề
        const QString v = colValues[i].trimmed();
        if (v.isEmpty()) continue; // ô trống: giữ
        if (!keep.contains(v)) hide.push_back(i);
    }
    return hide;
}

QVector<int> rowsToHideByNumber(const QVector<QString> &colValues, NumOp op,
                                double v1, double v2) {
    // Trung bình các ô số (bỏ tiêu đề) — cho AboveAvg/BelowAvg.
    double avg = 0.0;
    if (op == NumOp::AboveAvg || op == NumOp::BelowAvg) {
        double sum = 0.0; int n = 0;
        for (int i = 1; i < colValues.size(); ++i) {
            bool ok = false;
            double d = colValues[i].trimmed().toDouble(&ok);
            if (ok) { sum += d; ++n; }
        }
        avg = n ? sum / n : 0.0;
    }

    QVector<int> hide;
    for (int i = 1; i < colValues.size(); ++i) { // bỏ qua tiêu đề
        bool ok = false;
        const double x = colValues[i].trimmed().toDouble(&ok);
        bool keep = false;
        if (ok) {
            switch (op) {
            case NumOp::Eq:         keep = (x == v1); break;
            case NumOp::Ne:         keep = (x != v1); break;
            case NumOp::Gt:         keep = (x > v1);  break;
            case NumOp::Ge:         keep = (x >= v1); break;
            case NumOp::Lt:         keep = (x < v1);  break;
            case NumOp::Le:         keep = (x <= v1); break;
            case NumOp::Between:    keep = (x >= qMin(v1, v2) && x <= qMax(v1, v2)); break;
            case NumOp::NotBetween: keep = (x < qMin(v1, v2) || x > qMax(v1, v2)); break;
            case NumOp::AboveAvg:   keep = (x > avg); break;
            case NumOp::BelowAvg:   keep = (x < avg); break;
            }
        }
        if (!keep) hide.push_back(i); // ô không phải số -> luôn ẩn
    }
    return hide;
}

} // namespace filterutil
