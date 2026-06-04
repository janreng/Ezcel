#include "model/RefCycle.h"
#include <QRegularExpression>

namespace refcycle {

// Tham chiếu ô: [$]CỘT[$]HÀNG, cột là 1-3 chữ cái, hàng là số.
static QRegularExpression refRe() {
    return QRegularExpression(QStringLiteral("(\\$?)([A-Za-z]{1,3})(\\$?)([0-9]+)"));
}

// Dựng lại ref từ các phần với trạng thái khóa mới.
static QString rebuild(const QString &col, const QString &rowNum, bool colAbs, bool rowAbs) {
    return (colAbs ? QStringLiteral("$") : QString()) + col
         + (rowAbs ? QStringLiteral("$") : QString()) + rowNum;
}

// Trạng thái kế tiếp theo vòng Excel: none -> both -> rowOnly -> colOnly -> none.
static void nextState(bool &colAbs, bool &rowAbs) {
    if (!colAbs && !rowAbs)      { colAbs = true;  rowAbs = true;  } // none -> both
    else if (colAbs && rowAbs)   { colAbs = false; rowAbs = true;  } // both -> rowOnly
    else if (!colAbs && rowAbs)  { colAbs = true;  rowAbs = false; } // rowOnly -> colOnly
    else                         { colAbs = false; rowAbs = false; } // colOnly -> none
}

QString cycle(const QString &ref) {
    QRegularExpression re(QStringLiteral("^") + refRe().pattern() + QStringLiteral("$"));
    QRegularExpressionMatch m = re.match(ref.trimmed());
    if (!m.hasMatch()) return ref;
    bool colAbs = !m.captured(1).isEmpty();
    bool rowAbs = !m.captured(3).isEmpty();
    nextState(colAbs, rowAbs);
    return rebuild(m.captured(2), m.captured(4), colAbs, rowAbs);
}

QString cycleAt(const QString &text, int pos) {
    QRegularExpression re = refRe();
    QRegularExpressionMatchIterator it = re.globalMatch(text);
    while (it.hasNext()) {
        QRegularExpressionMatch m = it.next();
        const int start = m.capturedStart(0);
        const int end = m.capturedEnd(0); // vị trí ngay sau token
        // Con trỏ nằm trong hoặc kề token (cho phép ở cuối token).
        if (pos >= start && pos <= end) {
            bool colAbs = !m.captured(1).isEmpty();
            bool rowAbs = !m.captured(3).isEmpty();
            nextState(colAbs, rowAbs);
            const QString rebuilt = rebuild(m.captured(2), m.captured(4), colAbs, rowAbs);
            return text.left(start) + rebuilt + text.mid(end);
        }
    }
    return text;
}

} // namespace refcycle
