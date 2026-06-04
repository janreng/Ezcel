#include "formula/Formula.h"
#include "formula/Functions.h"

#include <QRegularExpression>
#include <cmath>
#include <algorithm>

namespace formula {

// ---------------------------------------------------------------- tiện ích ô

int colLettersToIndex(const QString &letters) {
    int result = 0;
    for (QChar ch : letters) {
        ch = ch.toUpper();
        result = result * 26 + (ch.unicode() - 'A' + 1);
    }
    return result - 1;
}

QString colIndexToLetters(int index) {
    QString result;
    int n = index + 1;
    while (n > 0) {
        int rem = (n - 1) % 26;
        result.prepend(QChar('A' + rem));
        n = (n - 1) / 26;
    }
    return result;
}

static const QRegularExpression kCellRe(QStringLiteral("^\\$?([A-Za-z]+)\\$?(\\d+)$"));

// Tách "Sheet1!A1" hoặc "'Tên Sheet'!A1" -> (tên sheet, "A1"). Bỏ dấu nháy đơn bao tên.
static QPair<QString,QString> splitSheetCell(const QString &tok) {
    const int bang = tok.lastIndexOf(QLatin1Char('!'));
    QString sheet = tok.left(bang);
    const QString cell = tok.mid(bang + 1);
    if (sheet.size() >= 2 && sheet.startsWith(QLatin1Char('\'')) && sheet.endsWith(QLatin1Char('\'')))
        sheet = sheet.mid(1, sheet.size() - 2);
    return {sheet, cell};
}

// 'B3' -> (row=2, col=1). Ném nếu sai cú pháp.
static QPair<int,int> parseCellRef(const QString &ref) {
    auto m = kCellRe.match(ref);
    if (!m.hasMatch())
        throw FormulaError(QStringLiteral("Tham chiếu ô không hợp lệ: %1").arg(ref));
    int col = colLettersToIndex(m.captured(1));
    int row = m.captured(2).toInt() - 1;
    if (row < 0)
        throw FormulaError(QStringLiteral("Tham chiếu ô không hợp lệ: %1").arg(ref));
    return {row, col};
}

// ---------------------------------------------------------------- ép kiểu

double toNumber(const Value &v) {
    switch (v.type) {
    case Type::Bool:   return v.boolean ? 1.0 : 0.0;
    case Type::Number:
        if (!std::isfinite(v.num)) throw FormulaError(QStringLiteral("Số không hữu hạn"), ERR_NUM);
        return v.num;
    case Type::Empty:  return 0.0;
    case Type::Text: {
        if (v.text.isEmpty()) return 0.0;
        bool ok = false;
        double n = v.text.trimmed().toDouble(&ok);
        if (!ok || !std::isfinite(n))
            throw FormulaError(QStringLiteral("Không phải số: %1").arg(v.text));
        return n;
    }
    case Type::Range: {
        auto f = v.range.flat();
        if (f.size() == 1) return toNumber(f.front());
        throw FormulaError(QStringLiteral("Không phải số (vùng)"));
    }
    }
    return 0.0;
}

int toInt(const Value &v) { return int(toNumber(v)); }

// Định dạng số kiểu Python str(float): số nguyên không có '.0'.
static QString numberToText(double n) {
    if (std::floor(n) == n && std::abs(n) < 1e15)
        return QString::number(static_cast<long long>(n));
    QString s = QString::number(n, 'g', 15);
    return s;
}

QString toText(const Value &v) {
    switch (v.type) {
    case Type::Empty:  return QString();
    case Type::Bool:   return v.boolean ? QStringLiteral("TRUE") : QStringLiteral("FALSE");
    case Type::Number: return numberToText(v.num);
    case Type::Text:   return v.text;
    case Type::Range: {
        auto f = v.range.flat();
        return f.empty() ? QString() : toText(f.front());
    }
    }
    return QString();
}

bool toBool(const Value &v) {
    switch (v.type) {
    case Type::Bool:   return v.boolean;
    case Type::Number: return v.num != 0.0;
    case Type::Empty:  return false;
    case Type::Text: {
        QString s = v.text.trimmed().toUpper();
        if (s == QLatin1String("TRUE")) return true;
        if (s == QLatin1String("FALSE") || s.isEmpty()) return false;
        bool ok = false;
        double n = v.text.toDouble(&ok);
        return ok ? (n != 0.0) : true;
    }
    case Type::Range: {
        auto f = v.range.flat();
        return f.empty() ? false : toBool(f.front());
    }
    }
    return false;
}

int cmp(const Value &a, const Value &b) {
    try {
        double na = toNumber(a), nb = toNumber(b);
        return (na > nb) - (na < nb);
    } catch (const FormulaError &) {
        QString sa = toText(a).toLower(), sb = toText(b).toLower();
        return (sa > sb) - (sa < sb);
    }
}

bool looseEqual(const Value &a, const Value &b) {
    try {
        return toNumber(a) == toNumber(b);
    } catch (const FormulaError &) {
        return toText(a).toLower() == toText(b).toLower();
    }
}

std::vector<Value> flatten(const std::vector<Value> &args) {
    std::vector<Value> out;
    for (const auto &a : args) {
        if (a.isRange()) {
            auto f = a.range.flat();
            out.insert(out.end(), f.begin(), f.end());
        } else {
            out.push_back(a);
        }
    }
    return out;
}

std::vector<double> numbers(const std::vector<Value> &args) {
    std::vector<double> out;
    for (const auto &a : flatten(args)) {
        if (a.type == Type::Empty) continue;
        if (a.type == Type::Text && a.text.isEmpty()) continue;
        try {
            out.push_back(toNumber(a));
        } catch (const FormulaError &) {
            continue; // bỏ qua giá trị không phải số
        }
    }
    return out;
}

// ---------------------------------------------------------------- tokenizer

namespace {

enum class Tk { Number, String, SheetCell, Cell, Ident, Op, Ws };
struct Token { Tk kind; QString value; };

QString unescape(const QString &literal) {
    QString s = literal.mid(1, literal.size() - 2);
    s.replace(QStringLiteral("\\\""), QStringLiteral("\""));
    s.replace(QStringLiteral("\\\\"), QStringLiteral("\\"));
    return s;
}

const QRegularExpression &tokenRe() {
    static const QRegularExpression re(QStringLiteral(
        "(?<NUMBER>\\d+\\.\\d+|\\d+|\\.\\d+)"
        "|(?<STRING>\"(?:[^\"\\\\]|\\\\.)*\")"
        "|(?<SHEETCELL>(?:'[^']*'|[A-Za-z_][A-Za-z0-9_]*)!\\$?[A-Za-z]+\\$?\\d+(?![A-Za-z0-9_])(?!\\s*\\())"
        "|(?<CELL>\\$?[A-Za-z]+\\$?\\d+(?![A-Za-z0-9_])(?!\\s*\\())"
        "|(?<IDENT>[A-Za-z_][A-Za-z0-9_]*)"
        "|(?<OP>>=|<=|<>|[+\\-*/^(),:<>=])"
        "|(?<WS>\\s+)"));
    return re;
}

std::vector<Token> tokenize(const QString &text) {
    std::vector<Token> tokens;
    int pos = 0;
    const QRegularExpression &re = tokenRe();
    while (pos < text.size()) {
        auto m = re.match(text, pos, QRegularExpression::NormalMatch,
                          QRegularExpression::AnchorAtOffsetMatchOption);
        if (!m.hasMatch() || m.capturedStart() != pos)
            throw FormulaError(QStringLiteral("Ký tự không hợp lệ tại: %1").arg(text.mid(pos)));
        pos = m.capturedEnd();
        static const struct { const char *name; Tk kind; } groups[] = {
            {"NUMBER", Tk::Number}, {"STRING", Tk::String}, {"SHEETCELL", Tk::SheetCell},
            {"CELL", Tk::Cell}, {"IDENT", Tk::Ident}, {"OP", Tk::Op}, {"WS", Tk::Ws},
        };
        for (const auto &g : groups) {
            QString cap = m.captured(QString::fromLatin1(g.name));
            if (!cap.isNull()) {
                if (g.kind != Tk::Ws) tokens.push_back({g.kind, cap});
                break;
            }
        }
    }
    return tokens;
}

// So sánh hai giá trị cho toán tử (port _compare): số nếu được, else chuỗi (phân biệt hoa thường).
Value compareOp(const Value &left, const QString &op, const Value &right) {
    double a, b; QString sa, sb; bool numeric = true;
    try { a = toNumber(left); b = toNumber(right); }
    catch (const FormulaError &) { numeric = false; sa = toText(left); sb = toText(right); }
    bool r;
    if (op == QLatin1String("="))  r = numeric ? (a == b) : (sa == sb);
    else if (op == QLatin1String("<>")) r = numeric ? (a != b) : (sa != sb);
    else if (op == QLatin1String("<"))  r = numeric ? (a < b)  : (sa < sb);
    else if (op == QLatin1String(">"))  r = numeric ? (a > b)  : (sa > sb);
    else if (op == QLatin1String("<=")) r = numeric ? (a <= b) : (sa <= sb);
    else if (op == QLatin1String(">=")) r = numeric ? (a >= b) : (sa >= sb);
    else throw FormulaError(QStringLiteral("Toán tử so sánh lạ: %1").arg(op));
    return Value::boolv(r);
}

// ---------------------------------------------------------------- parser

class Parser {
public:
    Parser(std::vector<Token> tokens, const Resolver &resolver, const SheetResolver &sheetResolver = {})
        : m_tokens(std::move(tokens)), m_resolver(resolver), m_sheetResolver(sheetResolver) {}

