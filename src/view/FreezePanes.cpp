#include "view/FreezePanes.h"
#include <QTableView>
#include <QHeaderView>
#include <QScrollBar>
#include <QEvent>
#include <QAbstractItemModel>
#include <QItemSelectionModel>

namespace freeze {

FreezePanes::FreezePanes(QTableView *main, QObject *parent)
    : QObject(parent), m_main(main)
{
    m_main->installEventFilter(this);
    // Cuộn / đổi kích thước cột-hàng ở bảng chính -> cập nhật lại các bảng phủ.
    connect(m_main->horizontalScrollBar(), &QScrollBar::valueChanged, this, [this] { relayout(); });
    connect(m_main->verticalScrollBar(), &QScrollBar::valueChanged, this, [this] { relayout(); });
    connect(m_main->horizontalHeader(), &QHeaderView::sectionResized, this, [this] { relayout(); });
    connect(m_main->verticalHeader(), &QHeaderView::sectionResized, this, [this] { relayout(); });
}

QTableView *FreezePanes::makeOverlay()
{
    auto *v = new QTableView(m_main);
    v->setModel(m_main->model());
    v->setSelectionModel(m_main->selectionModel());
    v->setFocusPolicy(Qt::NoFocus);
    v->setFrameShape(QFrame::NoFrame);
    v->horizontalHeader()->hide();
    v->verticalHeader()->hide();
    v->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    v->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    v->setStyleSheet(m_main->styleSheet());
    v->setFont(m_main->font());
    v->setShowGrid(m_main->showGrid());
    v->setItemDelegate(m_main->itemDelegate());
    v->show();
    return v;
}

void FreezePanes::destroyOverlays()
{
    delete m_top; m_top = nullptr;
    delete m_left; m_left = nullptr;
    delete m_corner; m_corner = nullptr;
}

void FreezePanes::apply(int rows, int cols)
{
    m_rows = qMax(0, rows);
    m_cols = qMax(0, cols);
    destroyOverlays();
    if (m_rows == 0 && m_cols == 0) return;

    if (m_rows > 0) m_top = makeOverlay();
    if (m_cols > 0) m_left = makeOverlay();
    if (m_rows > 0 && m_cols > 0) m_corner = makeOverlay();

    // Mỗi bảng phủ chỉ hiện phần cần: ẩn các hàng/cột không thuộc vùng cố định.
    const int rc = m_main->model() ? m_main->model()->rowCount() : 0;
    const int cc = m_main->model() ? m_main->model()->columnCount() : 0;
    if (m_top) for (int r = m_rows; r < rc; ++r) m_top->setRowHidden(r, true);
    if (m_left) for (int c = m_cols; c < cc; ++c) m_left->setColumnHidden(c, true);
    if (m_corner) {
        for (int r = m_rows; r < rc; ++r) m_corner->setRowHidden(r, true);
        for (int c = m_cols; c < cc; ++c) m_corner->setColumnHidden(c, true);
    }
    relayout();
}

void FreezePanes::rebind(QAbstractItemModel *model, QItemSelectionModel *sel)
{
    const int r = m_rows, c = m_cols;
    apply(0, 0);          // bỏ overlay cũ (gắn model cũ)
    if (r > 0 || c > 0) { Q_UNUSED(model); Q_UNUSED(sel); apply(r, c); } // dựng lại với model mới
}

void FreezePanes::syncSections()
{
    auto copyH = [this](QTableView *v) {
        if (!v) return;
        for (int c = 0; c < m_main->model()->columnCount(); ++c)
            v->setColumnWidth(c, m_main->columnWidth(c));
    };
    auto copyV = [this](QTableView *v) {
        if (!v) return;
        for (int r = 0; r < m_main->model()->rowCount(); ++r)
            v->setRowHeight(r, m_main->rowHeight(r));
    };
    copyH(m_top); copyH(m_left); copyH(m_corner);
    copyV(m_top); copyV(m_left); copyV(m_corner);
}

void FreezePanes::relayout()
{
    if (!m_main->model() || (m_rows == 0 && m_cols == 0)) return;
    syncSections();

    const int ox = m_main->verticalHeader()->isVisible() ? m_main->verticalHeader()->width() : 0;
    const int oy = m_main->horizontalHeader()->isVisible() ? m_main->horizontalHeader()->height() : 0;
    int fw = 0; for (int c = 0; c < m_cols; ++c) fw += m_main->columnWidth(c);
    int fh = 0; for (int r = 0; r < m_rows; ++r) fh += m_main->rowHeight(r);
    const int vw = m_main->viewport()->width();
    const int vh = m_main->viewport()->height();

    if (m_top) {
        m_top->setGeometry(ox, oy, vw, fh);
        m_top->horizontalScrollBar()->setValue(m_main->horizontalScrollBar()->value());
        m_top->verticalScrollBar()->setValue(0);
        m_top->raise();
    }
    if (m_left) {
        m_left->setGeometry(ox, oy, fw, vh);
        m_left->verticalScrollBar()->setValue(m_main->verticalScrollBar()->value());
        m_left->horizontalScrollBar()->setValue(0);
        m_left->raise();
    }
    if (m_corner) {
        m_corner->setGeometry(ox, oy, fw, fh);
        m_corner->horizontalScrollBar()->setValue(0);
        m_corner->verticalScrollBar()->setValue(0);
        m_corner->raise();
    }
}

bool FreezePanes::eventFilter(QObject *obj, QEvent *ev)
{
    if (obj == m_main && (ev->type() == QEvent::Resize || ev->type() == QEvent::Show))
        relayout();
    return QObject::eventFilter(obj, ev);
}

} // namespace freeze
