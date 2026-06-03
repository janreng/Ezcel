#pragma once
#include <QStyledItemDelegate>

class QAbstractItemView;

// Vẽ viền xanh 2px (#217346) quanh ô ĐANG CHỌN — dấu hiệu đặc trưng của Excel.
class CellBorderDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit CellBorderDelegate(QAbstractItemView *view, QObject *parent = nullptr);
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;

private:
    QAbstractItemView *m_view;
};
