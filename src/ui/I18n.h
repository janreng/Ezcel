#pragma once
#include <QString>

// Dịch giao diện Việt/Anh. Tra bảng theo khóa; khóa không có -> trả chính khóa.
// Logic thuần (test headless được). Dịch dần các phần UI.
namespace i18n {

enum class Lang { Vi, En };

void setLang(Lang l);
Lang lang();
QString tr(const QString &key); // bản dịch theo ngôn ngữ hiện tại

} // namespace i18n
