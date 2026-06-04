#pragma once
#include <QString>

// Đổi kiểu chữ hoa/thường cho văn bản (thao tác vùng chọn, Spec 05/30). Logic thuần, test được.
namespace textcase {

enum class Mode { Upper, Lower, Proper };

// Upper: HOA hết. Lower: thường hết. Proper: viết hoa chữ cái đầu mỗi từ.
QString convert(const QString &s, Mode m);

// Cắt gọn khoảng trắng (giống hàm TRIM): bỏ khoảng trắng đầu/cuối và rút nhiều
// khoảng trắng liên tiếp ở giữa thành một dấu cách. Cũng coi tab/xuống dòng là trắng.
QString trimSpaces(const QString &s);

// Bỏ ký tự không in được (giống hàm CLEAN): loại các ký tự điều khiển (mã < 32) và
// các khoảng trắng đặc biệt (non-breaking space U+00A0...). Giữ nguyên nội dung khác.
QString removeNonPrintable(const QString &s);

} // namespace textcase
