#pragma once
#include <QString>
#include <QStringList>
#include <QVector>

// Kiểm tra trợ năng (Accessibility, Spec 41): quét lưới tìm vấn đề khiến trình
// đọc màn hình / người khiếm thị khó dùng. Logic thuần để kiểm thử.
namespace a11y {

// Quét lưới, trả danh sách mô tả vấn đề (rỗng = không có vấn đề). Phát hiện:
//  - Cột trong vùng dữ liệu thiếu tiêu đề ở hàng đầu.
//  - Hàng trống nằm xen giữa vùng dữ liệu.
//  - Cột trống nằm xen giữa vùng dữ liệu.
inline QStringList check(const QVector<QVector<QString>> &grid)
{
    QStringList out;
    const int rows = grid.size();
    if (rows == 0) return out;

    auto at = [&](int r, int c) -> QString {
        return (c >= 0 && c < grid[r].size()) ? grid[r][c].trimmed() : QString();
    };

    // Vùng dữ liệu: hàng/cột cuối còn nội dung.
    int lastRow = -1, lastCol = -1;
    for (int r = 0; r < rows; ++r)
        for (int c = 0; c < grid[r].size(); ++c)
            if (!grid[r][c].trimmed().isEmpty()) { if (r > lastRow) lastRow = r; if (c > lastCol) lastCol = c; }
    if (lastRow < 0) return out; // bảng rỗng

    // 1) Tiêu đề thiếu ở hàng đầu.
    for (int c = 0; c <= lastCol; ++c)
        if (at(0, c).isEmpty())
            out << QStringLiteral("Cột %1 thiếu tiêu đề ở hàng đầu").arg(c + 1);

    // 2) Hàng trống xen giữa vùng dữ liệu.
    auto rowEmpty = [&](int r) {
        for (int c = 0; c <= lastCol; ++c) if (!at(r, c).isEmpty()) return false;
        return true;
    };
    for (int r = 1; r < lastRow; ++r)
        if (rowEmpty(r))
            out << QStringLiteral("Hàng %1 trống nằm giữa vùng dữ liệu").arg(r + 1);

    // 3) Cột trống xen giữa vùng dữ liệu.
    auto colEmpty = [&](int c) {
        for (int r = 0; r <= lastRow; ++r) if (!at(r, c).isEmpty()) return false;
        return true;
    };
    for (int c = 1; c < lastCol; ++c)
        if (colEmpty(c))
            out << QStringLiteral("Cột %1 trống nằm giữa vùng dữ liệu").arg(c + 1);

    return out;
}

} // namespace a11y
