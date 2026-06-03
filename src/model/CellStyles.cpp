#include "model/CellStyles.h"

namespace cellstyles {

QStringList names() {
    return {QStringLiteral("Bình thường"), QStringLiteral("Tốt"), QStringLiteral("Xấu"),
            QStringLiteral("Trung tính"), QStringLiteral("Tiêu đề"), QStringLiteral("Tựa đề")};
}

QHash<QString, QVariant> style(const QString &name) {
    QHash<QString, QVariant> f;
    if (name == QStringLiteral("Tốt")) {
        f["bg"] = "#C6EFCE"; f["color"] = "#006100";
    } else if (name == QStringLiteral("Xấu")) {
        f["bg"] = "#FFC7CE"; f["color"] = "#9C0006";
    } else if (name == QStringLiteral("Trung tính")) {
        f["bg"] = "#FFEB9C"; f["color"] = "#9C6500";
    } else if (name == QStringLiteral("Tiêu đề")) {
        f["bold"] = true; f["size"] = 13; f["color"] = "#44546A";
    } else if (name == QStringLiteral("Tựa đề")) {
        f["bold"] = true; f["size"] = 18; f["color"] = "#44546A";
    } else if (name == QStringLiteral("Bình thường")) {
        // Xóa định dạng: đặt các key = null.
        for (const char *k : {"bold", "italic", "underline", "strike", "bg", "color", "size"})
            f[QString::fromLatin1(k)] = QVariant();
    }
    return f;
}

} // namespace cellstyles
