#include "model/TextSearch.h"

namespace textsearch {

QString replaceSubstr(const QString &text, const QString &find, const QString &repl, bool matchCase) {
    if (find.isEmpty()) return text;
    QString out = text;
    out.replace(find, repl, matchCase ? Qt::CaseSensitive : Qt::CaseInsensitive);
    return out;
}

std::optional<QPair<int, int>> findNext(
    int rows, int cols, int startRow, int startCol,
    const QString &needle, bool matchCase,
    const std::function<QString(int, int)> &cellText)
{
    const qint64 total = qint64(rows) * cols;
    if (total <= 0 || needle.trimmed().isEmpty()) return std::nullopt;

    Qt::CaseSensitivity cs = matchCase ? Qt::CaseSensitive : Qt::CaseInsensitive;
    if (startRow < 0) startRow = 0;
    // Bắt đầu từ ngay sau ô hiện tại (startCol có thể = -1 khi chưa chọn).
    qint64 begin = qint64(startRow) * cols + startCol + 1;
    for (qint64 off = 0; off < total; ++off) {
        qint64 pos = (begin + off) % total;
        int r = int(pos / cols), c = int(pos % cols);
        if (cellText(r, c).contains(needle, cs))
            return QPair<int, int>(r, c);
    }
    return std::nullopt;
}

} // namespace textsearch