    Value parse() {
        Value v = comparison();
        if (peek()) throw FormulaError(QStringLiteral("Token thừa: %1").arg(peek()->value));
        return v;
    }

private:
    const Token *peek() const { return m_pos < int(m_tokens.size()) ? &m_tokens[m_pos] : nullptr; }
    const Token &next() {
        const Token *t = peek();
        if (!t) throw FormulaError(QStringLiteral("Công thức kết thúc đột ngột"));
        ++m_pos; return m_tokens[m_pos - 1];
    }
    void expectOp(const QString &op) {
        const Token *t = peek();
        if (!t || t->kind != Tk::Op || t->value != op)
            throw FormulaError(QStringLiteral("Thiếu '%1'").arg(op));
        ++m_pos;
    }
    bool isOp(const Token *t, const QString &v) const { return t && t->kind == Tk::Op && t->value == v; }

    Value comparison() {
        Value v = expr();
        const Token *t = peek();
        if (t && t->kind == Tk::Op &&
            (t->value == "=" || t->value == "<>" || t->value == "<" ||
             t->value == ">" || t->value == "<=" || t->value == ">=")) {
            QString op = t->value; ++m_pos;
            return compareOp(v, op, expr());
        }
        return v;
    }
    Value expr() {
        Value v = term();
        while (true) {
            const Token *t = peek();
            if (t && t->kind == Tk::Op && (t->value == "+" || t->value == "-")) {
                ++m_pos;
                Value rhs = term();
                v = Value::number(t->value == "+" ? toNumber(v) + toNumber(rhs)
                                                   : toNumber(v) - toNumber(rhs));
            } else return v;
        }
    }
    Value term() {
        Value v = power();
        while (true) {
            const Token *t = peek();
            if (t && t->kind == Tk::Op && (t->value == "*" || t->value == "/")) {
                QString op = t->value; ++m_pos;
                Value rhs = power();
                if (op == "*") v = Value::number(toNumber(v) * toNumber(rhs));
                else {
                    double d = toNumber(rhs);
                    if (d == 0) throw FormulaError(QStringLiteral("Chia cho 0"), ERR_DIV0);
                    v = Value::number(toNumber(v) / d);
                }
            } else return v;
        }
    }
    Value power() {
        Value v = unary();
        const Token *t = peek();
        if (t && t->kind == Tk::Op && t->value == "^") {
            ++m_pos;
            return Value::number(std::pow(toNumber(v), toNumber(power()))); // phải-kết hợp
        }
        return v;
    }
    Value unary() {
        const Token *t = peek();
        if (t && t->kind == Tk::Op && (t->value == "+" || t->value == "-")) {
            QString op = t->value; ++m_pos;
            double val = toNumber(unary());
            return Value::number(op == "-" ? -val : val);
        }
        return primary();
    }
    Value primary() {
        const Token &t = next();
        switch (t.kind) {
        case Tk::Number: return Value::number(t.value.toDouble());
        case Tk::String: return Value::str(unescape(t.value));
        case Tk::Cell: {
            auto [row, col] = parseCellRef(t.value);
            return Value::fromCell(m_resolver(row, col));
        }
        case Tk::SheetCell: {
            if (!m_sheetResolver) throw FormulaError(QStringLiteral("Tham chiếu chéo sheet chưa hỗ trợ"), ERR_REF);
            auto [sheet, cellref] = splitSheetCell(t.value);
            auto [row, col] = parseCellRef(cellref);
            return Value::fromCell(m_sheetResolver(sheet, row, col));
        }
        case Tk::Ident: {
            // TRUE/FALSE viết trần (không ngoặc) -> literal boolean (kiểu Excel).
            if (!isOp(peek(), QStringLiteral("("))) {
                QString up = t.value.toUpper();
                if (up == QLatin1String("TRUE")) return Value::boolv(true);
                if (up == QLatin1String("FALSE")) return Value::boolv(false);
                throw FormulaError(QStringLiteral("Tên không xác định: %1").arg(t.value), ERR_NAME);
            }
            return functionCall(t.value);
        }
        case Tk::Op:
            if (t.value == "(") {
                Value v = comparison();
                expectOp(QStringLiteral(")"));
                return v;
            }
            break;
        default: break;
        }
        throw FormulaError(QStringLiteral("Không mong đợi: %1").arg(t.value));
    }

