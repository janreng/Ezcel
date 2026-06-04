#pragma once
#include <QString>

// Đổi kiểu chữ hoa/thường cho văn bản (thao tác vùng chọn, Spec 05/30). Logic thuần, test được.
namespace textcase {

enum class Mode { Upper, Lower, Proper };

// Upper: HOA hết. Lower: thường hết. Proper: viết hoa chữ cái đầu mỗi từ.
QString convert(const QString &s, Mode m);

} // namespace textcase
