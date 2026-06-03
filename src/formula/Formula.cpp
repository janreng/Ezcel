#include "formula/Formula.h"

// KHUNG TẠM (P0). Bản port đầy đủ của formula.py làm ở P2:
//   tokenizer regex -> _Parser (recursive-descent) -> registry _FUNCTIONS.
// Hiện chỉ trả lại chính chuỗi để skeleton build/chạy được.
namespace formula {

QVariant evaluate(const QString &formula, const Resolver &resolver)
{
    Q_UNUSED(resolver);
    return formula; // TODO(P2): tính thật
}

QString offsetFormula(const QString &text, int drow, int dcol)
{
    Q_UNUSED(drow);
    Q_UNUSED(dcol);
    return text; // TODO(P2): dịch tham chiếu tương đối
}

} // namespace formula
