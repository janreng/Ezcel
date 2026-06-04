#include "view/CellBorderDelegate.h"
#include "view/FormulaHint.h"
#include "ui/Theme.h"

#include <QAbstractItemView>
#include <QPainter>
#include <QLineEdit>

CellBorderDelegate::CellBorderDelegate(QAbstractItemView *view, QObject *parent)
    : QStyledItemDelegate(parent), m_view(view) {}

QWidget *CellBorderDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                                          const QModelIndex &index) const
{
    QWidget *editor = QStyledItemDelegate::createEditor(parent, option, index);
    if (!m_fnNames.isEmpty())
        if (auto *le = qobject_cast<QLineEdit *>(editor))
            formulahint::install(le, m_fnNames, m_fnSigs); // popup gợi ý hàm + tooltip tham số
    return editor;
}

void CellBorderDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                               const QModelIndex &index) const
{
    QStyledItemDelegate::paint(painter, option, index);

    // Dấu tam giác đỏ góc trên-phải nếu ô có ghi chú (note).
    if (!index.data(Qt::ToolTipRole).toString().isEmpty()) {
        painter->save();
        QPointF tip[3] = {
            QPointF(option.rect.right() - 6, option.rect.top() + 1),
            QPointF(option.rect.right() - 1, option.rect.top() + 1),
            QPointF(option.rect.right() - 1, option.rect.top() + 6),
        };
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor("#C0392B"));
        painter->drawPolygon(tip, 3);
        painter->restore();
    }

    if (!m_view || index != m_view->currentIndex()) return;

    painter->save();
    QPen pen(QColor(theme::SelectionBorder));
    pen.setWidth(2);
    pen.setJoinStyle(Qt::MiterJoin);
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    // Lùi 1px để viền 2px nằm gọn trong ô, không bị xén.
    painter->drawRect(option.rect.adjusted(1, 1, -1, -1));
    painter->restore();
}
