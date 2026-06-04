#include "model/SpreadsheetModel.h"
#include "formula/Formula.h"
#include "model/TextSearch.h"
#include "model/Sort.h"
#include "model/Validation.h"
#include "model/NumberFormat.h"
#include "model/SeriesFill.h"

#include <QColor>
#include <QDate>
#include <QDateTime>
#include <QStringList>
#include <QRegularExpression>
#include <cmath>
#include <climits>
#include <optional>

// ---------------------------------------------------------------- helpers
namespace {

const QHash<QString, Qt::Alignment> kHAlign = {
    {"left", Qt::AlignLeft}, {"center", Qt::AlignHCenter}, {"right", Qt::AlignRight},
};
const QHash<QString, Qt::Alignment> kVAlign = {
    {"top", Qt::AlignTop}, {"middle", Qt::AlignVCenter}, {"bottom", Qt::AlignBottom},
};

// Thuộc tính định dạng hợp lệ (port _FORMAT_KEYS).
const QSet<QString> kFormatKeys = {
    "font", "size", "bold", "italic", "underline", "strike",
    "halign", "valign", "wrap", "bg", "color", "border", "number_format",
    "locked", "hidden",
};

// Định dạng số/giá trị thành chuỗi hiển thị (port _format).
QString formatValue(const QVariant &v) {
    if (!v.isValid() || v.isNull()) return QString();
    if (v.typeId() == QMetaType::Bool) return v.toBool() ? QStringLiteral("TRUE") : QStringLiteral("FALSE");
    if (v.typeId() == QMetaType::Double) {
        double d = v.toDouble();
        if (std::floor(d) == d && std::abs(d) < 1e15) return QString::number((long long)d);
        return QString::number(d, 'g', 6);
    }
    return v.toString();
}

// Ép chuỗi thô thành số nếu được (port _coerce_literal đơn giản).
QVariant coerceLiteral(const QString &raw) {
    if (raw.isEmpty()) return QString();
    bool ok = false;
    double d = raw.toDouble(&ok);
    if (ok) return d;
    return raw;
}

// Số -> chuỗi gọn (nguyên không có '.0').
QString numToText(double v) {
    if (std::floor(v) == v && std::abs(v) < 1e15) return QString::number((long long)v);
    return QString::number(v, 'g', 15);
}

// Chuỗi không phải công thức -> số, hoặc nullopt.
std::optional<double> parseNum(const QString &t) {
    if (t.startsWith('=')) return std::nullopt;
    bool ok = false; double d = t.toDouble(&ok);
    return ok ? std::optional<double>(d) : std::nullopt;
}

// Nếu các ô là số tạo cấp số cộng: trả (đầu, bước). 1 ô -> bước 0. Port _as_series.
std::optional<std::pair<double,double>> asSeries(const QStringList &src) {
    QList<double> nums;
    for (const QString &c : src) { auto n = parseNum(c); if (!n) return std::nullopt; nums << *n; }
    if (nums.isEmpty()) return std::nullopt;
    if (nums.size() == 1) return std::make_pair(nums[0], 0.0);
    double step = nums[1] - nums[0];
    for (int i = 1; i < nums.size(); ++i)
        if (std::abs((nums[i] - nums[i-1]) - step) > 1e-9) return std::nullopt;
    return std::make_pair(nums[0], step);
}

// 'Item1' + pos -> 'Item{1+pos}'. nullopt nếu không có phần chữ ở đầu. Port _increment_trailing_number.
std::optional<QString> incrementTrailing(const QString &text, int pos) {
    static const QRegularExpression re(QStringLiteral("^(.*?)(\\d+)$"));
    auto m = re.match(text);
    if (!m.hasMatch() || m.captured(1).isEmpty()) return std::nullopt;
    long long n = m.captured(2).toLongLong() + pos;
    if (n < 0) n = 0;
    return m.captured(1) + QString::number(n);
}

// Áp number_format -> chuỗi (chuyển sang module numfmt, test riêng được).
QString applyNumberFormat(const QVariant &v, const QString &code) {
    return numfmt::apply(v, code);
}

} // namespace

// ---------------------------------------------------------------- ctor / kích thước
SpreadsheetModel::SpreadsheetModel(QObject *parent) : QAbstractTableModel(parent) {}

int SpreadsheetModel::rowCount(const QModelIndex &parent) const {
    return parent.isValid() ? 0 : m_data.size();
}
int SpreadsheetModel::columnCount(const QModelIndex &parent) const {
    if (parent.isValid()) return 0;
    return m_data.isEmpty() ? 0 : m_data.first().size();
}

// ---------------------------------------------------------------- đọc
QVariant SpreadsheetModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) return {};
    const int row = index.row(), col = index.column();

    switch (role) {
    case Qt::EditRole:
        return m_data[row][col];
    case Qt::DisplayRole:
        if (m_showFormulas) return m_data[row][col];
        return displayValue(row, col);
    case Qt::ToolTipRole: {
        auto it = m_notes.constFind(key(row, col));
        return it != m_notes.constEnd() ? it.value() : QVariant();
    }
    case DataBarRole: {
        double frac = 0; QString barColor;
        if (!dataBarAt(row, col, frac, barColor)) return {};
        return QVariantList{ frac, barColor };
    }
    case SpillEdgesRole: {
        int t, l, b, rg; // bitmask cạnh biên vùng spill: 1=trên,2=trái,4=dưới,8=phải
        if (!spillRangeAt(row, col, t, l, b, rg)) return 0;
        int edges = 0;
        if (row == t) edges |= 1;
        if (col == l) edges |= 2;
        if (row == b) edges |= 4;
        if (col == rg) edges |= 8;
        return edges;
    }
    case Qt::TextAlignmentRole:
        return alignmentFlags(row, col);
    case Qt::FontRole:
        return fontFor(row, col);
    case Qt::BackgroundRole: {
        QString cbg = condColorFor(row, col, /*fg*/ false); // định dạng điều kiện đè màu nền
        if (!cbg.isEmpty()) return QColor(cbg);
        const Format &f = m_fmt[key(row, col)];
        auto it = f.constFind(QStringLiteral("bg"));
        return it != f.constEnd() ? QColor(it->toString()) : QVariant();
    }
    case Qt::ForegroundRole: {
        QString cfg = condColorFor(row, col, /*fg*/ true);
        if (!cfg.isEmpty()) return QColor(cfg);
        const Format &f = m_fmt[key(row, col)];
        auto it = f.constFind(QStringLiteral("color"));
        return it != f.constEnd() ? QColor(it->toString()) : QVariant();
    }
    }
    return {};
}

