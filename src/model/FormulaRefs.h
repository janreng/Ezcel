#pragma once
#include "model/SpreadsheetModel.h" // MergeRange
#include <QString>
#include <QVector>
#include <QRegularExpression>

// Trích các vùng tham chiếu (ô đơn "A1" hoặc vùng "A1:B3", có/không $) từ một công thức,
// để tô viền nét đứt "kiến bò" khi đang nhập công thức (point mode, Spec 12).
// Logic thuần để kiểm thử.
namespace formularefs {

// "A" -> 0, "B" -> 1, "AA" -> 26... (-1 nếu rỗng/sai).
inline int colToIndex(const QString &letters)
{
    if (letters.isEmpty()) return -1;
    int n = 0;
    for (QChar ch : letters) {
        if (!ch.isLetter()) return -1;
        n = n * 26 + (ch.toUpper().unicode() - 'A' + 1);
    }
    return n - 1;
}

// Phân tích một địa chỉ ô "A1"/"$A$1" -> (row,col) 0-based; trả false nếu sai.
inline bool parseCell(const QString &ref, int &row, int &col)
{
    static const QRegularExpression re(QStringLiteral("^\\$?([A-Za-z]{1,3})\\$?([0-9]+)$"));
    const QRegularExpressionMatch m = re.match(ref.trimmed());
    if (!m.hasMatch()) return false;
    col = colToIndex(m.captured(1));
    bool ok = false;
    const int r = m.captured(2).toInt(&ok);
    if (col < 0 || !ok || r < 1) return false;
    row = r - 1;
    return true;
}

// Trích mọi vùng tham chiếu từ công thức, đã kẹp trong [0,rows)×[0,cols).
inline QVector<MergeRange> extract(const QString &formula, int rows, int cols)
{
    QVector<MergeRange> out;
    static const QRegularExpression re(QStringLiteral(
        "\\$?[A-Za-z]{1,3}\\$?[0-9]+(?::\\$?[A-Za-z]{1,3}\\$?[0-9]+)?"));
    auto it = re.globalMatch(formula);
    while (it.hasNext()) {
        const QString tok = it.next().captured(0);
        const int colon = tok.indexOf(QLatin1Char(':'));
        int r1, c1, r2, c2;
        if (colon < 0) {
            if (!parseCell(tok, r1, c1)) continue;
            r2 = r1; c2 = c1;
        } else {
            if (!parseCell(tok.left(colon), r1, c1)) continue;
            if (!parseCell(tok.mid(colon + 1), r2, c2)) continue;
        }
        MergeRange mr;
        mr.top = qBound(0, qMin(r1, r2), rows > 0 ? rows - 1 : 0);
        mr.bottom = qBound(0, qMax(r1, r2), rows > 0 ? rows - 1 : 0);
        mr.left = qBound(0, qMin(c1, c2), cols > 0 ? cols - 1 : 0);
        mr.right = qBound(0, qMax(c1, c2), cols > 0 ? cols - 1 : 0);
        out << mr;
    }
    return out;
}

} // namespace formularefs
