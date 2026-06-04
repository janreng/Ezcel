#include "view/CellBorderDelegate.h"
#include "view/FormulaHint.h"
#include "view/TableFilter.h"
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

    // Chấm bộ biểu tượng (Icon Set) ở mép TRÁI ô — số căn phải nên không đè chữ.
    const QString iconColor = index.data(SpreadsheetModel::IconSetRole).toString();
    if (!iconColor.isEmpty()) {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);
        const int d = qMin(option.rect.height() - 6, 12);
        if (d > 2) {
            const int x = option.rect.left() + 3;
            const int y = option.rect.top() + (option.rect.height() - d) / 2;
            painter->setPen(Qt::NoPen);
            painter->setBrush(QColor(iconColor));
            painter->drawEllipse(QRect(x, y, d, d));
        }
        painter->restore();
    }

    // Sparkline (biểu đồ mini) trong ô đích.
    const QVariant spv = index.data(SpreadsheetModel::SparkLineRole);
    if (spv.typeId() == QMetaType::QVariantList) {
        const QVariantList l = spv.toList();
        if (l.size() >= 2) {
            const int type = l[0].toInt();
            QVector<double> vals;
            for (int i = 1; i < l.size(); ++i) vals << l[i].toDouble();
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing, true);
            const QRectF rc = option.rect;
            const double margin = 2.0;
            const QColor sparkColor(QStringLiteral("#2C7BE5"));
            if (type == int(sparkline::Type::Line)) {
                const auto pts = sparkline::linePoints(vals, rc.width(), rc.height(), margin);
                QPolygonF poly;
                for (const QPointF &p : pts) poly << QPointF(rc.left() + p.x(), rc.top() + p.y());
                QPen pen(sparkColor); pen.setWidthF(1.2);
                painter->setPen(pen); painter->setBrush(Qt::NoBrush);
                painter->drawPolyline(poly);
            } else {
                const auto hs = sparkline::columnHeights(vals, rc.height(), margin);
                const int n = hs.size();
                const double innerW = rc.width() - 2 * margin;
                const double slot = n > 0 ? innerW / n : 0;
                const double bw = slot * 0.7;
                painter->setPen(Qt::NoPen); painter->setBrush(sparkColor);
                for (int i = 0; i < n; ++i) {
                    const double x = rc.left() + margin + slot * i + (slot - bw) / 2;
                    const double y = rc.bottom() - margin - hs[i];
                    painter->drawRect(QRectF(x, y, bw, hs[i]));
                }
            }
            painter->restore();
        }
    }

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

    // Nút lọc ▼ ở mép phải ô tiêu đề bảng (chữ trắng trên nền đậm -> dùng mũi tên trắng).
    if (index.data(SpreadsheetModel::TableHeaderRole).toBool()) {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);
        const QRect rc = option.rect;
        const int bx = rc.right() - tablefilter::arrowZone() + 4;
        const int by = rc.center().y() - 1;
        const QPointF tri[3] = { QPointF(bx, by), QPointF(bx + 8, by), QPointF(bx + 4, by + 5) };
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(Qt::white));
        painter->drawPolygon(tri, 3);
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
