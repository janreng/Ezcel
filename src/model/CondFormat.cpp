#include "model/CondFormat.h"
#include <cmath>

namespace cond {

bool match(const QVariant &value, Op op, double v1, double v2, const QString &text) {
    if (op == Op::Contains)
        return !text.isEmpty() && value.toString().contains(text, Qt::CaseInsensitive);

    bool ok = false;
    double d = value.toDouble(&ok);
    if (!ok) return false; // ô không phải số -> các phép số không khớp
    switch (op) {
    case Op::GreaterThan: return d > v1;
    case Op::LessThan:    return d < v1;
    case Op::Equal:       return std::fabs(d - v1) < 1e-9;
    case Op::NotEqual:    return std::fabs(d - v1) >= 1e-9;
    case Op::Between:     return d >= std::min(v1, v2) && d <= std::max(v1, v2);
    default:              return false;
    }
}

double dataBarFraction(double value, double mn, double mx) {
    if (mx <= mn) return 1.0;
    double f = (value - mn) / (mx - mn);
    if (f < 0.0) return 0.0;
    if (f > 1.0) return 1.0;
    return f;
}

namespace {
void parseHex(const QString &s, int &r, int &g, int &b) {
    QString h = s.startsWith(QLatin1Char('#')) ? s.mid(1) : s;
    if (h.size() >= 6) {
        r = h.mid(0, 2).toInt(nullptr, 16);
        g = h.mid(2, 2).toInt(nullptr, 16);
        b = h.mid(4, 2).toInt(nullptr, 16);
    } else { r = g = b = 0; }
}
} // namespace

QString lerpHex(const QString &a, const QString &b, double t) {
    if (t < 0.0) t = 0.0; if (t > 1.0) t = 1.0;
    int r1, g1, b1, r2, g2, b2;
    parseHex(a, r1, g1, b1);
    parseHex(b, r2, g2, b2);
    const int r = int(r1 + (r2 - r1) * t + 0.5);
    const int g = int(g1 + (g2 - g1) * t + 0.5);
    const int bb = int(b1 + (b2 - b1) * t + 0.5);
    return QStringLiteral("#%1%2%3")
        .arg(r, 2, 16, QLatin1Char('0')).arg(g, 2, 16, QLatin1Char('0')).arg(bb, 2, 16, QLatin1Char('0'));
}

QString colorScale(double fraction, const QString &low, const QString &mid, const QString &high) {
    if (fraction < 0.0) fraction = 0.0; if (fraction > 1.0) fraction = 1.0;
    if (mid.isEmpty()) return lerpHex(low, high, fraction);
    if (fraction <= 0.5) return lerpHex(low, mid, fraction / 0.5);
    return lerpHex(mid, high, (fraction - 0.5) / 0.5);
}

int iconIndex(double fraction, int nIcons) {
    if (nIcons <= 1) return 0;
    if (fraction < 0.0) fraction = 0.0; if (fraction > 1.0) fraction = 1.0;
    int idx = int(fraction * nIcons);
    if (idx >= nIcons) idx = nIcons - 1; // fraction==1 rơi vào mức cao nhất
    return idx;
}

} // namespace cond
