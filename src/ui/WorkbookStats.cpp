// Thống kê bảng tính (Spec 57.1).
#include "ui/WorkbookStats.h"
#include <QRegularExpression>

namespace wbstats {

Result analyze(const std::vector<QString> &cellTexts)
{
    Result r;
    static const QRegularExpression ws(QStringLiteral("\\s+"));
    for (const QString &raw : cellTexts) {
        const QString t = raw.trimmed();
        if (t.isEmpty()) continue;
        ++r.cellsWithData;
        if (t.startsWith('=')) { ++r.formulas; continue; }
        bool ok = false;
        t.toDouble(&ok);
        if (ok) { ++r.numbers; continue; }
        // Ô văn bản: đếm số từ.
        const auto parts = t.split(ws, Qt::SkipEmptyParts);
        r.words += int(parts.size());
    }
    return r;
}

Result &add(Result &dst, const Result &src)
{
    dst.cellsWithData += src.cellsWithData;
    dst.formulas += src.formulas;
    dst.numbers += src.numbers;
    dst.words += src.words;
    return dst;
}

} // namespace wbstats
