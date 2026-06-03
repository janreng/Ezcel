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

bool matchCond(const QString &cell, FiltOp op, const QString &operand) {
    const QString c = cell.trimmed();
    switch (op) {
    case FiltOp::Contains:    return c.contains(operand, Qt::CaseInsensitive);
    case FiltOp::NotContains: return !c.contains(operand, Qt::CaseInsensitive);
    case FiltOp::BeginsWith:  return c.startsWith(operand, Qt::CaseInsensitive);
    case FiltOp::EndsWith:    return c.endsWith(operand, Qt::CaseInsensitive);
    default: break;
    }
    // Eq..Le: ưu tiên so theo số nếu cả hai là số.
    bool cn = false, on = false;
    const double cv = c.toDouble(&cn);
    const double ov = operand.trimmed().toDouble(&on);
    if (cn && on) {
        switch (op) {
        case FiltOp::Eq: return cv == ov;
        case FiltOp::Ne: return cv != ov;
        case FiltOp::Gt: return cv > ov;
        case FiltOp::Ge: return cv >= ov;
        case FiltOp::Lt: return cv < ov;
        case FiltOp::Le: return cv <= ov;
        default: return false;
        }
    }
    const int cmp = c.compare(operand.trimmed(), Qt::CaseInsensitive);
    switch (op) {
    case FiltOp::Eq: return cmp == 0;
    case FiltOp::Ne: return cmp != 0;
    case FiltOp::Gt: return cmp > 0;
    case FiltOp::Ge: return cmp >= 0;
    case FiltOp::Lt: return cmp < 0;
    case FiltOp::Le: return cmp <= 0;
    default: return false;
    }
}

QVector<int> rowsToHideCustom(const QVector<QString> &colValues,
                              FiltOp op1, const QString &v1,
                              bool useAnd, bool hasSecond,
                              FiltOp op2, const QString &v2) {
    QVector<int> hide;
    for (int i = 1; i < colValues.size(); ++i) { // bỏ qua tiêu đề
        bool keep = matchCond(colValues[i], op1, v1);
        if (hasSecond) {
            const bool m2 = matchCond(colValues[i], op2, v2);
            keep = useAnd ? (keep && m2) : (keep || m2);
        }
        if (!keep) hide.push_back(i);
    }
    return hide;
}

} // namespace filterutil
