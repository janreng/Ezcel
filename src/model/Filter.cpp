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

} // namespace filterutil