    Value functionCall(const QString &name) {
        QString fname = name.toUpper();
        expectOp(QStringLiteral("("));
        if (const LazyFn *lf = lookupLazy(fname)) {
            std::vector<Thunk> thunks = lazyArgs();
            expectOp(QStringLiteral(")"));
            return (*lf)(thunks);
        }
        const Fn *fn = lookupFunction(fname);
        if (!fn) throw FormulaError(QStringLiteral("Hàm không hỗ trợ: %1").arg(name), ERR_NAME);
        std::vector<Value> args;
        const Token *p = peek();
        if (!isOp(p, QStringLiteral(")"))) {
            arg(args);
            while (isOp(peek(), QStringLiteral(","))) { ++m_pos; arg(args); }
        }
        expectOp(QStringLiteral(")"));
        return (*fn)(args);
    }

    // Một đối số: vùng A1:B3 (mở rộng) hoặc một biểu thức.
    void arg(std::vector<Value> &out) {
        const Token *t = peek();
        const Token *nxt = (m_pos + 1 < int(m_tokens.size())) ? &m_tokens[m_pos + 1] : nullptr;
        if (t && t->kind == Tk::Cell && isOp(nxt, QStringLiteral(":"))) {
            QString start = t->value;
            m_pos += 2;
            const Token &endTok = next();
            if (endTok.kind != Tk::Cell) throw FormulaError(QStringLiteral("Vùng không hợp lệ"));
            out.push_back(expandRange(start, endTok.value));
            return;
        }
        // Vùng chéo sheet: Sheet1!A1:B3  (token SheetCell, ':', Cell).
        if (t && t->kind == Tk::SheetCell && isOp(nxt, QStringLiteral(":"))) {
            if (!m_sheetResolver) throw FormulaError(QStringLiteral("Tham chiếu chéo sheet chưa hỗ trợ"), ERR_REF);
            auto [sheet, startCell] = splitSheetCell(t->value);
            m_pos += 2;
            const Token &endTok = next();
            if (endTok.kind != Tk::Cell) throw FormulaError(QStringLiteral("Vùng không hợp lệ"));
            out.push_back(expandRangeSheet(sheet, startCell, endTok.value));
            return;
        }
        out.push_back(comparison());
    }

