#pragma once
#include <QString>
#include <QVariant>

// Áp mã định dạng số (number_format) -> chuỗi hiển thị. Logic thuần, test được.
// Hỗ trợ: ngày/giờ, phần trăm, dấu phẩy hàng nghìn + thập phân, khoa học (E),
// tiền tệ ($), phân số (/). Trả chuỗi rỗng (null) nếu không áp được.
namespace numfmt {

QString apply(const QVariant &value, const QString &code);

} // namespace numfmt
