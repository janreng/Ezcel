#include "model/SpreadsheetModel.h"
#include "formula/Formula.h"

SpreadsheetModel::SpreadsheetModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

int SpreadsheetModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    return m_data.size();
}

int SpreadsheetModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    return m_data.isEmpty() ? 0 : m_data.first().size();
}

QVariant SpreadsheetModel::data(const QModelIndex &index, int role) const
{
    // HOT PATH — giữ tối thiểu cấp phát (xem nguyên tắc hiệu năng trong CLAUDE.md gốc).
    if (!index.isValid()) return {};
    if (role != Qt::DisplayRole && role != Qt::EditRole) return {};

    const QString &raw = m_data[index.row()][index.column()];
    if (role == Qt::EditRole) return raw; // sửa: hiện công thức thô

    if (formula::isFormula(raw))
        return evalCell(index.row(), index.column());
    return raw;
}

QVariant SpreadsheetModel::evalCell(int row, int col) const
{
    if (row < 0 || col < 0 || row >= m_data.size()
        || (m_data.isEmpty() ? true : col >= m_data.first().size()))
        return {};

    const QString &raw = m_data[row][col];
    if (!formula::isFormula(raw))
        return raw; // ô thường: trả chuỗi/số thô

    const qint64 k = key(row, col);
    auto it = m_evalCache.constFind(k);
    if (it != m_evalCache.constEnd()) return it.value();

    if (m_evaluating.contains(k)) // tham chiếu vòng
        return QString::fromLatin1(formula::ERR_REF);

    m_evaluating.insert(k);
    QVariant result;
    try {
        result = formula::evaluate(raw, [this](int r, int c) { return evalCell(r, c); });
    } catch (const formula::FormulaError &e) {
        result = e.etype(); // hiện mã lỗi kiểu Excel (#VALUE!, #DIV/0!...)
    }
    m_evaluating.remove(k);
    m_evalCache.insert(k, result);
    return result;
}

bool SpreadsheetModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || role != Qt::EditRole) return false;
    // P1: _push_undo() ở đầu, vô hiệu hóa cache chọn lọc theo đồ thị phụ thuộc.
    m_data[index.row()][index.column()] = value.toString();
    m_evalCache.clear(); // P1 sẽ thay bằng vô hiệu hóa cục bộ
    emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});
    return true;
}

QVariant SpreadsheetModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) return {};
    if (orientation == Qt::Horizontal) return columnLabel(section);
    return section + 1;
}

Qt::ItemFlags SpreadsheetModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) return Qt::NoItemFlags;
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
}

void SpreadsheetModel::resizeGrid(int rows, int cols)
{
    beginResetModel();
    m_data.resize(rows);
    for (auto &row : m_data) row.resize(cols);
    m_evalCache.clear();
    endResetModel();
}

QString SpreadsheetModel::columnLabel(int col)
{
    QString s;
    int n = col;
    do {
        s.prepend(QChar('A' + (n % 26)));
        n = n / 26 - 1;
    } while (n >= 0);
    return s;
}
