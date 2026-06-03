// AutoComplete cột text (Spec 05).
#include "model/AutoComplete.h"
#include <QSet>
#include <algorithm>

namespace autocomplete {

// Giá trị có phải text (không rỗng, không phải số, không phải công thức)?
static bool isText(const QString &t) {
    if (t.isEmpty() || t.startsWith('=')) return false;
    bool ok = false;
    t.toDouble(&ok);
    return !ok;
}

QString suggest(const std::vector<QString> &colValues, const QString &prefix)
{
    const QString p = prefix.trimmed();
    if (p.isEmpty()) return QString();
    const QString pl = p.toLower();
    QSet<QString> distinct;
    QString match;
    for (const QString &raw : colValues) {
        const QString t = raw.trimmed();
        if (!isText(t)) continue;
        if (t.toLower() == pl) continue;             // trùng prefix -> không gợi ý
        if (!t.toLower().startsWith(pl)) continue;
        if (!distinct.contains(t)) { distinct.insert(t); match = t; }
        if (distinct.size() > 1) return QString();   // nhiều match khác nhau -> không gợi ý
    }
    return distinct.size() == 1 ? match : QString();
}

QStringList uniqueTexts(const std::vector<QString> &colValues)
{
    QStringList out;
    QSet<QString> seen;
    for (const QString &raw : colValues) {
        const QString t = raw.trimmed();
        if (!isText(t) || seen.contains(t)) continue;
        seen.insert(t);
        out << t;
    }
    out.sort(Qt::CaseInsensitive);
    return out;
}

} // namespace autocomplete
