#include "model/Filter.h"

namespace filterutil {

QVector<int> rowsToHide(const QVector<QString> &colValues, const QString &text) {
    QVector<int> hide;
    if (text.isEmpty()) return hide;
    for (int i = 1; i < colValues.size(); ++i) // bỏ qua hàng 0 (tiêu đề)
        if (!colValues[i].contains(text, Qt::CaseInsensitive))
            hide.push_back(i);
    return hide;
}

} // namespace filterutil
