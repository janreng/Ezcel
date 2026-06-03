#pragma once
#include <QAbstractTableModel>
#include <QVector>
#include <QString>
#include <QHash>

// Bản port của table_model.py — SpreadsheetModel(QAbstractTableModel).
// P1 sẽ bổ sung: _eval_cache theo đồ thị phụ thuộc, _fmt định dạng ô,
// undo/redo snapshot, copy/paste, autofill, sort, move_row/move_column.
class SpreadsheetModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit SpreadsheetModel(QObject *parent = nullptr);

    // QAbstractTableModel
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    void resizeGrid(int rows, int cols);

    // Nhãn cột kiểu Excel: 0->A, 25->Z, 26->AA...
    static QString columnLabel(int col);

private:
    QVector<QVector<QString>> m_data;            // lưới thô (chuỗi/công thức)
    mutable QHash<qint64, QVariant> m_evalCache; // giá trị công thức đã tính (P1: vô hiệu hóa chọn lọc)

    static qint64 key(int row, int col) { return (qint64(row) << 32) | quint32(col); }
};
