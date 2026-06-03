#pragma once
#include <QString>
#include <QVariant>
#include <functional>

// Engine công thức — bản port của formula.py (P2, perf core).
// Hiện là khung API; tokenizer/parser/registry hàm sẽ được port đầy đủ ở P2.
namespace formula {

// resolver(row, col) -> giá trị ô (để công thức tham chiếu ô khác).
using Resolver = std::function<QVariant(int row, int col)>;

// Tính một công thức (chuỗi bắt đầu bằng '='). Trả lỗi dạng "#ERROR" nếu sai.
QVariant evaluate(const QString &formula, const Resolver &resolver);

// Dịch tham chiếu tương đối khi kéo-điền/paste; tham chiếu '$' giữ nguyên.
QString offsetFormula(const QString &text, int drow, int dcol);

} // namespace formula