    Value expandRange(const QString &start, const QString &end) {
        auto [r1, c1] = parseCellRef(start);
        auto [r2, c2] = parseCellRef(end);
        if (r1 > r2) std::swap(r1, r2);
        if (c1 > c2) std::swap(c1, c2);
        Range rg;
        rg.rows = std::make_shared<std::vector<std::vector<Value>>>();
        rg.rows->reserve(r2 - r1 + 1);
        for (int r = r1; r <= r2; ++r) {
            std::vector<Value> row;
            row.reserve(c2 - c1 + 1);
            for (int c = c1; c <= c2; ++c)
                row.push_back(Value::fromCell(m_resolver(r, c)));
            rg.rows->push_back(std::move(row));
        }
        return Value::rangev(std::move(rg));
    }

    // Vùng đọc từ sheet khác (Sheet1!A1:B3).
    Value expandRangeSheet(const QString &sheet, const QString &start, const QString &end) {
        auto [r1, c1] = parseCellRef(start);
        auto [r2, c2] = parseCellRef(end);
        if (r1 > r2) std::swap(r1, r2);
        if (c1 > c2) std::swap(c1, c2);
        Range rg;
        rg.rows = std::make_shared<std::vector<std::vector<Value>>>();
        rg.rows->reserve(r2 - r1 + 1);
        for (int r = r1; r <= r2; ++r) {
            std::vector<Value> row;
            row.reserve(c2 - c1 + 1);
            for (int c = c1; c <= c2; ++c)
                row.push_back(Value::fromCell(m_sheetResolver(sheet, r, c)));
            rg.rows->push_back(std::move(row));
        }
        return Value::rangev(std::move(rg));
    }

