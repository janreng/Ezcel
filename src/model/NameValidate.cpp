#include "model/NameValidate.h"
#include <QRegularExpression>

namespace namevalidate {

static bool looksLikeCellRef(const QString &s) {
    static const QRegularExpression a1(QStringLiteral("^\\$?[A-Za-z]{1,3}\\$?[0-9]+$"));
    static const QRegularExpression r1c1(QStringLiteral("^[Rr][0-9]+[Cc][0-9]+$"));
    return a1.match(s).hasMatch() || r1c1.match(s).hasMatch();
}

QString reason(const QString &name) {
    const QString n = name; // không trim: khoảng trắng là không hợp lệ
    if (n.isEmpty()) return QStringLiteral("Tên rỗng");
    if (n.size() > 255) return QStringLiteral("Tên quá dài (tối đa 255 ký tự)");
    if (n.contains(QLatin1Char(' '))) return QStringLiteral("Tên không được chứa khoảng trắng");

    const QChar c0 = n.at(0);
    if (!(c0.isLetter() || c0 == QLatin1Char('_') || c0 == QLatin1Char('\\')))
        return QStringLiteral("Ký tự đầu phải là chữ cái, '_' hoặc '\\'");

    for (int i = 1; i < n.size(); ++i) {
        const QChar ch = n.at(i);
        if (!(ch.isLetterOrNumber() || ch == QLatin1Char('.') || ch == QLatin1Char('_')))
            return QStringLiteral("Chỉ được dùng chữ, số, '.', '_'");
    }

    if (n.compare(QStringLiteral("C"), Qt::CaseInsensitive) == 0 ||
        n.compare(QStringLiteral("R"), Qt::CaseInsensitive) == 0)
        return QStringLiteral("\"C\"/\"R\" là tên dành riêng");

    if (looksLikeCellRef(n))
        return QStringLiteral("Tên không được trùng dạng địa chỉ ô (vd A1, R1C1)");

    return QString();
}

bool isValid(const QString &name) { return reason(name).isEmpty(); }

} // namespace namevalidate
