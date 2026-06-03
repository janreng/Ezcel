// Chỉ báo chế độ ô (Spec 11/03).
#include "ui/CellMode.h"

namespace cellmode {

QString label(Mode m)
{
    switch (m) {
        case Mode::Ready: return QStringLiteral("Sẵn sàng");
        case Mode::Enter: return QStringLiteral("Nhập");
        case Mode::Edit:  return QStringLiteral("Sửa");
        case Mode::Point: return QStringLiteral("Chọn");
    }
    return QStringLiteral("Sẵn sàng");
}

} // namespace cellmode