QString SpreadsheetModel::displayValue(int row, int col) const {
    QVariant v = evalCell(row, col);
    auto fit = m_fmt.constFind(key(row, col));
    if (fit != m_fmt.constEnd()) {
        auto cit = fit->constFind(QStringLiteral("number_format"));
        if (cit != fit->constEnd()) {
            QString shown = applyNumberFormat(v, cit->toString());
            if (!shown.isNull()) return shown;
        }
    }
    return formatValue(v);
}

QVariant SpreadsheetModel::evalCell(int row, int col) const {
    if (m_data.isEmpty() || row < 0 || col < 0 || row >= m_data.size() || col >= m_data.first().size())
        return {};
    const qint64 k = key(row, col);
    // Ô TRÀN (do anchor khác sở hữu): trả giá trị đã đổ từ mảng spill.
    auto vit = m_spillVals.constFind(k);
    if (vit != m_spillVals.constEnd() && m_spillOwner.contains(k)) return vit.value();

    const QString &raw = m_data[row][col];
    if (!formula::isFormula(raw)) return coerceLiteral(raw);

    // Anchor đã đăng ký spill (đổ được hoặc #SPILL!): trả giá trị đã ghi.
    if (vit != m_spillVals.constEnd()) return vit.value();

    auto it = m_evalCache.constFind(k);
    if (it != m_evalCache.constEnd()) return it.value();
    if (m_evaluating.contains(k)) return QString::fromLatin1(formula::ERR_REF);

    m_evaluating.insert(k);
    QVariant result;
    try {
        result = formula::evaluate(raw, [this](int r, int c) { return evalCell(r, c); }, m_sheetResolver);
    } catch (const formula::FormulaError &e) {
        result = e.etype();
    }
    m_evaluating.remove(k);
    m_evalCache.insert(k, result);
    return result;
}

QVariant SpreadsheetModel::registerSpill(int row, int col, const formula::Value &vv) const {
    const int h = vv.range.height(), w = vv.range.width();
    const qint64 anchor = key(row, col);
    const int rows = m_data.size(), cols = m_data.isEmpty() ? 0 : m_data.first().size();
    // Vùng vượt mép lưới -> #SPILL!.
    if (row + h > rows || col + w > cols) {
        m_spillVals.insert(anchor, QString::fromLatin1(formula::ERR_SPILL));
        return m_spillVals.value(anchor);
    }
    // Vùng tràn đè lên ô gộp (merge) -> #SPILL! (không xé merge).
    for (const MergeRange &m : m_merges)
        if (!(m.right < col || m.left > col + w - 1 || m.bottom < row || m.top > row + h - 1)) {
            m_spillVals.insert(anchor, QString::fromLatin1(formula::ERR_SPILL));
            return m_spillVals.value(anchor);
        }
    // Kiểm tra chặn: ô tràn (trừ anchor) phải trống & chưa bị spill khác chiếm.
    for (int r = 0; r < h; ++r)
        for (int c = 0; c < w; ++c) {
            if (r == 0 && c == 0) continue;
            const int rr = row + r, cc = col + c;
            if (!m_data[rr][cc].isEmpty()) {
                m_spillVals.insert(anchor, QString::fromLatin1(formula::ERR_SPILL));
                return m_spillVals.value(anchor);
            }
        }
    // OK -> đổ mảng.
    const auto &grid = *vv.range.rows;
    m_spillSize.insert(anchor, QSize(w, h));
    QVariant topLeft;
    for (int r = 0; r < h; ++r)
        for (int c = 0; c < w; ++c) {
            const qint64 kk = key(row + r, col + c);
            QVariant cellVal = (r < int(grid.size()) && c < int(grid[r].size()))
                                   ? grid[r][c].toVariant() : QVariant(QString());
            m_spillVals.insert(kk, cellVal);
            if (r == 0 && c == 0) topLeft = cellVal;
            else                  m_spillOwner.insert(kk, anchor);
        }
    return topLeft;
}

void SpreadsheetModel::rebuildSpills() const {
    m_spillSize.clear();
    m_spillVals.clear();
    m_spillOwner.clear();
    const int rows = m_data.size();
    for (int r = 0; r < rows; ++r) {
        const QVector<QString> &rowv = m_data[r];
        for (int c = 0; c < rowv.size(); ++c) {
            if (!formula::isFormula(rowv[c])) continue;
            formula::Value vv;
            try {
                vv = formula::evaluateValue(rowv[c],
                    [this](int rr, int cc) { return evalCell(rr, cc); }, m_sheetResolver);
            } catch (const formula::FormulaError &) {
                continue; // ô lỗi vô hướng -> evalCell hiển thị #... như cũ
            }
            if (vv.type == formula::Type::Range && (vv.range.height() > 1 || vv.range.width() > 1))
                registerSpill(r, c, vv);
        }
    }
}

bool SpreadsheetModel::spillRangeAt(int row, int col, int &top, int &left, int &bottom, int &right) const {
    const qint64 k = key(row, col);
    qint64 anchor;
    if (m_spillSize.contains(k)) anchor = k;
    else if (m_spillOwner.contains(k)) anchor = m_spillOwner.value(k);
    else return false;
    auto sit = m_spillSize.constFind(anchor);
    if (sit == m_spillSize.constEnd()) return false;
    top = keyRow(anchor); left = keyCol(anchor);
    bottom = top + sit->height() - 1; right = left + sit->width() - 1;
    return true;
}

bool SpreadsheetModel::looksNumeric(int row, int col) const {
    QVariant v = evalCell(row, col);
    return v.typeId() == QMetaType::Double || v.typeId() == QMetaType::Int
        || v.typeId() == QMetaType::LongLong;
}

int SpreadsheetModel::alignmentFlags(int row, int col) const {
    const Format &f = m_fmt[key(row, col)];
    Qt::Alignment h;
    auto hit = f.constFind(QStringLiteral("halign"));
    if (hit != f.constEnd()) h = kHAlign.value(hit->toString(), Qt::AlignLeft);
    else h = looksNumeric(row, col) ? Qt::AlignRight : Qt::AlignLeft;
    Qt::Alignment v = kVAlign.value(f.value(QStringLiteral("valign")).toString(), Qt::AlignVCenter);
    return int(h | v);
}

