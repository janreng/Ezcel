// Công cụ dữ liệu (Spec 27) — tách cột, xóa hàng trùng. Logic thuần, test được.
#pragma once
#include <QString>
#include <QStringList>
#include <QVector>
#include <vector>

namespace datatools {

// Tách `text` theo chuỗi phân cách `delim`. Nếu mergeConsecutive=true thì bỏ token rỗng
// (coi nhiều dấu phân cách liền nhau như một).
QStringList splitDelimited(const QString &text, const QString &delim, bool mergeConsecutive = false);

// Trả về chỉ số các HÀNG trùng cần xóa (giữ lần xuất hiện đầu), dựa trên giá trị ở `keyCols`.
// Nếu hasHeader=true thì bỏ qua hàng 0. Kết quả tăng dần.
QVector<int> duplicateRowIndices(const std::vector<std::vector<QString>> &rows,
                                 const QVector<int> &keyCols, bool hasHeader);

// Hàm tổng hợp cho Tổng phụ (Subtotal, Spec 27.6).
enum class Agg { Sum, Count, Average, Max, Min };

// Chèn dòng "tổng phụ" sau mỗi nhóm liên tiếp có cùng giá trị ở cột `groupCol`,
// cộng thêm dòng "tổng cộng" ở cuối. Dữ liệu phải đã sắp xếp theo `groupCol`.
// - `rows`: khối dữ liệu (KHÔNG gồm dòng tiêu đề; caller tự loại).
// - `aggCols`: các cột cần tổng hợp; `fn`: kiểu tổng hợp.
// - Dòng tổng phụ: ô ở `groupCol` = "<giá trị nhóm> <totalLabel>"; ô ở các `aggCols`
//   = kết quả tổng hợp; ô khác để trống. Dòng tổng cộng dùng nhãn `grandLabel`.
// Kết quả gồm toàn bộ dòng gốc xen kẽ dòng tổng phụ + dòng tổng cộng cuối.
QVector<QVector<QString>> subtotal(const QVector<QVector<QString>> &rows,
                                   int groupCol, const QVector<int> &aggCols, Agg fn,
                                   const QString &totalLabel, const QString &grandLabel);

// Đảo ngược thứ tự các HÀNG của khối (hàng đầu thành cuối). Giữ nguyên thứ tự cột.
QVector<QVector<QString>> reverseRows(const QVector<QVector<QString>> &rows);

// Điền ô trống bằng giá trị KHÔNG RỖNG gần nhất phía trên (theo từng cột). Hữu ích
// khi dữ liệu xuất ra chỉ ghi nhãn nhóm ở hàng đầu mỗi nhóm. Trả khối đã điền.
QVector<QVector<QString>> fillBlanksDown(const QVector<QVector<QString>> &rows);

// Gộp các cột của mỗi hàng thành MỘT chuỗi, ngăn bằng `sep` (ngược với tách cột).
// Nếu skipEmpty=true thì bỏ qua ô rỗng khi ghép. Trả 1 chuỗi cho mỗi hàng.
QStringList joinColumns(const QVector<QVector<QString>> &rows, const QString &sep, bool skipEmpty);

// Trả chỉ số các phần tử có giá trị XUẤT HIỆN NHIỀU HƠN MỘT LẦN (tất cả các lần lặp).
// So sánh không phân biệt hoa/thường; bỏ qua ô rỗng. Dùng để tô/chọn ô trùng.
QVector<int> duplicateValueIndices(const QVector<QString> &values);

} // namespace datatools
