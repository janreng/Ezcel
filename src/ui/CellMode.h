// Chỉ báo chế độ ô trên thanh trạng thái (Spec 11/03). Logic thuần, test được.
#pragma once
#include <QString>

namespace cellmode {

// Các chế độ con trỏ ô của bảng tính.
enum class Mode { Ready, Enter, Edit, Point };

// Nhãn hiển thị tiếng Việt cho từng chế độ.
QString label(Mode m);

} // namespace cellmode