QVariant SpreadsheetModel::fontFor(int row, int col) const {
    auto fit = m_fmt.constFind(key(row, col));
    if (fit == m_fmt.constEnd()) return {};
    const Format &f = *fit;
    QString family = f.value(QStringLiteral("font")).toString();
    int size = f.value(QStringLiteral("size")).toInt();
    bool bold = f.value(QStringLiteral("bold")).toBool();
    bool italic = f.value(QStringLiteral("italic")).toBool();
    bool underline = f.value(QStringLiteral("underline")).toBool();
    bool strike = f.value(QStringLiteral("strike")).toBool();
    if (family.isEmpty() && !size && !bold && !italic && !underline && !strike) return {};
    QString ck = QStringLiteral("%1|%2|%3%4%5%6").arg(family).arg(size)
        .arg(bold).arg(italic).arg(underline).arg(strike);
    auto cit = m_fontCache.constFind(ck);
    if (cit != m_fontCache.constEnd()) return cit.value();
    QFont fnt;
    if (!family.isEmpty()) fnt.setFamily(family);
    if (size) fnt.setPointSize(size);
    fnt.setBold(bold); fnt.setItalic(italic); fnt.setUnderline(underline); fnt.setStrikeOut(strike);
    m_fontCache.insert(ck, fnt);
    return fnt;
}

// ---------------------------------------------------------------- ghi
bool SpreadsheetModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (role != Qt::EditRole || !index.isValid()) return false;
    const int row = index.row(), col = index.column();
    // Không cho ghi đè ô TRÀN (spill) — phải sửa ở ô gốc.
    if (m_spillOwner.contains(key(row, col))) {
        emit validationFailed(QStringLiteral("Không sửa được ô trong vùng tràn (spill); hãy sửa ô gốc."));
        return false;
    }
    QString nw = value.isNull() ? QString() : value.toString();
    QString old = m_data[row][col];
    if (old == nw) return true;
    // Kiểm tra dữ liệu: từ chối nếu vi phạm quy tắc của ô.
    for (const validation::Rule &vr : m_validationRules)
        if (vr.contains(row, col) && !validation::check(nw, vr.allow, vr.op, vr.v1, vr.v2)) {
            emit validationFailed(QStringLiteral("Giá trị không hợp lệ theo quy tắc kiểm tra dữ liệu"));
            return false;
        }
    UndoEntry e; e.cells.push_back({row, col, old, nw});
    pushUndo(std::move(e));
    m_data[row][col] = nw;
    updateDeps(row, col);
    recalculate(row, col);
    return true;
}

// ---------------------------------------------------------------- định dạng
void SpreadsheetModel::setFormat(int top, int left, int bottom, int right, const Format &attrs) {
    Format clean;
    for (auto it = attrs.constBegin(); it != attrs.constEnd(); ++it)
        if (kFormatKeys.contains(it.key())) clean.insert(it.key(), it.value());
    if (clean.isEmpty()) return;

    UndoEntry e;
    for (int r = top; r <= bottom; ++r) {
        for (int c = left; c <= right; ++c) {
            Format oldF = m_fmt.value(key(r, c));
            Format newF = oldF;
            for (auto it = clean.constBegin(); it != clean.constEnd(); ++it) {
                if (!it.value().isValid() || it.value().isNull()) newF.remove(it.key());
                else newF.insert(it.key(), it.value());
            }
            if (newF != oldF) e.fmts.push_back({r, c, oldF, newF});
        }
    }
    if (e.fmts.isEmpty()) return;
    for (const auto &fc : e.fmts) {
        if (fc.newFmt.isEmpty()) m_fmt.remove(key(fc.row, fc.col));
        else m_fmt.insert(key(fc.row, fc.col), fc.newFmt);
    }
    pushUndo(std::move(e));
    m_fontCache.clear();
    emit dataChanged(index(top, left), index(bottom, right));
}

QString SpreadsheetModel::condColorFor(int row, int col, bool fg) const {
    if (m_condRules.isEmpty()) return QString();
    QString result; // quy tắc khớp sau cùng thắng (đè quy tắc trước)
    QVariant v;
    bool evaluated = false;
    for (const cond::Rule &rule : m_condRules) {
        if (!rule.contains(row, col)) continue;
        if (!evaluated) { v = evalCell(row, col); evaluated = true; }
        if (cond::match(v, rule.op, rule.v1, rule.v2, rule.text)) {
            const QString &c = fg ? rule.color : rule.bg;
            if (!c.isEmpty()) result = c;
        }
    }
    return result;
}

void SpreadsheetModel::addCondRule(const cond::Rule &rule) {
    m_condRules.push_back(rule);
    emit dataChanged(index(rule.top, rule.left), index(rule.bottom, rule.right),
                     {Qt::BackgroundRole, Qt::ForegroundRole});
}

void SpreadsheetModel::clearCondRules() {
    if (m_condRules.isEmpty()) return;
    m_condRules.clear();
    if (rowCount() && columnCount())
        emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1),
                         {Qt::BackgroundRole, Qt::ForegroundRole});
}

void SpreadsheetModel::addDataBar(const cond::DataBar &bar) {
    m_dataBars.push_back(bar);
    emit dataChanged(index(bar.top, bar.left), index(bar.bottom, bar.right));
}

void SpreadsheetModel::clearDataBars() {
    if (m_dataBars.isEmpty()) return;
    m_dataBars.clear();
    if (rowCount() && columnCount())
        emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));
}

bool SpreadsheetModel::dataBarAt(int row, int col, double &fraction, QString &color) const {
    // Quy tắc khớp sau cùng thắng (đè quy tắc trước, giống cond rule).
    const cond::DataBar *hit = nullptr;
    for (const cond::DataBar &b : m_dataBars)
        if (b.contains(row, col)) hit = &b;
    if (!hit) return false;
    bool ok = false;
    const double v = evalCell(row, col).toDouble(&ok);
    if (!ok) return false; // ô không phải số -> không vẽ thanh
    // min/max trên các ô SỐ của vùng.
    double mn = 0, mx = 0; bool has = false;
    for (int r = hit->top; r <= hit->bottom; ++r)
        for (int c = hit->left; c <= hit->right; ++c) {
            bool okc = false;
            double d = evalCell(r, c).toDouble(&okc);
            if (!okc) continue;
            if (!has) { mn = mx = d; has = true; }
            else { mn = qMin(mn, d); mx = qMax(mx, d); }
        }
    if (!has) return false;
    fraction = cond::dataBarFraction(v, mn, mx);
    color = hit->color;
    return true;
}

