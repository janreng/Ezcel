#include "model/TextCase.h"

namespace textcase {

QString convert(const QString &s, Mode m) {
    if (m == Mode::Upper) return s.toUpper();
    if (m == Mode::Lower) return s.toLower();
    // Proper: viết hoa chữ cái đầu mỗi từ, các chữ còn lại viết thường.
    QString out = s;
    bool startWord = true;
    for (QChar &c : out) {
        if (c.isLetter()) {
            c = startWord ? c.toUpper() : c.toLower();
            startWord = false;
        } else {
            startWord = true; // ký tự không phải chữ -> từ mới bắt đầu sau đó
        }
    }
    return out;
}

QString trimSpaces(const QString &s) {
    return s.simplified(); // Qt: bỏ trắng đầu/cuối + gộp khoảng trắng giữa thành 1 dấu cách
}

QString removeNonPrintable(const QString &s) {
    QString out;
    out.reserve(s.size());
    for (const QChar &c : s) {
        const ushort u = c.unicode();
        if (u < 32) continue;                       // ký tự điều khiển ASCII
        if (u == 0x00A0) { out += QLatin1Char(' '); continue; } // non-breaking space -> dấu cách
        if (u == 0x200B || u == 0xFEFF) continue;   // zero-width space / BOM
        out += c;
    }
    return out;
}

} // namespace textcase
