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

} // namespace cond
