#include "model/RangeParse.h"

namespace rangeparse {

namespace {
// "A" -> 0, "B" -> 1, ... "AA" -> 26. Trả -1 nếu rỗng/không phải chữ.
int colFromLetters(const QString &s) {
    if (s.isEmpty()) return -1;
    int col = 0;
    for (const QChar &ch : s) {
        if (!ch.isLetter()) return -1;
        col = col * 26 + (ch.toUpper().unicode() - 'A' + 1);
    }
    return col - 1;
}

// Tách 1 nửa "A1" thành (col, row); col/row = -1 nếu thiếu phần đó.
// "A" -> col only; "1" -> row only; "A1" -> cả hai.
struct Half { int col = -1; int row = -1; bool ok = false; };
Half parseHalf(const QString &t) {
    Half h;
    QString s = t.trimmed();
    if (s.isEmpty()) return h;
    int i = 0;
    while (i < s.size() && s[i].isLetter()) ++i;
    const QString letters = s.left(i);
    const QString digits = s.mid(i);
    if (!letters.isEmpty()) { h.col = colFromLetters(letters); if (h.col < 0) return h; }
    if (!digits.isEmpty()) {
        bool ok = false; int r = digits.toInt(&ok);
        if (!ok || r < 1) return h;
        h.row = r - 1;
    }
    // Phải có ít nhất cột hoặc hàng; nếu có chữ thì phải parse được.
    h.ok = (h.col >= 0 || h.row >= 0) && (digits.isEmpty() || h.row >= 0);
    return h;
}
} // namespace

std::optional<MergeRange> parseOne(const QString &token, int rows, int cols) {
    const QString t = token.trimmed();
    if (t.isEmpty() || rows <= 0 || cols <= 0) return std::nullopt;

    const int colon = t.indexOf(QLatin1Char(':'));
    auto clampRange = [&](int top, int left, int bottom, int right) -> std::optional<MergeRange> {
        if (top < 0 || left < 0) return std::nullopt;
        top = qBound(0, top, rows - 1); bottom = qBound(0, bottom, rows - 1);
        left = qBound(0, left, cols - 1); right = qBound(0, right, cols - 1);
        if (top > bottom) std::swap(top, bottom);
        if (left > right) std::swap(left, right);
        return MergeRange{top, left, bottom, right};
    };

    if (colon < 0) {
        Half h = parseHalf(t);
        if (!h.ok || h.col < 0 || h.row < 0) return std::nullopt; // ô đơn phải đủ cột+hàng
        return clampRange(h.row, h.col, h.row, h.col);
    }

    Half a = parseHalf(t.left(colon));
    Half b = parseHalf(t.mid(colon + 1));
    if (!a.ok || !b.ok) return std::nullopt;

    // Cả cột "A:C" (không có số) -> mọi hàng.
    if (a.row < 0 && b.row < 0 && a.col >= 0 && b.col >= 0)
        return clampRange(0, a.col, rows - 1, b.col);
    // Cả hàng "1:3" (không có chữ) -> mọi cột.
    if (a.col < 0 && b.col < 0 && a.row >= 0 && b.row >= 0)
        return clampRange(a.row, 0, b.row, cols - 1);
    // Vùng ô đầy đủ "A1:B3".
    if (a.col >= 0 && a.row >= 0 && b.col >= 0 && b.row >= 0)
        return clampRange(a.row, a.col, b.row, b.col);
    return std::nullopt;
}

QVector<MergeRange> parseMulti(const QString &text, int rows, int cols) {
    QVector<MergeRange> out;
    const QStringList parts = text.split(QLatin1Char(','), Qt::SkipEmptyParts);
    for (const QString &p : parts)
        if (auto r = parseOne(p, rows, cols)) out.push_back(*r);
    return out;
}

} // namespace rangeparse
