#pragma once
#include <QString>
#include <QVector>

// Lấy & Biến đổi dữ liệu (Get & Transform, Spec 20) — logic thuần để kiểm thử.
// Bản 1: chọn cột muốn giữ + bỏ dòng trống.
namespace dquery {

using Grid = QVector<QVector<QString>>;

// Giữ lại các cột theo chỉ số trong keep (đúng thứ tự keep); bỏ các cột khác.
// Chỉ số ngoài phạm vi bị bỏ qua. keep rỗng -> trả lưới rỗng.
inline Grid selectColumns(const Grid &grid, const QVector<int> &keep)
{
    Grid out;
    if (keep.isEmpty()) return out;
    out.reserve(grid.size());
    for (const auto &row : grid) {
        QVector<QString> nr;
        nr.reserve(keep.size());
        for (int c : keep)
            nr << (c >= 0 && c < row.size() ? row[c] : QString());
        out << nr;
    }
    return out;
}

// Dòng có rỗng hoàn toàn không (mọi ô trimmed đều rỗng)?
inline bool isEmptyRow(const QVector<QString> &row)
{
    for (const QString &c : row)
        if (!c.trimmed().isEmpty()) return false;
    return true;
}

// Bỏ các dòng rỗng hoàn toàn.
inline Grid removeEmptyRows(const Grid &grid)
{
    Grid out;
    out.reserve(grid.size());
    for (const auto &row : grid)
        if (!isEmptyRow(row)) out << row;
    return out;
}

// Áp các bước biến đổi theo thứ tự: chọn cột (nếu keep không rỗng) rồi bỏ dòng trống (nếu bật).
inline Grid apply(const Grid &grid, const QVector<int> &keep, bool dropEmptyRows)
{
    Grid out = keep.isEmpty() ? grid : selectColumns(grid, keep);
    if (dropEmptyRows) out = removeEmptyRows(out);
    return out;
}

} // namespace dquery
