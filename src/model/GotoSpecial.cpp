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

} // namespace gotospecial
