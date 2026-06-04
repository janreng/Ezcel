#include "model/NumberFormat.h"
#include <QHash>
#include <QDate>
#include <QDateTime>
#include <cmath>

namespace numfmt {

QString adjustDecimals(const QString &codeIn, int delta) {
    QString code = codeIn.isEmpty() ? QStringLiteral("0") : codeIn;
    QString suffix;
    if (code.endsWith(QLatin1Char('%'))) { suffix = QStringLiteral("%"); code.chop(1); }
    const int dot = code.indexOf(QLatin1Char('.'));
    const QString intPart = dot < 0 ? code : code.left(dot);
    const int curDecimals = dot < 0 ? 0 : code.size() - dot - 1;
    int n = curDecimals + delta;
    if (n < 0) n = 0;
    if (n > 10) n = 10;
    QString out = intPart;
    if (n > 0) out += QLatin1Char('.') + QString(n, QLatin1Char('0'));
    return out + suffix;
}

static const QHash<QString, QString> &dateCodes() {
    static const QHash<QString, QString> m = {
        {"dd/mm/yyyy", "dd/MM/yyyy"}, {"mm/dd/yyyy", "MM/dd/yyyy"},
        {"yyyy-mm-dd", "yyyy-MM-dd"}, {"hh:mm:ss", "HH:mm:ss"}, {"hh:mm", "HH:mm"},
    };
    return m;
}

// Chèn dấu phẩy hàng nghìn vào phần nguyên của chuỗi số "1234.50".
static QString withThousands(QString core) {
    int dot = core.indexOf('.');
    QString intPart = dot < 0 ? core : core.left(dot);
    QString frac = dot < 0 ? QString() : core.mid(dot);
    bool neg = intPart.startsWith('-');
    if (neg) intPart.remove(0, 1);
    for (int i = intPart.size() - 3; i > 0; i -= 3) intPart.insert(i, ',');
    return (neg ? "-" : "") + intPart + frac;
}

// Xấp xỉ phân số mẫu số <= maxDen. Trả "w p/q" hoặc "p/q".
static QString toFraction(double val, int maxDen) {
    int sign = val < 0 ? -1 : 1;
    val = std::abs(val);
    long long whole = (long long)std::floor(val);
    double frac = val - whole;
    int bestP = 0, bestQ = 1; double bestErr = frac;
    for (int q = 1; q <= maxDen; ++q) {
        int p = int(std::round(frac * q));
        double err = std::abs(frac - double(p) / q);
        if (err < bestErr) { bestErr = err; bestP = p; bestQ = q; }
    }
    QString s = sign < 0 ? "-" : "";
    if (bestP == 0) return s + QString::number(whole);
    if (whole == 0) return s + QStringLiteral("%1/%2").arg(bestP).arg(bestQ);
    return s + QStringLiteral("%1 %2/%3").arg(whole).arg(bestP).arg(bestQ);
}

QString apply(const QVariant &v, const QString &code) {
    if (v.typeId() != QMetaType::Double && v.typeId() != QMetaType::Int
        && v.typeId() != QMetaType::LongLong)
        return QString();
    double val = v.toDouble();

    if (dateCodes().contains(code)) {
        QDateTime dt(QDate(1899, 12, 30).startOfDay());
        dt = dt.addMSecs(qint64(val * 86400000.0));
        return dt.isValid() ? dt.toString(dateCodes().value(code)) : QString();
    }

    // Khoa học: "0.00E+00".
    if (code.contains('E', Qt::CaseInsensitive) && !code.contains('/')) {
        int dec = 0; int e = code.indexOf('E', 0, Qt::CaseInsensitive);
        int dot = code.indexOf('.');
        if (dot >= 0 && dot < e) for (int i = dot + 1; i < e; ++i) if (code[i] == '0') ++dec;
        return QString::number(val, 'E', dec);
    }

    // Phân số: "# ?/?" (mẫu 1 chữ số) hoặc "# ??/??" (2 chữ số).
    if (code.contains('/')) {
        int q = code.count('?');
        return toFraction(val, q >= 2 ? 99 : 9);
    }

    bool currency = code.contains('$');
    bool percent = code.contains('%');
    QString body = code; body.remove('%'); body.remove('$');
    double v2 = percent ? val * 100 : val;
    int decimals = 0, dot = body.indexOf('.');
    if (dot >= 0) for (int i = dot + 1; i < body.size(); ++i) if (body[i] == '0') ++decimals;
    bool thousands = body.left(dot < 0 ? body.size() : dot).contains(',');

    QString core = QString::number(std::abs(v2), 'f', decimals);
    if (thousands) core = withThousands(core);
    QString sign = v2 < 0 ? "-" : "";
    return sign + (currency ? "$" : "") + core + (percent ? "%" : "");
}

} // namespace numfmt