void SpreadsheetModel::addValidationRule(const validation::Rule &rule) { m_validationRules.push_back(rule); }
void SpreadsheetModel::clearValidationRules() { m_validationRules.clear(); }

void SpreadsheetModel::defineName(const QString &name, const MergeRange &range) {
    if (!name.isEmpty()) m_names.insert(name, range);
}

bool SpreadsheetModel::lookupName(const QString &name, MergeRange &out) const {
    auto it = m_names.constFind(name);
    if (it == m_names.constEnd()) return false;
    out = it.value();
    return true;
}

bool SpreadsheetModel::removeName(const QString &name) {
    return m_names.remove(name) > 0;
}

QString SpreadsheetModel::rangeRef(const MergeRange &r) {
    const QString a = columnLabel(r.left) + QString::number(r.top + 1);
    if (r.top == r.bottom && r.left == r.right) return a;
    const QString b = columnLabel(r.right) + QString::number(r.bottom + 1);
    return a + QLatin1Char(':') + b;
}

void SpreadsheetModel::setNote(int row, int col, const QString &text) {
    const qint64 k = key(row, col);
    if (text.isEmpty()) m_notes.remove(k);
    else m_notes.insert(k, text);
    emit dataChanged(index(row, col), index(row, col), {Qt::ToolTipRole, Qt::DecorationRole});
}

QMap<QPair<int, int>, SpreadsheetModel::Format> SpreadsheetModel::cellFormats() const {
    QMap<QPair<int, int>, Format> m;
    for (auto it = m_fmt.constBegin(); it != m_fmt.constEnd(); ++it)
        if (!it.value().isEmpty()) m.insert({keyRow(it.key()), keyCol(it.key())}, it.value());
    return m;
}

void SpreadsheetModel::setCellFormats(const QMap<QPair<int, int>, Format> &fmts) {
    m_fmt.clear();
    for (auto it = fmts.constBegin(); it != fmts.constEnd(); ++it)
        if (!it.value().isEmpty()) m_fmt.insert(key(it.key().first, it.key().second), it.value());
    m_fontCache.clear();
    if (rowCount() && columnCount())
        emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));
}

// ---------------------------------------------------------------- hiện công thức
void SpreadsheetModel::setShowFormulas(bool on) {
    if (on == m_showFormulas) return;
    m_showFormulas = on;
    if (rowCount() && columnCount())
        emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1), {Qt::DisplayRole});
}

// ---------------------------------------------------------------- đồ thị phụ thuộc
void SpreadsheetModel::rebuildDeps() {
    m_deps.clear();
    m_dependents.clear();
    for (int r = 0; r < m_data.size(); ++r)
        for (int c = 0; c < m_data[r].size(); ++c)
            if (formula::isFormula(m_data[r][c])) {
                auto refs = formula::extractRefs(m_data[r][c]);
                if (!refs.isEmpty()) {
                    QSet<qint64> set;
                    for (const auto &p : refs) { qint64 dk = key(p.first, p.second); set.insert(dk); m_dependents[dk].insert(key(r, c)); }
                    m_deps.insert(key(r, c), set);
                }
            }
}

void SpreadsheetModel::updateDeps(int row, int col) {
    const qint64 k = key(row, col);
    auto old = m_deps.take(k);
    for (qint64 dep : old) {
        auto it = m_dependents.find(dep);
        if (it != m_dependents.end()) { it->remove(k); if (it->isEmpty()) m_dependents.erase(it); }
    }
    const QString &raw = m_data[row][col];
    if (formula::isFormula(raw)) {
        auto refs = formula::extractRefs(raw);
        if (!refs.isEmpty()) {
            QSet<qint64> set;
            for (const auto &p : refs) { qint64 dk = key(p.first, p.second); set.insert(dk); m_dependents[dk].insert(k); }
            m_deps.insert(k, set);
        }
    }
}

// Lan truyền BFS qua một bản đồ phụ thuộc (m_deps hoặc m_dependents) từ ô gốc.
// directOnly=true -> chỉ lấy mức kế cận trực tiếp. Bỏ chính ô gốc khỏi kết quả.
static QVector<QPair<int, int>> traceDeps(const QHash<qint64, QSet<qint64>> &graph,
                                          qint64 start, bool allLevels) {
    QVector<QPair<int, int>> out;
    QSet<qint64> seen;
    QList<qint64> queue;
    auto it0 = graph.constFind(start);
    if (it0 != graph.constEnd()) for (qint64 d : it0.value()) queue.push_back(d);
    while (!queue.isEmpty()) {
        qint64 cur = queue.takeFirst();
        if (cur == start || seen.contains(cur)) continue;
        seen.insert(cur);
        out.push_back({int(cur >> 32), int(cur & 0xffffffff)}); // giải mã key(row,col)
        if (allLevels) {
            auto it = graph.constFind(cur);
            if (it != graph.constEnd()) for (qint64 d : it.value()) if (!seen.contains(d)) queue.push_back(d);
        }
    }
    return out;
}

QVector<QPair<int, int>> SpreadsheetModel::precedents(int row, int col, bool allLevels) const {
    return traceDeps(m_deps, key(row, col), allLevels);
}

QVector<QPair<int, int>> SpreadsheetModel::dependents(int row, int col, bool allLevels) const {
    return traceDeps(m_dependents, key(row, col), allLevels);
}

void SpreadsheetModel::recalculate(int row, int col) {
    // BFS ngược qua _dependents từ ô vừa đổi; xóa cache ô dirty + emit bounding-box.
    QSet<qint64> dirty;
    QVector<qint64> queue{key(row, col)};
    while (!queue.isEmpty()) {
        qint64 cell = queue.takeLast();
        if (dirty.contains(cell)) continue;
        dirty.insert(cell);
        auto it = m_dependents.constFind(cell);
        if (it != m_dependents.constEnd())
            for (qint64 d : *it) if (!dirty.contains(d)) queue.push_back(d);
    }
    int rMin = INT_MAX, rMax = INT_MIN, cMin = INT_MAX, cMax = INT_MIN;
    for (qint64 cell : dirty) {
        m_evalCache.remove(cell);
        const int r = keyRow(cell), c = keyCol(cell);
        rMin = qMin(rMin, r); rMax = qMax(rMax, r);
        cMin = qMin(cMin, c); cMax = qMax(cMax, c);
    }
    const bool hadSpill = !m_spillSize.isEmpty();
    rebuildSpills(); // vùng spill có thể đổi (anchor sửa / ô chặn bỏ đi) -> dựng lại
    // Khi có mảng động, spill có thể đổi ô NGOÀI bounding-box -> phát lại toàn lưới cho chắc.
    // Trường hợp thường (không spill) chỉ phát bounding-box để khỏi vẽ lại cả lưới.
    if (rowCount() && columnCount()) {
        if (hadSpill || !m_spillSize.isEmpty())
            emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));
        else
            emit dataChanged(index(rMin, cMin), index(rMax, cMax), {Qt::DisplayRole});
    }
    emit contentChanged();
}

