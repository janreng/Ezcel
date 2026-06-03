// Công cụ dữ liệu (Spec 27).
#include "model/DataTools.h"
#include <QSet>

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

} // namespace datatools
