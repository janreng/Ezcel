#pragma once
#include <QStringList>
#include <QHash>

class QLineEdit;

// Popup gợi ý tên hàm + tooltip cú pháp đối số khi nhập công thức (Spec 12). Gắn vào một
// QLineEdit (thanh công thức hoặc editor trong ô): gõ '=' rồi gõ chữ -> xổ danh sách hàm
// khớp; chọn -> chèn "TÊN(". Khi con trỏ ở trong ngoặc hàm -> hiện tooltip cú pháp tham số.
namespace formulahint {

void install(QLineEdit *edit, const QStringList &functionNames,
             const QHash<QString, QString> &signatures = {});

} // namespace formulahint
