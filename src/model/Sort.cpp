#include "model/Sort.h"
#include <algorithm>

namespace sort {

// Thử đọc ô như số (toàn bộ chuỗi đã trim phải là số). true + đặt `out` nếu là số.
static bool asNumber(const QString &s, double &out) {
    const QString t = s.trimmed();
    if (t.isEmpty()) return false;
    bool ok = false;
    const double v = t.toDouble(&ok);
    if (ok) { out = v; return true; }
    return false;
}

int compareCells(const QString &a, const QString &b) {
    double na = 0, nb = 0;
    const bool an = asNumber(a, na);
    const bool bn = asNumber(b, nb);
    if (an && bn) {
        if (na < nb) return -1;
        if (na > nb) return 1;
        return 0;
    }
    // Số đứng trước chữ.
    if (an && !bn) return -1;
    if (!an && bn) return 1;
    // Cả hai là chữ: so KHÔNG phân biệt hoa-thường; nếu hòa thì tie-break có phân
    // biệt để kết quả xác định và ổn định.
    const int ci = QString::compare(a, b, Qt::CaseInsensitive);
    if (ci != 0) return ci;
    return QString::compare(a, b, Qt::CaseSensitive);
}

QVector<QVector<QString>> sortRows(const QVector<QVector<QString>> &rows,
                                   const QVector<SortKey> &keys) {
    // Đọc ô (hàng, cột) an toàn — ngoài biên coi như rỗng.
    auto cellAt = [](const QVector<QString> &row, int col) -> QString {
        return (col >= 0 && col < row.size()) ? row.at(col) : QString();
    };

    // Sắp theo chỉ số hàng để stable_sort giữ thứ tự gốc khi bằng nhau.
    QVector<int> idx(rows.size());
    for (int i = 0; i < rows.size(); ++i) idx[i] = i;

    std::stable_sort(idx.begin(), idx.end(), [&](int lhs, int rhs) {
        for (const SortKey &k : keys) {
            const QString a = cellAt(rows.at(lhs), k.column);
            const QString b = cellAt(rows.at(rhs), k.column);
            const bool ae = a.trimmed().isEmpty();
            const bool be = b.trimmed().isEmpty();
            // Ô rỗng luôn xuống cuối ở cả hai chiều (độc lập với Order).
            if (ae && be) continue;   // bằng nhau theo khóa này -> xét khóa kế
            if (ae) return false;     // a rỗng -> a xếp sau b
            if (be) return true;      // b rỗng -> a xếp trước b
            int c = compareCells(a, b);
            if (k.order == Order::Descending) c = -c;
            if (c != 0) return c < 0;
        }
        return false; // hòa toàn bộ khóa -> stable giữ nguyên thứ tự gốc
    });

    QVector<QVector<QString>> out;
    out.reserve(rows.size());
    for (int i : idx) out.append(rows.at(i));
    return out;
}

} // namespace sort
