// Gộp dữ liệu nhiều vùng theo nhãn (Consolidate, Spec 27). Logic thuần, test được.
#pragma once
#include "model/DataTools.h" // dùng chung datatools::Agg
#include <QString>
#include <QVector>

namespace consolidate {

// Mỗi "bảng có nhãn": hàng 0 = nhãn cột (ô [0][0] là góc, bỏ qua), cột 0 = nhãn hàng,
// phần trong là số. Gộp NHIỀU bảng theo nhãn: nhãn hàng/cột lấy hợp (union) các bảng,
// mỗi ô = tổng hợp `fn` của tất cả giá trị cùng (nhãn hàng, nhãn cột) trên mọi bảng.
// Trả bảng kết quả cùng định dạng: [góc + nhãn cột...] rồi từng [nhãn hàng + giá trị...].
// Nhãn sắp xếp tăng dần (không phân biệt hoa/thường); ô không có dữ liệu để trống.
QVector<QVector<QString>> byLabels(const QVector<QVector<QVector<QString>>> &tables,
                                   datatools::Agg fn, const QString &corner);

} // namespace consolidate