void SpreadsheetModel::recalculateAll() {
    m_evalCache.clear();
    rebuildSpills();
    if (rowCount() && columnCount())
        emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));
    emit contentChanged();
}

// ---------------------------------------------------------------- undo/redo
void SpreadsheetModel::pushUndo(UndoEntry entry) {
    m_redo.clear();
    m_undo.push_back(std::move(entry));
    if (m_undo.size() > kUndoLimit) m_undo.removeFirst();
}

void SpreadsheetModel::applyEntry(const UndoEntry &e, bool useOld) {
    if (e.snapBefore.has_value()) {            // thao tác cấu trúc -> khôi phục toàn lưới
        restoreSnapshot(useOld ? *e.snapBefore : *e.snapAfter);
        return;
    }
    for (const auto &cc : e.cells) {
        m_data[cc.row][cc.col] = useOld ? cc.oldVal : cc.newVal;
        updateDeps(cc.row, cc.col);
    }
    for (const auto &fc : e.fmts) {
        const Format &f = useOld ? fc.oldFmt : fc.newFmt;
        if (f.isEmpty()) m_fmt.remove(key(fc.row, fc.col));
        else m_fmt.insert(key(fc.row, fc.col), f);
    }
    if (!e.fmts.isEmpty()) m_fontCache.clear();
    if (e.hasMerges) { m_merges = useOld ? e.mergesOld : e.mergesNew; emit mergesChanged(); }
    recalculateAll();
}

bool SpreadsheetModel::undo() {
    if (m_undo.isEmpty()) return false;
    UndoEntry e = m_undo.takeLast();
    applyEntry(e, /*useOld*/ true);
    m_redo.push_back(std::move(e));
    return true;
}

bool SpreadsheetModel::redo() {
    if (m_redo.isEmpty()) return false;
    UndoEntry e = m_redo.takeLast();
    applyEntry(e, /*useOld*/ false);
    m_undo.push_back(std::move(e));
    return true;
}

// ---------------------------------------------------------------- thao tác vùng
void SpreadsheetModel::applyCellChanges(QVector<CellChange> changes) {
    // Bỏ thay đổi không thực sự đổi.
    QVector<CellChange> real;
    for (auto &c : changes) if (c.oldVal != c.newVal) real.push_back(c);
    if (real.isEmpty()) return;
    UndoEntry e; e.cells = real;
    for (const auto &c : real) { m_data[c.row][c.col] = c.newVal; updateDeps(c.row, c.col); }
    pushUndo(std::move(e));
    recalculateAll();
}

int SpreadsheetModel::replaceAll(const QString &find, const QString &repl, bool matchCase) {
    if (find.isEmpty()) return 0;
    QVector<CellChange> changes;
    for (int r = 0; r < m_data.size(); ++r)
        for (int c = 0; c < m_data[r].size(); ++c) {
            const QString &raw = m_data[r][c];
            QString nw = textsearch::replaceSubstr(raw, find, repl, matchCase);
            if (nw != raw) changes.push_back({r, c, raw, nw});
        }
    int n = changes.size();
    applyCellChanges(std::move(changes));
    return n;
}

void SpreadsheetModel::sortRange(int top, int left, int bottom, int right,
                                 int keyCol, bool ascending) {
    if (top < 0 || left < 0 || bottom >= m_data.size() || keyCol < left || keyCol > right)
        return;
    // Trích block, sắp theo cột (đổi keyCol về chỉ số trong block).
    QVector<QVector<QString>> block;
    for (int r = top; r <= bottom; ++r)
        block.push_back(m_data[r].mid(left, right - left + 1));
    QVector<sort::SortKey> keys{
        {keyCol - left, ascending ? sort::Order::Ascending : sort::Order::Descending}};
    QVector<QVector<QString>> sorted = sort::sortRows(block, keys);

    QVector<CellChange> changes;
    for (int i = 0; i < sorted.size(); ++i)
        for (int j = 0; j < sorted[i].size(); ++j) {
            int r = top + i, c = left + j;
            if (m_data[r][c] != sorted[i][j])
                changes.push_back({r, c, m_data[r][c], sorted[i][j]});
        }
    applyCellChanges(std::move(changes));
}

void SpreadsheetModel::sortRangeMulti(int top, int left, int bottom, int right,
                                      const QVector<QPair<int, bool>> &keyCols) {
    if (top < 0 || left < 0 || bottom >= m_data.size() || keyCols.isEmpty()) return;
    QVector<sort::SortKey> keys;
    for (const auto &kc : keyCols) {
        if (kc.first < left || kc.first > right) continue; // bỏ cấp ngoài vùng
        keys.push_back({kc.first - left, kc.second ? sort::Order::Ascending : sort::Order::Descending});
    }
    if (keys.isEmpty()) return;

    QVector<QVector<QString>> block;
    for (int r = top; r <= bottom; ++r)
        block.push_back(m_data[r].mid(left, right - left + 1));
    QVector<QVector<QString>> sorted = sort::sortRows(block, keys);

    QVector<CellChange> changes;
    for (int i = 0; i < sorted.size(); ++i)
        for (int j = 0; j < sorted[i].size(); ++j) {
            int r = top + i, c = left + j;
            if (m_data[r][c] != sorted[i][j])
                changes.push_back({r, c, m_data[r][c], sorted[i][j]});
        }
    applyCellChanges(std::move(changes));
}

void SpreadsheetModel::clearRange(int top, int left, int bottom, int right) {
    QVector<CellChange> changes;
    for (int r = top; r <= bottom; ++r)
        for (int c = left; c <= right; ++c)
            if (r < m_data.size() && c < m_data[r].size() && !m_data[r][c].isEmpty())
                changes.push_back({r, c, m_data[r][c], QString()});
    applyCellChanges(std::move(changes));
}

