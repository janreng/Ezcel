#pragma once
#include <QString>

// Tô nổi bật ô theo quy tắc (Highlight Cells Rules, Spec 13) — logic thuần để kiểm thử.
// So khớp giá trị một ô với điều kiện; UI tô nền cho các ô khớp.
namespace hlrule {

enum class Op { Greater, Less, Equal, Between, Contains };

// Ô có khớp điều kiện không.
// - Greater/Less/Equal/Between: so SỐ (ô không phải số -> không khớp). Between dùng [a,b] (kể cả 2 biên).
// - Contains: so CHỮ, ô có chứa `text` (không phân biệt hoa thường); text rỗng -> không khớp.
inline bool matches(const QString &cell, Op op, double a, double b = 0.0, const QString &text = QString())
{
    if (op == Op::Contains) {
        if (text.trimmed().isEmpty()) return false;
        return cell.contains(text.trimmed(), Qt::CaseInsensitive);
    }
    bool ok = false;
    const double v = cell.trimmed().toDouble(&ok);
    if (!ok) return false;
    switch (op) {
    case Op::Greater: return v > a;
    case Op::Less:    return v < a;
    case Op::Equal:   return qFuzzyCompare(v + 1.0, a + 1.0);
    case Op::Between: {
        const double lo = qMin(a, b), hi = qMax(a, b);
        return v >= lo && v <= hi;
    }
    default: return false;
    }
}

} // namespace hlrule
