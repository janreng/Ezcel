#include "model/Validation.h"
#include <cmath>
#include <algorithm>

namespace validation {

static bool compare(double x, Op op, double v1, double v2) {
    switch (op) {
    case Op::Between:      return x >= std::min(v1, v2) && x <= std::max(v1, v2);
    case Op::NotBetween:   return x < std::min(v1, v2) || x > std::max(v1, v2);
    case Op::Equal:        return std::fabs(x - v1) < 1e-9;
    case Op::NotEqual:     return std::fabs(x - v1) >= 1e-9;
    case Op::Greater:      return x > v1;
    case Op::Less:         return x < v1;
    case Op::GreaterEqual: return x >= v1;
    case Op::LessEqual:    return x <= v1;
    }
    return true;
}

bool check(const QString &value, Allow allow, Op op, double v1, double v2) {
    if (allow == Allow::Any) return true;
    if (value.isEmpty()) return true; // ignore blank

    double x;
    if (allow == Allow::TextLength) {
        x = value.length();
    } else {
        bool ok = false;
        x = value.toDouble(&ok);
        if (!ok) return false; // không phải số -> không hợp lệ
        if (allow == Allow::WholeNumber && std::floor(x) != x) return false;
    }
    return compare(x, op, v1, v2);
}

} // namespace validation
