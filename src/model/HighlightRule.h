#pragma once
#include <QString>
#include <QStringList>
#include <QVector>
#include <QSet>
#include <QHash>
#include <algorithm>

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

// Tập CHỈ SỐ của n ô SỐ lớn nhất (isTop=true) hoặc nhỏ nhất (isTop=false) trong `cells`.
// Ô không phải số bị bỏ qua. Bằng điểm ở ngưỡng -> giữ tất cả (như Excel, có thể >n).
inline QSet<int> topN(const QStringList &cells, int n, bool isTop)
{
    QSet<int> out;
    if (n <= 0) return out;
    QVector<double> nums; // (giá trị) của các ô là số
    QVector<int> idxOf;
    for (int i = 0; i < cells.size(); ++i) {
        bool ok = false; const double v = cells[i].trimmed().toDouble(&ok);
        if (ok) { nums << v; idxOf << i; }
    }
    if (nums.isEmpty()) return out;
    QVector<double> sorted = nums;
    std::sort(sorted.begin(), sorted.end(), [isTop](double a, double b){ return isTop ? a > b : a < b; });
    const int k = qMin(n, sorted.size());
    const double threshold = sorted[k - 1]; // ngưỡng để vào top/bottom n (kể cả đồng hạng)
    for (int j = 0; j < nums.size(); ++j) {
        if (isTop ? (nums[j] >= threshold) : (nums[j] <= threshold)) out.insert(idxOf[j]);
    }
    return out;
}

// Tập CHỈ SỐ các ô có giá trị TRÙNG LẶP (xuất hiện >1 lần). So chuỗi trimmed, không phân biệt
// hoa thường; ô rỗng bị bỏ qua.
inline QSet<int> duplicates(const QStringList &cells)
{
    QHash<QString, int> count;
    for (const QString &c : cells) {
        const QString k = c.trimmed().toLower();
        if (!k.isEmpty()) ++count[k];
    }
    QSet<int> out;
    for (int i = 0; i < cells.size(); ++i) {
        const QString k = cells[i].trimmed().toLower();
        if (!k.isEmpty() && count.value(k) > 1) out.insert(i);
    }
    return out;
}

} // namespace hlrule