// Chỉ xóa định dạng (giữ nội dung) — undoable. Mirror setFormat: gom FmtChange,
// gỡ khỏi m_fmt, đẩy undo, dọn cache font, báo dataChanged.
void SpreadsheetModel::clearFormatsRange(int top, int left, int bottom, int right) {
    UndoEntry e;
    for (int r = top; r <= bottom; ++r)
        for (int c = left; c <= right; ++c) {
            Format oldF = m_fmt.value(key(r, c));
            if (!oldF.isEmpty()) e.fmts.push_back({r, c, oldF, Format()});
        }
    if (e.fmts.isEmpty()) return;
    for (const auto &fc : e.fmts) m_fmt.remove(key(fc.row, fc.col));
    pushUndo(std::move(e));
    m_fontCache.clear();
    emit dataChanged(index(top, left), index(bottom, right));
}

// Xóa cả nội dung lẫn định dạng trong một bước hoàn tác duy nhất.
void SpreadsheetModel::clearAllRange(int top, int left, int bottom, int right) {
    UndoEntry e;
    for (int r = top; r <= bottom; ++r)
        for (int c = left; c <= right; ++c) {
            if (r < m_data.size() && c < m_data[r].size() && !m_data[r][c].isEmpty())
                e.cells.push_back({r, c, m_data[r][c], QString()});
            Format oldF = m_fmt.value(key(r, c));
            if (!oldF.isEmpty()) e.fmts.push_back({r, c, oldF, Format()});
        }
    if (e.cells.isEmpty() && e.fmts.isEmpty()) return;
    const bool hadFmts = !e.fmts.isEmpty();
    for (const auto &cc : e.cells) { m_data[cc.row][cc.col] = QString(); updateDeps(cc.row, cc.col); }
    for (const auto &fc : e.fmts) m_fmt.remove(key(fc.row, fc.col));
    pushUndo(std::move(e));
    if (hadFmts) m_fontCache.clear();
    recalculateAll();
    emit dataChanged(index(top, left), index(bottom, right));
}

void SpreadsheetModel::pasteBlock(int top, int left, const QVector<QVector<QString>> &block,
                                  int srcAnchorRow, int srcAnchorCol) {
    QVector<CellChange> changes;
    bool offset = (srcAnchorRow >= 0 && srcAnchorCol >= 0);
    for (int i = 0; i < block.size(); ++i) {
        for (int j = 0; j < block[i].size(); ++j) {
            int r = top + i, c = left + j;
            if (r >= m_data.size() || c >= m_data[r].size()) continue; // chưa hỗ trợ tự nới lưới
            QString val = block[i][j];
            if (offset && formula::isFormula(val))
                val = formula::offsetFormula(val, top - srcAnchorRow, left - srcAnchorCol);
            changes.push_back({r, c, m_data[r][c], val});
        }
    }
    applyCellChanges(std::move(changes));
}

void SpreadsheetModel::autofillVertical(int col, int srcTop, int srcBottom, int dstBottom) {
    int srcLen = srcBottom - srcTop + 1;
    if (srcLen <= 0 || dstBottom <= srcBottom || col < 0 || col >= columnCount()) return;
    QStringList src;
    for (int r = srcTop; r <= srcBottom; ++r) src << m_data[r][col];
    auto series = asSeries(src);
    QVector<CellChange> changes;
    for (int r = srcBottom + 1; r <= dstBottom && r < rowCount(); ++r) {
        QString val;
        if (series) {
            val = numToText(series->first + series->second * (r - srcTop));
        } else {
            int idx = (r - srcTop) % srcLen;
            QString base = src[idx];
            if (srcLen == 1) {
                auto cal = seriesfill::next(base, r - srcBottom);
                auto inc = incrementTrailing(base, r - srcBottom);
                if (cal) val = *cal;
                else if (inc) val = *inc;
                else if (formula::isFormula(base)) val = formula::offsetFormula(base, r - srcTop, 0);
                else val = base;
            } else {
                val = formula::isFormula(base) ? formula::offsetFormula(base, r - (srcTop + idx), 0) : base;
            }
        }
        changes.push_back({r, col, m_data[r][col], val});
    }
    applyCellChanges(std::move(changes));
}

void SpreadsheetModel::autofillHorizontal(int row, int srcLeft, int srcRight, int dstRight) {
    int srcLen = srcRight - srcLeft + 1;
    if (srcLen <= 0 || dstRight <= srcRight || row < 0 || row >= rowCount()) return;
    QStringList src;
    for (int c = srcLeft; c <= srcRight; ++c) src << m_data[row][c];
    auto series = asSeries(src);
    QVector<CellChange> changes;
    for (int c = srcRight + 1; c <= dstRight && c < columnCount(); ++c) {
        QString val;
        if (series) {
            val = numToText(series->first + series->second * (c - srcLeft));
        } else {
            int idx = (c - srcLeft) % srcLen;
            QString base = src[idx];
            if (srcLen == 1) {
                auto cal = seriesfill::next(base, c - srcRight);
                auto inc = incrementTrailing(base, c - srcRight);
                if (cal) val = *cal;
                else if (inc) val = *inc;
                else if (formula::isFormula(base)) val = formula::offsetFormula(base, 0, c - srcLeft);
                else val = base;
            } else {
                val = formula::isFormula(base) ? formula::offsetFormula(base, 0, c - (srcLeft + idx)) : base;
            }
        }
        changes.push_back({row, c, m_data[row][c], val});
    }
    applyCellChanges(std::move(changes));
}

// ---------------------------------------------------------------- gộp ô (merge)
namespace {
bool boxesOverlap(const MergeRange &a, const MergeRange &b) {
    return !(a.right < b.left || b.right < a.left || a.bottom < b.top || b.bottom < a.top);
}
}

std::optional<MergeRange> SpreadsheetModel::mergeAt(int row, int col) const {
    for (const auto &m : m_merges)
        if (m.contains(row, col)) return m;
    return std::nullopt;
}

