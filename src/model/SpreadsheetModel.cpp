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

    if (raw.startsWith(QLatin1Char('='))) {
        const qint64 k = key(index.row(), index.column());
        auto it = m_evalCache.constFind(k);
        if (it != m_evalCache.constEnd()) return it.value();
        QVariant v = formula::evaluate(raw, [this](int r, int c) -> QVariant {
            if (r < 0 || c < 0 || r >= m_data.size() || c >= m_data.first().size()) return {};
            return m_data[r][c];
        });
        m_evalCache.insert(k, v);
        return v;
    }
    return raw;
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
