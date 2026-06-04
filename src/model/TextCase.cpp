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

} // namespace textcase