    // --- đối số tính lười (IF/IFERROR/IFS/ISERROR/SWITCH...) ---
    std::vector<Thunk> lazyArgs() {
        std::vector<Thunk> thunks;
        if (isOp(peek(), QStringLiteral(")"))) return thunks;
        while (true) {
            std::vector<Token> toks = collectOneArg();
            Resolver res = m_resolver;
            thunks.push_back([toks, res]() -> Value {
                if (toks.empty()) return Value::str(QString());
                return Parser(toks, res).parse();
            });
            if (isOp(peek(), QStringLiteral(","))) { ++m_pos; continue; }
            break;
        }
        return thunks;
    }
    std::vector<Token> collectOneArg() {
        int depth = 0;
        std::vector<Token> toks;
        while (true) {
            const Token *t = peek();
            if (!t) break;
            if (isOp(t, QStringLiteral("("))) depth++;
            else if (isOp(t, QStringLiteral(")"))) { if (depth == 0) break; depth--; }
            else if (isOp(t, QStringLiteral(",")) && depth == 0) break;
            toks.push_back(*t);
            ++m_pos;
        }
        return toks;
    }

    std::vector<Token> m_tokens;
    Resolver m_resolver;
    SheetResolver m_sheetResolver;
    int m_pos = 0;
};

Value finalize(Value v) {
    if (v.isRange()) {
        auto f = v.range.flat();
        if (f.size() == 1) v = f.front();
        else throw FormulaError(QStringLiteral("Kết quả là vùng nhiều ô (chưa hỗ trợ spill)"), ERR_VALUE);
    }
    if (v.type == Type::Number && !std::isfinite(v.num))
        throw FormulaError(QStringLiteral("Kết quả không hữu hạn"), ERR_NUM);
    return v;
}

} // anonymous namespace

// ---------------------------------------------------------------- API công khai

bool isFormula(const QString &text) {
    return text.startsWith(QLatin1Char('=')) && text.size() > 1;
}

QVariant evaluate(const QString &formula, const Resolver &resolver,
                  const SheetResolver &sheetResolver) {
    QString body = formula.startsWith(QLatin1Char('=')) ? formula.mid(1) : formula;
    std::vector<Token> tokens = tokenize(body);
    if (tokens.empty()) throw FormulaError(QStringLiteral("Công thức rỗng"));
    Value v = finalize(Parser(std::move(tokens), resolver, sheetResolver).parse());
    return v.toVariant();
}

Value evaluateValue(const QString &formula, const Resolver &resolver,
                    const SheetResolver &sheetResolver) {
    QString body = formula.startsWith(QLatin1Char('=')) ? formula.mid(1) : formula;
    std::vector<Token> tokens = tokenize(body);
    if (tokens.empty()) throw FormulaError(QStringLiteral("Công thức rỗng"));
    Value v = Parser(std::move(tokens), resolver, sheetResolver).parse();
    if (v.isRange()) {
        auto f = v.range.flat();
        if (f.size() == 1) v = f.front(); // vùng 1 ô -> vô hướng
        else return v;                     // GIỮ vùng nhiều ô cho spill
    }
    if (v.type == Type::Number && !std::isfinite(v.num))
        throw FormulaError(QStringLiteral("Kết quả không hữu hạn"), ERR_NUM);
    return v;
}

// offset một tham chiếu ô theo (drow,dcol); giữ phần có '$'.
static const QRegularExpression kOffsetCellRe(QStringLiteral("^(\\$?)([A-Za-z]+)(\\$?)(\\d+)$"));

static QString offsetCell(const QString &ref, int drow, int dcol) {
    auto m = kOffsetCellRe.match(ref);
    if (!m.hasMatch()) return ref;
    QString colAbs = m.captured(1), rowAbs = m.captured(3);
    int col = colLettersToIndex(m.captured(2));
    int row = m.captured(4).toInt() - 1;
    if (colAbs.isEmpty()) col = std::max(0, col + dcol);
    if (rowAbs.isEmpty()) row = std::max(0, row + drow);
    return colAbs + colIndexToLetters(col) + rowAbs + QString::number(row + 1);
}

QString offsetFormula(const QString &text, int drow, int dcol) {
    QString body = text.startsWith(QLatin1Char('=')) ? text.mid(1) : text;
    std::vector<Token> tokens;
    try { tokens = tokenize(body); }
    catch (const FormulaError &) { return text; }
    QString out;
    for (const Token &t : tokens) {
        if (t.kind == Tk::Cell) out += offsetCell(t.value, drow, dcol);
        else if (t.kind == Tk::SheetCell) {
            int bang = t.value.indexOf(QLatin1Char('!'));
            out += t.value.left(bang + 1) + offsetCell(t.value.mid(bang + 1), drow, dcol);
        } else out += t.value;
    }
    return QStringLiteral("=") + out;
}

QSet<QPair<int,int>> extractRefs(const QString &formula) {
    QString body = formula.startsWith(QLatin1Char('=')) ? formula.mid(1) : formula;
    std::vector<Token> tokens;
    try { tokens = tokenize(body); }
    catch (const FormulaError &) { return {}; }
    QSet<QPair<int,int>> refs;
    int i = 0, n = int(tokens.size());
    while (i < n) {
        const Token &t = tokens[i];
        if (t.kind == Tk::Cell) {
            if (i + 2 < n && tokens[i+1].kind == Tk::Op && tokens[i+1].value == ":"
                && tokens[i+2].kind == Tk::Cell) {
                try {
                    auto [r1, c1] = parseCellRef(t.value);
                    auto [r2, c2] = parseCellRef(tokens[i+2].value);
                    if (r1 > r2) std::swap(r1, r2);
                    if (c1 > c2) std::swap(c1, c2);
                    for (int r = r1; r <= r2; ++r)
                        for (int c = c1; c <= c2; ++c)
                            refs.insert({r, c});
                } catch (const FormulaError &) {}
                i += 3;
            } else {
                try { refs.insert(parseCellRef(t.value)); } catch (const FormulaError &) {}
                i += 1;
            }
        } else i += 1;
    }
    return refs;
}

} // namespace formula
