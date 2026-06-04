#pragma once
#include <QString>

// Định dạng "bảng có cấu trúc" (Spec 16): một vùng được tô SỌC XEN KẼ màu theo
// hàng + (tùy chọn) hàng tiêu đề tô đậm. Logic thuần, tách khỏi model để test.
namespace tbl {

struct Table {
    int top = 0, left = 0, bottom = 0, right = 0;
    bool header = true;                  // hàng đầu là tiêu đề (tô màu đậm riêng)
    QString headerColor;                 // màu nền hàng tiêu đề
    QString band1;                       // màu hàng dữ liệu chẵn (offset 0,2,4…)
    QString band2;                       // màu hàng dữ liệu lẻ (offset 1,3,5…)
};

// Chọn màu sọc theo độ lệch hàng trong vùng dữ liệu: chẵn -> c1, lẻ -> c2.
inline QString tableStripeColor(int rowOffset, const QString &c1, const QString &c2)
{
    return (rowOffset % 2 == 0) ? c1 : c2;
}

// Màu nền cho ô (row,col) nếu nằm trong bảng t; chuỗi rỗng nếu ngoài vùng.
QString stripeColorAt(const Table &t, int row, int col);

// Công thức tổng cho 1 cột của bảng: =SUM(<cột><hàngĐầu>:<cột><hàngCuối>)
// (số hàng là 1-based như hiển thị). Thuần chuỗi để kiểm thử.
inline QString sumFormula(const QString &colLabel, int top1Based, int bottom1Based)
{
    return QStringLiteral("=SUM(%1%2:%1%3)").arg(colLabel).arg(top1Based).arg(bottom1Based);
}

} // namespace tbl
