#pragma once
#include "formula/Value.h"
#include <QString>
#include <QVariant>
#include <QSet>
#include <QPair>
#include <functional>
#include <stdexcept>

// Engine công thức — bản port của formula.py (P2, perf core).
namespace formula {

// Mã lỗi chuẩn Excel.
inline constexpr const char *ERR_DIV0  = "#DIV/0!";
inline constexpr const char *ERR_NA    = "#N/A";
inline constexpr const char *ERR_VALUE = "#VALUE!";
inline constexpr const char *ERR_NUM   = "#NUM!";
inline constexpr const char *ERR_NAME  = "#NAME?";
inline constexpr const char *ERR_REF   = "#REF!";
inline constexpr const char *ERR_SPILL = "#SPILL!";

// Lỗi khi phân tích/tính công thức; etype mang mã lỗi kiểu Excel.
class FormulaError : public std::exception {
public:
    explicit FormulaError(QString message = {}, QString etype = QStringLiteral("#VALUE!"))
        : m_msg(std::move(message)), m_etype(std::move(etype)) {}
    const char *what() const noexcept override { return "FormulaError"; }
    const QString &etype() const { return m_etype; }
    const QString &message() const { return m_msg; }
private:
    QString m_msg;
    QString m_etype;
};

// resolver(row, col) -> giá trị ĐÃ TÍNH của ô (số/chuỗi). Một sheet/file.
using Resolver = std::function<QVariant(int row, int col)>;

// resolver cho ô của SHEET KHÁC theo tên (tham chiếu chéo sheet, vd Sheet1!A1).
// Trả QVariant rỗng nếu không tìm thấy sheet. Bỏ trống nếu không hỗ trợ.
using SheetResolver = std::function<QVariant(const QString &sheet, int row, int col)>;

// --- API công khai (tương đương formula.py) ---

// Công thức là chuỗi bắt đầu '=' và dài > 1.
bool isFormula(const QString &text);

// Tính một công thức (đã có '='). Trả số/chuỗi/bool dưới dạng QVariant.
// Ném FormulaError nếu sai (caller hiển thị etype).
QVariant evaluate(const QString &formula, const Resolver &resolver,
                  const SheetResolver &sheetResolver = {});

// Như evaluate() nhưng GIỮ kết quả vùng (ma trận nhiều ô) thay vì gộp/ném lỗi — phục vụ
// spill (mảng động). Vùng 1 ô vẫn rút về vô hướng. Ném FormulaError nếu sai.
Value evaluateValue(const QString &formula, const Resolver &resolver,
                    const SheetResolver &sheetResolver = {});

// Dịch tham chiếu tương đối khi kéo-điền/paste; '$' giữ nguyên.
QString offsetFormula(const QString &text, int drow, int dcol);

// Tập (row,col) mà công thức tham chiếu (cho đồ thị phụ thuộc ở P1).
QSet<QPair<int,int>> extractRefs(const QString &formula);

// --- tiện ích nội bộ (dùng chung giữa Formula.cpp và Functions.cpp) ---
double toNumber(const Value &v);
int    toInt(const Value &v);
QString toText(const Value &v);
bool   toBool(const Value &v);
int    cmp(const Value &a, const Value &b);            // -1/0/1
bool   looseEqual(const Value &a, const Value &b);
std::vector<Value> flatten(const std::vector<Value> &args); // trải Range
std::vector<double> numbers(const std::vector<Value> &args); // số, bỏ rỗng/không-số

int colLettersToIndex(const QString &letters);
QString colIndexToLetters(int index);

} // namespace formula
