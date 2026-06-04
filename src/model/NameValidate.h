#pragma once
#include <QString>

// Kiểm tra tên vùng (named range) hợp lệ kiểu Excel (Spec 04/31). Logic thuần, test được.
namespace namevalidate {

// Quy tắc:
//  - Độ dài 1..255.
//  - Ký tự đầu: chữ cái, '_' hoặc '\'.
//  - Ký tự sau: chữ, số, '.', '_'.
//  - KHÔNG được trùng dạng địa chỉ ô (A1, $B$2) hay kiểu R1C1.
//  - Một mình chữ "C"/"R" (không phân biệt hoa/thường) bị cấm (reserved).
//  - Không chứa khoảng trắng.
bool isValid(const QString &name);

// Lý do không hợp lệ (chuỗi tiếng Việt ngắn) — rỗng nếu hợp lệ. Dùng để báo người dùng.
QString reason(const QString &name);

} // namespace namevalidate
