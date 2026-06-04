#include "view/CellBorderDelegate.h"
#include "view/FormulaHint.h"
#include "ui/Theme.h"
#include "model/SpreadsheetModel.h" // chỉ dùng hằng SpillEdgesRole

#include <QAbstractItemView>
#include <QPainter>
#include <QLineEdit>
#include <QApplication>
#include <QFocusEvent>

CellBorderDelegate::CellBorderDelegate(QAbstractItemView *view, QObject *parent)
    : QStyledItemDelegate(parent), m_view(view) {}

QWidget *CellBorderDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                                          const QModelIndex &index) const
{
    QWidget *editor = QStyledItemDelegate::createEditor(parent, option, index);
    if (!m_fnNames.isEmpty())
        if (auto *le = qobject_cast<QLineEdit *>(editor))
            formulahint::install(le, m_fnNames, m_fnSigs); // popup gợi ý hàm + tooltip tham số
    // Theo dõi editor đang mở để hỗ trợ "point mode" (click ô chèn địa chỉ vào công thức).
    m_activeEditor = editor;
    QObject::connect(editor, &QObject::destroyed, this, [this](QObject *o) {
        if (m_activeEditor == o) m_activeEditor = nullptr;
    });
    return editor;
}

bool CellBorderDelegate::eventFilter(QObject *obj, QEvent *event)
{
    // "Point mode": đang sửa công thức (=...) mà bấm vào lưới -> đừng commit dở dang.
    // Chỉ chặn khi focus mới rơi vào chính bảng (view/viewport); click ra ngoài (thanh
    // công thức, app khác) vẫn commit như thường.
    if (event->type() == QEvent::FocusOut) {
        if (auto *le = qobject_cast<QLineEdit *>(obj)) {
            if (m_view && le->text().startsWith(QLatin1Char('='))) {
                const QWidget *fw = QApplication::focusWidget();
                if (fw == m_view || fw == m_view->viewport())
                    return true; // nuốt FocusOut -> không commit, giữ editor mở
            }
        }
    }
    return QStyledItemDelegate::eventFilter(obj, event);
}

void CellBorderDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                               const QModelIndex &index) const
{
    // Thanh dữ liệu (Data Bar): vẽ TRƯỚC nội dung để chữ nằm trên thanh.
    const QVariant dbv = index.data(SpreadsheetModel::DataBarRole);
    if (dbv.typeId() == QMetaType::QVariantList) {
        const QVariantList db = dbv.toList();
        if (db.size() == 2) {
            const double frac = db[0].toDouble();
            QColor barColor(db[1].toString());
            if (barColor.isValid() && frac > 0.0) {
                painter->save();
                const QRect rc = option.rect.adjusted(1, 1, -1, -1);
                const int w = int(rc.width() * frac);
                painter->fillRect(QRect(rc.left(), rc.top(), w, rc.height()), barColor);
                painter->restore();
            }
        }
    }

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

    // Viền nét đứt xanh quanh vùng tràn (spill) — chỉ vẽ ở các cạnh BIÊN của vùng.
    const int spillEdges = index.data(SpreadsheetModel::SpillEdgesRole).toInt();
    if (spillEdges) {
        painter->save();
        QPen dash(QColor(theme::SelectionBorder));
        dash.setStyle(Qt::DashLine);
        dash.setWidth(1);
        painter->setPen(dash);
        const QRect rc = option.rect.adjusted(0, 0, -1, -1);
        if (spillEdges & 1) painter->drawLine(rc.topLeft(), rc.topRight());       // trên
        if (spillEdges & 2) painter->drawLine(rc.topLeft(), rc.bottomLeft());     // trái
        if (spillEdges & 4) painter->drawLine(rc.bottomLeft(), rc.bottomRight()); // dưới
        if (spillEdges & 8) painter->drawLine(rc.topRight(), rc.bottomRight());   // phải
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
