#pragma once
#include <QStringList>

class QLineEdit;

// Popup gợi ý tên hàm khi nhập công thức (Spec 12). Gắn vào một QLineEdit (thanh công
// thức hoặc editor trong ô): gõ '=' rồi gõ chữ -> xổ danh sách hàm khớp; chọn -> chèn
// "TÊN(". Logic token thuần (chỉ lấy chữ cái cuối tại con trỏ).
namespace formulahint {

void install(QLineEdit *edit, const QStringList &functionNames);

} // namespace formulahint
