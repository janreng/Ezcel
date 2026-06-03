#include "model/SpreadsheetModel.h"
#include "formula/Formula.h"

#include <QColor>
#include <QDate>
#include <QDateTime>
#include <cmath>
#include <climits>

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

// Mã định dạng ngày -> format QDate.
const QHash<QString, QString> kDateCodes = {
    {"dd/mm/yyyy", "dd/MM/yyyy"}, {"mm/dd/yyyy", "MM/dd/yyyy"},
    {"yyyy-mm-dd", "yyyy-MM-dd"}, {"hh:mm:ss", "HH:mm:ss"}, {"hh:mm", "HH:mm"},
};
const QDate kEpoch(1899, 12, 30);

// Áp number_format cơ bản (ngày + % + số thập phân). null nếu không áp được.
QString applyNumberFormat(const QVariant &v, const QString &code) {
    if (v.typeId() != QMetaType::Double && v.typeId() != QMetaType::Int) return QString();
    double val = v.toDouble();
    if (kDateCodes.contains(code)) {
        QDateTime dt(kEpoch.startOfDay());
        dt = dt.addMSecs(qint64(val * 86400000.0));
        return dt.isValid() ? dt.toString(kDateCodes.value(code)) : QString();
    }
    bool percent = code.contains('%');
    QString body = code; body.remove('%');
    double v2 = percent ? val * 100 : val;
    int decimals = 0;
    int dot = body.indexOf('.');
    if (dot >= 0) for (int i = dot + 1; i < body.size(); ++i) if (body[i] == '0') ++decimals;
    bool thousands = body.left(dot < 0 ? body.size() : dot).contains(',');
    QString core = QString::number(std::abs(v2), 'f', decimals);
    if (thousands) {
        // chèn dấu phẩy hàng nghìn vào phần nguyên
        int dotPos = core.indexOf('.'); QString intPart = dotPos<0?core:core.left(dotPos);
        QString frac = dotPos<0?QString():core.mid(dotPos);
        for (int i = intPart.size() - 3; i > 0; i -= 3) intPart.insert(i, ',');
        core = intPart + frac;
    }
    QString sign = v2 < 0 ? "-" : "";
    return sign + core + (percent ? "%" : "");
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
    case Qt::TextAlignmentRole:
        return alignmentFlags(row, col);
    case Qt::FontRole:
        return fontFor(row, col);
    case Qt::BackgroundRole: {
        const Format &f = m_fmt[key(row, col)];
        auto it = f.constFind(QStringLiteral("bg"));
        return it != f.constEnd() ? QColor(it->toString()) : QVariant();
    }
    case Qt::ForegroundRole: {
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
    const QString &raw = m_data[row][col];
    if (!formula::isFormula(raw)) return coerceLiteral(raw);

    const qint64 k = key(row, col);
    auto it = m_evalCache.constFind(k);
    if (it != m_evalCache.constEnd()) return it.value();
    if (m_evaluating.contains(k)) return QString::fromLatin1(formula::ERR_REF);

    m_evaluating.insert(k);
    QVariant result;
    try {
        result = formula::evaluate(raw, [this](int r, int c) { return evalCell(r, c); });
    } catch (const formula::FormulaError &e) {
        result = e.etype();
    }
    m_evaluating.remove(k);
    m_evalCache.insert(k, result);
    return result;
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
    QString nw = value.isNull() ? QString() : value.toString();
    QString old = m_data[row][col];
    if (old == nw) return true;
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
        int r = keyRow(cell), c = keyCol(cell);
        rMin = qMin(rMin, r); rMax = qMax(rMax, r);
        cMin = qMin(cMin, c); cMax = qMax(cMax, c);
    }
    emit dataChanged(index(rMin, cMin), index(rMax, cMax), {Qt::DisplayRole});
    emit contentChanged();
}

void SpreadsheetModel::recalculateAll() {
    m_evalCache.clear();
    if (rowCount() && columnCount())
        emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1), {Qt::DisplayRole});
    emit contentChanged();
}

// ---------------------------------------------------------------- undo/redo
void SpreadsheetModel::pushUndo(UndoEntry entry) {
    m_redo.clear();
    m_undo.push_back(std::move(entry));
    if (m_undo.size() > kUndoLimit) m_undo.removeFirst();
}

void SpreadsheetModel::applyEntry(const UndoEntry &e, bool useOld) {
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

// ---------------------------------------------------------------- header / flags / kích thước
QVariant SpreadsheetModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole) return {};
    return orientation == Qt::Horizontal ? QVariant(columnLabel(section)) : QVariant(section + 1);
}

Qt::ItemFlags SpreadsheetModel::flags(const QModelIndex &index) const {
    if (!index.isValid()) return Qt::NoItemFlags;
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
}

void SpreadsheetModel::resizeGrid(int rows, int cols) {
    beginResetModel();
    m_data.resize(rows);
    for (auto &row : m_data) row.resize(cols);
    m_evalCache.clear();
    rebuildDeps();
    endResetModel();
}

QString SpreadsheetModel::columnLabel(int col) {
    QString s; int n = col;
    do { s.prepend(QChar('A' + (n % 26))); n = n / 26 - 1; } while (n >= 0);
    return s;
}