bool SpreadsheetModel::mergeBoxInto(const MergeRange &box, UndoEntry &e) {
    if (box.top == box.bottom && box.left == box.right) return false; // 1 ô: khỏi gộp
    // Bỏ các vùng gộp cũ giao với vùng mới.
    QVector<MergeRange> kept;
    for (const auto &m : m_merges) if (!boxesOverlap(m, box)) kept.push_back(m);
    m_merges = kept;
    // Xóa nội dung mọi ô trừ ô góc trên-trái (ghi lại để undo).
    for (int r = box.top; r <= box.bottom; ++r) {
        for (int c = box.left; c <= box.right; ++c) {
            if (r == box.top && c == box.left) continue;
            if (r >= m_data.size() || c >= m_data[r].size()) continue;
            if (!m_data[r][c].isEmpty()) {
                e.cells.push_back({r, c, m_data[r][c], QString()});
                m_data[r][c].clear();
                updateDeps(r, c);
            }
        }
    }
    m_merges.push_back(box);
    return true;
}

bool SpreadsheetModel::unmergeBoxInto(const MergeRange &box) {
    QVector<MergeRange> kept;
    bool hit = false;
    for (const auto &m : m_merges) {
        if (boxesOverlap(m, box)) hit = true;
        else kept.push_back(m);
    }
    if (!hit) return false;
    m_merges = kept;
    return true;
}

void SpreadsheetModel::mergeCells(int top, int left, int bottom, int right) {
    toggleMergeRangesImpl({MergeRange{top, left, bottom, right}}, /*forceMerge*/ true);
}

void SpreadsheetModel::unmergeCells(int top, int left, int bottom, int right) {
    QVector<MergeRange> before = m_merges;
    UndoEntry e;
    if (!unmergeBoxInto({top, left, bottom, right})) return;
    e.hasMerges = true; e.mergesOld = before; e.mergesNew = m_merges;
    pushUndo(std::move(e));
    recalculateAll();
    emit mergesChanged();
}

void SpreadsheetModel::toggleMerge(int top, int left, int bottom, int right) {
    toggleMergeRanges({MergeRange{top, left, bottom, right}});
}

void SpreadsheetModel::toggleMergeRanges(const QVector<MergeRange> &boxes) {
    toggleMergeRangesImpl(boxes, /*forceMerge*/ false);
}

void SpreadsheetModel::toggleMergeRangesImpl(const QVector<MergeRange> &boxes, bool forceMerge) {
    if (boxes.isEmpty()) return;
    // Nhất quán như Excel: nếu bất kỳ vùng nào đang gộp -> bỏ gộp tất cả.
    bool anyMerged = false;
    if (!forceMerge)
        for (const auto &b : boxes)
            for (const auto &m : m_merges)
                if (boxesOverlap(m, b)) { anyMerged = true; break; }

    QVector<MergeRange> before = m_merges;
    UndoEntry e;
    bool changed = false;
    for (const auto &box : boxes) {
        if (anyMerged) changed = unmergeBoxInto(box) || changed;
        else changed = mergeBoxInto(box, e) || changed;
    }
    if (!changed) return;
    e.hasMerges = true; e.mergesOld = before; e.mergesNew = m_merges;
    pushUndo(std::move(e));
    recalculateAll();
    emit mergesChanged();
}

// ---------------------------------------------------------------- chèn/xóa hàng-cột
SpreadsheetModel::Snapshot SpreadsheetModel::snapshot() const {
    return Snapshot{m_data, m_fmt, m_merges};
}

void SpreadsheetModel::restoreSnapshot(const Snapshot &s) {
    beginResetModel();
    m_data = s.data; m_fmt = s.fmt; m_merges = s.merges;
    m_evalCache.clear(); m_evaluating.clear(); m_fontCache.clear();
    rebuildDeps();
    endResetModel();
    emit mergesChanged();
    emit contentChanged();
}

void SpreadsheetModel::shiftFmtRows(int row, int count) {
    QHash<qint64, Format> nf;
    for (auto it = m_fmt.constBegin(); it != m_fmt.constEnd(); ++it) {
        int r = keyRow(it.key()), c = keyCol(it.key());
        if (count < 0 && r >= row && r < row - count) continue; // ô bị xóa
        if (r >= row) r += count;
        nf.insert(key(r, c), it.value());
    }
    m_fmt = nf;
}

void SpreadsheetModel::shiftFmtCols(int col, int count) {
    QHash<qint64, Format> nf;
    for (auto it = m_fmt.constBegin(); it != m_fmt.constEnd(); ++it) {
        int r = keyRow(it.key()), c = keyCol(it.key());
        if (count < 0 && c >= col && c < col - count) continue; // ô bị xóa
        if (c >= col) c += count;
        nf.insert(key(r, c), it.value());
    }
    m_fmt = nf;
}

namespace {
// Dời/co vùng gộp theo trục: chèn (count>0) hoặc xóa (count<0) tại pivot.
// Trả false nếu vùng nằm trọn trong dải bị xóa (cần loại bỏ).
bool shiftBand(int &lo, int &hi, int pivot, int count) {
    if (count > 0) {                 // chèn
        if (lo >= pivot) lo += count;
        if (hi >= pivot) hi += count;
    } else {                          // xóa [pivot, pivot-count)
        int del = -count, end = pivot + del;
        if (lo >= pivot && hi < end) return false;       // trọn trong dải xóa
        if (lo >= end) lo -= del; else if (lo >= pivot) lo = pivot;
        if (hi >= end) hi -= del; else if (hi >= pivot) hi = pivot - 1;
        if (hi < lo) return false;
    }
    return true;
}
} // namespace

void SpreadsheetModel::insertRows(int row, int count) {
    if (count <= 0) return;
    Snapshot before = snapshot();
    int width = columnCount() ? columnCount() : 1;
    beginInsertRows(QModelIndex(), row, row + count - 1);
    for (int i = 0; i < count; ++i) m_data.insert(row, QVector<QString>(width));
    shiftFmtRows(row, count);
    QVector<MergeRange> nm;
    for (auto m : m_merges) if (shiftBand(m.top, m.bottom, row, count)) nm.push_back(m);
    m_merges = nm;
    endInsertRows();
    m_evalCache.clear(); rebuildDeps();
    UndoEntry e; e.snapBefore = before; e.snapAfter = snapshot();
    pushUndo(std::move(e));
    recalculateAll(); emit mergesChanged();
}

