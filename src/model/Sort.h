#pragma once
#include <QString>
#include <QVector>

// Sắp xếp vùng dữ liệu kiểu Excel — module logic THUẦN (không phụ thuộc GUI/model).
// Dùng cho "Data → Sort": sắp các HÀNG của một block theo một hoặc nhiều cột khóa.
// Tách riêng để test headless và để ghép vào model/GUI ở commit sau.
namespace sort {

enum class Order { Ascending, Descending };

// Một cấp khóa sắp xếp: cột (chỉ số trong block) + chiều.
struct SortKey {
    int column;
    Order order = Order::Ascending;
};

// So sánh 2 ô kiểu Excel theo chiều TĂNG DẦN (chưa xét quy tắc ô rỗng — xem sortRows):
//   • Số đứng trước chữ.
//   • Số so theo GIÁ TRỊ ("10" > "9"), chữ so KHÔNG phân biệt hoa-thường.
// Trả <0 nếu a<b, 0 nếu bằng, >0 nếu a>b.
int compareCells(const QString &a, const QString &b);

// Sắp xếp ỔN ĐỊNH (stable) các hàng của `rows` theo danh sách khóa (đa cấp).
// Ô RỖNG luôn xuống cuối ở CẢ HAI chiều (giống Excel). Hàng giữ nguyên — chỉ đổi
// thứ tự hàng, không tách/ghép ô. `keys` rỗng -> trả nguyên block.
QVector<QVector<QString>> sortRows(const QVector<QVector<QString>> &rows,
                                   const QVector<SortKey> &keys);

} // namespace sort
