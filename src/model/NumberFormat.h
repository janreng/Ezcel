#pragma once
#include <QString>
#include <QVariant>

// Áp mã định dạng số (number_format) -> chuỗi hiển thị. Logic thuần, test được.
// Hỗ trợ: ngày/giờ, phần trăm, dấu phẩy hàng nghìn + thập phân, khoa học (E),
// tiền tệ ($), phân số (/). Trả chuỗi rỗng (null) nếu không áp được.
namespace numfmt {

QString apply(const QVariant &value, const QString &code);

// Tăng/giảm số chữ số thập phân của mã định dạng (delta = +1/-1). Giữ tiền tố
// (vd "$#,##0", "#,##0") và hậu tố "%"; rỗng coi như "0". Kẹp 0..10 chữ số. Thuần.
QString adjustDecimals(const QString &code, int delta);

} // namespace numfmt
