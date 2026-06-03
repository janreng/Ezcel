#include "model/GotoSpecial.h"

namespace gotospecial {

static bool isFormula(const QString &s) { return s.startsWith(QLatin1Char('=')) && s.size() > 1; }
static bool isNumber(const QString &s) {
    if (s.isEmpty() || isFormula(s)) return false;
    bool ok = false; s.toDouble(&ok); return ok;
}

static bool matches(const QString &s, Kind kind) {
    switch (kind) {
    case Kind::Blanks:    return s.isEmpty();
    case Kind::Formulas:  return isFormula(s);
    case Kind::Numbers:   return isNumber(s);
    case Kind::Text:      return !s.isEmpty() && !isFormula(s) && !isNumber(s);
    case Kind::Constants: return !s.isEmpty() && !isFormula(s); // số hoặc chữ, không phải công thức
    }
    return false;
}

QVector<QPair<int, int>> find(const QVector<QVector<QString>> &grid, Kind kind) {
    QVector<QPair<int, int>> out;
    for (int r = 0; r < grid.size(); ++r)
        for (int c = 0; c < grid[r].size(); ++c)
            if (matches(grid[r][c], kind)) out.push_back({r, c});
    return out;
}

namespace {
// Ô (r,c) trong lưới có dữ liệu không (an toàn ngoài biên -> rỗng).
bool nonEmpty(const QVector<QVector<QString>> &grid, int r, int c) {
    if (r < 0 || r >= grid.size() || c < 0 || c >= grid[r].size()) return false;
    return !grid[r][c].isEmpty();
}
} // namespace

Region currentRegion(const QVector<QVector<QString>> &grid, int row, int col) {
    Region reg{row, col, row, col};
    const int rows = grid.size();
    bool changed = true;
    while (changed) {
        changed = false;
        if (reg.top > 0)
            for (int c = reg.left; c <= reg.right; ++c)
                if (nonEmpty(grid, reg.top - 1, c)) { --reg.top; changed = true; break; }
        if (reg.bottom < rows - 1)
            for (int c = reg.left; c <= reg.right; ++c)
                if (nonEmpty(grid, reg.bottom + 1, c)) { ++reg.bottom; changed = true; break; }
        if (reg.left > 0)
            for (int r = reg.top; r <= reg.bottom; ++r)
                if (nonEmpty(grid, r, reg.left - 1)) { --reg.left; changed = true; break; }
        for (int r = reg.top; r <= reg.bottom; ++r)
            if (nonEmpty(grid, r, reg.right + 1)) { ++reg.right; changed = true; break; }
    }
    return reg;
}

QPair<int, int> lastCell(const QVector<QVector<QString>> &grid) {
    int maxRow = -1, maxCol = -1;
    for (int r = 0; r < grid.size(); ++r)
        for (int c = 0; c < grid[r].size(); ++c)
            if (!grid[r][c].isEmpty()) { if (r > maxRow) maxRow = r; if (c > maxCol) maxCol = c; }
    return {maxRow, maxCol};
}

} // namespace gotospecial