void SpreadsheetModel::removeRows(int row, int count) {
    if (count <= 0 || rowCount() - count < 1) return;
    Snapshot before = snapshot();
    beginRemoveRows(QModelIndex(), row, row + count - 1);
    m_data.remove(row, count);
    shiftFmtRows(row, -count);
    QVector<MergeRange> nm;
    for (auto m : m_merges) if (shiftBand(m.top, m.bottom, row, -count)) nm.push_back(m);
    m_merges = nm;
    endRemoveRows();
    m_evalCache.clear(); rebuildDeps();
    UndoEntry e; e.snapBefore = before; e.snapAfter = snapshot();
    pushUndo(std::move(e));
    recalculateAll(); emit mergesChanged();
}

void SpreadsheetModel::insertColumns(int col, int count) {
    if (count <= 0) return;
    Snapshot before = snapshot();
    beginInsertColumns(QModelIndex(), col, col + count - 1);
    for (auto &r : m_data) for (int i = 0; i < count; ++i) r.insert(col, QString());
    shiftFmtCols(col, count);
    QVector<MergeRange> nm;
    for (auto m : m_merges) if (shiftBand(m.left, m.right, col, count)) nm.push_back(m);
    m_merges = nm;
    endInsertColumns();
    m_evalCache.clear(); rebuildDeps();
    UndoEntry e; e.snapBefore = before; e.snapAfter = snapshot();
    pushUndo(std::move(e));
    recalculateAll(); emit mergesChanged();
}

void SpreadsheetModel::removeColumns(int col, int count) {
    if (count <= 0 || columnCount() - count < 1) return;
    Snapshot before = snapshot();
    beginRemoveColumns(QModelIndex(), col, col + count - 1);
    for (auto &r : m_data) r.remove(col, count);
    shiftFmtCols(col, -count);
    QVector<MergeRange> nm;
    for (auto m : m_merges) if (shiftBand(m.left, m.right, col, -count)) nm.push_back(m);
    m_merges = nm;
    endRemoveColumns();
    m_evalCache.clear(); rebuildDeps();
    UndoEntry e; e.snapBefore = before; e.snapAfter = snapshot();
    pushUndo(std::move(e));
    recalculateAll(); emit mergesChanged();
}

bool SpreadsheetModel::shiftCells(int top, int left, int bottom, int right, cellshift::Dir dir)
{
    const int rows = rowCount(), cols = columnCount();
    if (top < 0 || left < 0 || bottom >= rows || right >= cols || top > bottom || left > right)
        return false;

    // Vùng ẢNH HƯỞNG: Right/Left dời cả hàng tới mép phải; Down/Up dời cả cột tới mép dưới.
    int aTop = top, aLeft = left, aBottom = bottom, aRight = right;
    if (dir == cellshift::Dir::Right || dir == cellshift::Dir::Left) aRight = cols - 1;
    else                                                             aBottom = rows - 1;

    // Từ chối nếu vùng ảnh hưởng chạm bất kỳ ô gộp nào (tránh xé merge).
    for (const auto &m : m_merges)
        if (!(m.right < aLeft || m.left > aRight || m.bottom < aTop || m.top > aBottom))
            return false;

    const auto moves = cellshift::plan(rows, cols, top, left, bottom, right, dir);
    if (moves.isEmpty()) return false;

    Snapshot before = snapshot();
    // Đọc trạng thái CŨ trước khi ghi để các phép dời không đè lẫn nhau.
    const QVector<QVector<QString>> oldData = m_data;
    const QHash<qint64, Format> oldFmt = m_fmt;
    const QHash<qint64, QString> oldNotes = m_notes;
    for (const cellshift::Move &mv : moves) {
        const qint64 dk = key(mv.dstR, mv.dstC);
        if (mv.srcR < 0) {                       // ô đích để trống
            m_data[mv.dstR][mv.dstC].clear();
            m_fmt.remove(dk);
            m_notes.remove(dk);
        } else {
            const qint64 sk = key(mv.srcR, mv.srcC);
            m_data[mv.dstR][mv.dstC] = oldData[mv.srcR][mv.srcC];
            if (oldFmt.contains(sk)) m_fmt[dk] = oldFmt.value(sk); else m_fmt.remove(dk);
            if (oldNotes.contains(sk)) m_notes[dk] = oldNotes.value(sk); else m_notes.remove(dk);
        }
    }
    m_evalCache.clear();
    rebuildDeps();
    UndoEntry e; e.snapBefore = before; e.snapAfter = snapshot();
    pushUndo(std::move(e));
    recalculateAll();
    emit dataChanged(index(aTop, aLeft), index(aBottom, aRight));
    return true;
}

// ---------------------------------------------------------------- header / flags / kích thước
QVariant SpreadsheetModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole) return {};
    return orientation == Qt::Horizontal ? QVariant(columnLabel(section)) : QVariant(section + 1);
}

Qt::ItemFlags SpreadsheetModel::flags(const QModelIndex &index) const {
    if (!index.isValid()) return Qt::NoItemFlags;
    Qt::ItemFlags f = Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
    // Ô TRÀN (spill) không sửa được — chỉ sửa ô gốc (anchor).
    if (m_spillOwner.contains(key(index.row(), index.column()))) f &= ~Qt::ItemIsEditable;
    return f;
}

void SpreadsheetModel::resizeGrid(int rows, int cols) {
    beginResetModel();
    m_data.resize(rows);
    for (auto &row : m_data) row.resize(cols);
    m_evalCache.clear();
    rebuildDeps();
    endResetModel();
}

void SpreadsheetModel::loadGrid(const QVector<QVector<QString>> &rows) {
    beginResetModel();
    m_data = rows;
    // Bảo đảm lưới chữ nhật (đề phòng nguồn không chuẩn).
    int width = 0;
    for (const auto &r : m_data) width = qMax(width, int(r.size()));
    width = qMax(width, 1);
    if (m_data.isEmpty()) m_data.push_back(QVector<QString>(width));
    for (auto &r : m_data) while (r.size() < width) r.push_back(QString());

    m_evalCache.clear();
    m_evaluating.clear();
    m_fmt.clear();
    m_merges.clear();
    m_condRules.clear();
    m_validationRules.clear();
    m_notes.clear();
    m_names.clear();
    m_undo.clear();
    m_redo.clear();
    rebuildDeps();
    endResetModel();
    emit contentChanged();
    emit mergesChanged();
}

QString SpreadsheetModel::columnLabel(int col) {
    QString s; int n = col;
    do { s.prepend(QChar('A' + (n % 26))); n = n / 26 - 1; } while (n >= 0);
    return s;
}
