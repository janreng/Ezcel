#pragma once
#include <QString>
#include <QStringList>
#include <QVector>

// Phân tích nhanh (Quick Analysis, Spec 40): gợi ý thao tác phù hợp cho vùng chọn.
// Logic thuần để kiểm thử. Nhãn gợi ý dùng trực tiếp trên menu.
namespace quickanalysis {

// Vùng [t..b]×[l..r] có ít nhất một ô là SỐ không?
inline bool hasNumbers(const QVector<QVector<QString>> &grid, int t, int l, int b, int r)
{
    for (int row = t; row <= b && row < grid.size(); ++row)
        for (int c = l; c <= r && c < grid[row].size(); ++c) {
            bool ok = false;
            grid[row][c].trimmed().toDouble(&ok);
            if (ok && !grid[row][c].trimmed().isEmpty()) return true;
        }
    return false;
}

// Danh sách gợi ý cho vùng chọn. Có số -> kèm Tổng/Thanh dữ liệu/Thang màu;
// luôn có "Định dạng là bảng".
inline QStringList suggest(const QVector<QVector<QString>> &grid, int t, int l, int b, int r)
{
    QStringList s;
    if (hasNumbers(grid, t, l, b, r)) {
        s << QStringLiteral("Tổng cuối vùng")
          << QStringLiteral("Thanh dữ liệu")
          << QStringLiteral("Thang màu");
    }
    s << QStringLiteral("Định dạng là bảng");
    return s;
}

} // namespace quickanalysis
