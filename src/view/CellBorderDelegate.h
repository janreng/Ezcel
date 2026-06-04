#pragma once
#include <QStyledItemDelegate>
#include <QStringList>

class QAbstractItemView;

// Vẽ viền xanh 2px (#217346) quanh ô ĐANG CHỌN — dấu hiệu đặc trưng của Excel.
// Kèm popup gợi ý tên hàm khi gõ '=' trong ô (Spec 12).
class CellBorderDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit CellBorderDelegate(QAbstractItemView *view, QObject *parent = nullptr);
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;
    void setFunctionNames(const QStringList &names) { m_fnNames = names; }

private:
    QAbstractItemView *m_view;
    QStringList m_fnNames;
};
