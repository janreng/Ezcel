#include "MainWindow.h"
#include "model/SpreadsheetModel.h"
#include "io/Csv.h"
#include "io/Xlsx.h"
#include "view/MergeSpans.h"
#include "view/CellBorderDelegate.h"
#include "view/Visibility.h"
#include "view/HeaderMenu.h"
#include "view/Outline.h"
#include "view/CopyVisible.h"
#include "model/RefCycle.h"
#include "model/NameValidate.h"
#include "model/RangeParse.h"
#include "model/SeriesGen.h"
#include "model/TextCase.h"
#include "model/Filter.h"
#include "model/PasteOps.h"
#include "model/GotoSpecial.h"
#include "ui/I18n.h"
#include "update/Updater.h"
#include "ui/Theme.h"
#include "model/Stats.h"
#include "model/AutoSum.h"
#include "ui/Shortcuts.h"
#include "ui/Zoom.h"
#include "ui/SheetNav.h"
#include "ui/CellMode.h"
#include "ui/WorkbookStats.h"
#include "model/FlashFill.h"
#include "model/AutoComplete.h"
#include "model/DataTools.h"

#include <QTableView>
#include <QHeaderView>
#include <QLineEdit>
#include <QLabel>
#include <QWidget>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMenuBar>
#include <QMenu>
#include <QToolBar>
#include <QStatusBar>
#include <QAction>
#include <climits>
#include <QFileDialog>
#include <QMessageBox>
#include <QDialog>
#include <QListWidget>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QDialogButtonBox>
#include <QRadioButton>
#include <QComboBox>
#include <QCheckBox>
#include <QGroupBox>
#include <QButtonGroup>
#include <QAbstractItemView>
#include <QSlider>
#include <QToolButton>
#include <QShortcut>
#include <QColorDialog>
#include <QInputDialog>
#include <QApplication>
#include <QClipboard>
#include <QFileInfo>
#include <QKeySequence>
#include <QItemSelectionModel>
#include <QItemSelection>
#include <QPoint>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QEvent>
#include <QDate>
#include <QTime>
#include <QTabBar>
#include <QToolButton>
#include <QIcon>
#include <QSize>

// Bộ lọc file dùng chung cho mở/lưu.
static const char *kFileFilter =
    "Bảng tính (*.xlsx *.xlsm *.csv *.txt *.tsv);;Excel (*.xlsx *.xlsm);;CSV/Text (*.csv *.txt *.tsv);;Tất cả (*)";

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    m_sheets.push_back(new SpreadsheetModel(this));
    m_sheets[0]->resizeGrid(200, 50); // lưới khởi tạo, như bản Python
    m_model = m_sheets[0];

    m_view = new QTableView(this);
    m_view->setModel(m_model);
    m_view->horizontalHeader()->setDefaultSectionSize(theme::ColWidth);
    m_view->verticalHeader()->setDefaultSectionSize(theme::RowHeight);
    m_view->setStyleSheet(theme::tableStyle()); // bảng màu kiểu Excel
    m_view->setFont(theme::cellFont());         // font nội dung ô (Aptos Narrow 11)
    m_view->setShowGrid(true);
    m_view->horizontalHeader()->setHighlightSections(true); // header sáng khi chọn (giống Excel)
    m_view->verticalHeader()->setHighlightSections(true);
    m_view->setItemDelegate(new CellBorderDelegate(m_view, m_view)); // viền xanh ô đang chọn
    // Gõ phím là vào chế độ sửa ngay (giống Excel); Enter sau đó tự nhảy xuống ô dưới.
    m_view->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed
                            | QAbstractItemView::AnyKeyPressed);
    m_view->setTabKeyNavigation(true); // Tab sang phải, Enter xuống dưới
    m_view->viewport()->installEventFilter(this); // bắt Ctrl+wheel để zoom
    // Nút kéo điền (fill handle): ô vuông xanh nhỏ ở góc dưới-phải vùng chọn (Spec 05).
    m_fillHandle = new QWidget(m_view->viewport());
    m_fillHandle->resize(8, 8);
    m_fillHandle->setStyleSheet(QStringLiteral("background:#217346;border:1px solid white;"));
    m_fillHandle->setCursor(Qt::CrossCursor);
    m_fillHandle->hide();
    m_fillHandle->installEventFilter(this);
    connect(m_view->verticalScrollBar(), &QScrollBar::valueChanged, this, [this] { positionFillHandle(); });
    connect(m_view->horizontalScrollBar(), &QScrollBar::valueChanged, this, [this] { positionFillHandle(); });
    buildContextMenus(); // menu chuột phải: ô + đầu hàng/cột (Spec 06)

    buildFormulaBar();

    // Thanh tab trang tính ở dưới cùng + nút thêm trang.
    auto *tabRow = new QWidget(this);
    auto *tabLay = new QHBoxLayout(tabRow);
    tabLay->setContentsMargins(2, 0, 2, 0);
    tabLay->setSpacing(2);
    m_sheetTabs = new QTabBar(tabRow);
    m_sheetTabs->setExpanding(false);
    m_sheetTabs->setTabsClosable(true);
    m_sheetTabs->addTab(QStringLiteral("Trang 1"));
    auto *addSheetBtn = new QToolButton(tabRow);
    addSheetBtn->setText(QStringLiteral("+"));
    addSheetBtn->setToolTip(QStringLiteral("Thêm trang tính"));
    tabLay->addWidget(m_sheetTabs);
    tabLay->addWidget(addSheetBtn);
    tabLay->addStretch();
    connect(addSheetBtn, &QToolButton::clicked, this, [this] { addSheet(); });
    connect(m_sheetTabs, &QTabBar::currentChanged, this, &MainWindow::switchToSheet);
    connect(m_sheetTabs, &QTabBar::tabBarDoubleClicked, this, [this](int i) { renameSheet(i); });
    connect(m_sheetTabs, &QTabBar::tabCloseRequested, this, &MainWindow::closeSheet);

    // Menu chuột phải trên tab: Đổi tên / Màu tab / Xóa (Spec 10).
    m_sheetTabs->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_sheetTabs, &QTabBar::customContextMenuRequested, this, [this](const QPoint &p) {
        int i = m_sheetTabs->tabAt(p);
        if (i < 0) return;
        QMenu menu(this);
        menu.addAction(QStringLiteral("Đổi tên…"), this, [this, i] { renameSheet(i); });
        menu.addAction(QStringLiteral("Màu tab…"), this, [this, i] {
            QColor c = QColorDialog::getColor(m_sheetTabs->tabTextColor(i), this, QStringLiteral("Màu tab"));
            if (c.isValid()) m_sheetTabs->setTabTextColor(i, c);
        });
        menu.addSeparator();
        QAction *del = menu.addAction(QStringLiteral("Xóa trang"), this, [this, i] { closeSheet(i); });
        del->setEnabled(m_sheets.size() > 1);
        menu.exec(m_sheetTabs->mapToGlobal(p));
    });

    // Ctrl+PageDown / Ctrl+PageUp: chuyển trang sau / trước (quay vòng).
    auto *nextSh = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_PageDown), this);
    connect(nextSh, &QShortcut::activated, this, [this] { gotoSheetRelative(1); });
    auto *prevSh = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_PageUp), this);
    connect(prevSh, &QShortcut::activated, this, [this] { gotoSheetRelative(-1); });
    // Alt+Down: chọn từ danh sách giá trị trong cột (Spec 05).
    auto *pickSh = new QShortcut(QKeySequence(Qt::ALT | Qt::Key_Down), this);
    connect(pickSh, &QShortcut::activated, this, [this] { pickFromList(); });

    auto *central = new QWidget(this);
    auto *lay = new QVBoxLayout(central);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(0);
    lay->addWidget(m_formulaBar->parentWidget()); // dải thanh công thức
    lay->addWidget(m_view);
    lay->addWidget(tabRow);
    setCentralWidget(central);

    buildMenus();
    buildToolbar();
    buildFormatToolbar();
    m_modeLabel = new QLabel(this);
    m_modeLabel->setMinimumWidth(70);
    statusBar()->addWidget(m_modeLabel); // bên trái thanh trạng thái
    setCellMode(int(cellmode::Mode::Ready));
    m_statsLabel = new QLabel(this);
    statusBar()->addPermanentWidget(m_statsLabel);
    buildStatusBarZoom();

    bindActiveModel();
    updateTitle();
}

MainWindow::~MainWindow() = default;

// ---------------------------------------------------------------- nhiều trang tính
void MainWindow::bindActiveModel()
{
    for (const auto &c : m_modelConns) disconnect(c);
    m_modelConns.clear();
    m_modelConns << connect(m_view->selectionModel(), &QItemSelectionModel::currentChanged,
                            this, &MainWindow::onCurrentCellChanged);
    m_modelConns << connect(m_view->selectionModel(), &QItemSelectionModel::selectionChanged,
                            this, [this] { updateStats(); positionFillHandle(); });
    m_modelConns << connect(m_model, &SpreadsheetModel::contentChanged, this, [this] {
        onCurrentCellChanged(m_view->currentIndex(), QModelIndex());
    });
    m_modelConns << connect(m_model, &SpreadsheetModel::mergesChanged, this, [this] {
        viewutil::applyMergeSpans(m_view, m_model->merges());
    });
    m_modelConns << connect(m_model, &SpreadsheetModel::validationFailed, this, [this](const QString &msg) {
        statusBar()->showMessage(msg, 4000);
    });
    viewutil::applyMergeSpans(m_view, m_model->merges());
}

void MainWindow::addSheet(const QString &name)
{
    auto *m = new SpreadsheetModel(this);
    m->resizeGrid(200, 50);
    m_sheets.push_back(m);
    const QString nm = name.isEmpty() ? QStringLiteral("Trang %1").arg(m_sheets.size()) : name;
    m_sheetTabs->addTab(nm);
    m_sheetTabs->setCurrentIndex(m_sheets.size() - 1); // -> switchToSheet
}

void MainWindow::switchToSheet(int i)
{
    if (i < 0 || i >= m_sheets.size()) return;
    m_model = m_sheets[i];
    m_view->setModel(m_model);
    bindActiveModel();
    onCurrentCellChanged(m_view->currentIndex(), QModelIndex());
    updateStats();
}

// Chuyển trang theo bước (Ctrl+PageDown/Up), quay vòng — dùng sheetnav::wrapIndex.
void MainWindow::gotoSheetRelative(int delta)
{
    int next = sheetnav::wrapIndex(m_sheetTabs->currentIndex(), m_sheets.size(), delta);
    m_sheetTabs->setCurrentIndex(next); // kích hoạt switchToSheet
}

// Đổi tên trang (double-click hoặc menu chuột phải).
void MainWindow::renameSheet(int i)
{
    if (i < 0 || i >= m_sheets.size()) return;
    bool ok = false;
    QString n = QInputDialog::getText(this, QStringLiteral("Đổi tên trang"),
        QStringLiteral("Tên trang:"), QLineEdit::Normal, m_sheetTabs->tabText(i), &ok);
    if (ok && !n.isEmpty()) m_sheetTabs->setTabText(i, n);
}

// Xóa trang (giữ tối thiểu 1 trang).
void MainWindow::closeSheet(int i)
{
    if (i < 0 || i >= m_sheets.size() || m_sheets.size() <= 1) return;
    m_sheetTabs->blockSignals(true);
    m_sheets[i]->deleteLater();
    m_sheets.remove(i);
    m_sheetTabs->removeTab(i);
    int cur = qBound(0, m_sheetTabs->currentIndex(), m_sheets.size() - 1);
    m_sheetTabs->setCurrentIndex(cur);
    m_sheetTabs->blockSignals(false);
    switchToSheet(cur);
}

// -------------------------------------------------- menu chuột phải (Spec 06)
// Cờ ẩn của các section trên một header (để quyết định bật "Hiện lại").
static QVector<bool> hiddenFlags(QHeaderView *hdr, int count)
{
    QVector<bool> f(count, false);
    for (int i = 0; i < count; ++i) f[i] = hdr->isSectionHidden(i);
    return f;
}

void MainWindow::buildContextMenus()
{
    // --- Menu trên ô (vùng dữ liệu) ---
    m_view->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_view, &QTableView::customContextMenuRequested, this, [this](const QPoint &p) {
        // Nếu phải chuột ngoài vùng chọn -> chuyển ô hiện hành tới ô bị bấm.
        QModelIndex at = m_view->indexAt(p);
        if (at.isValid() && !m_view->selectionModel()->isSelected(at))
            m_view->setCurrentIndex(at);
        QMenu menu(this);
        menu.addAction(i18n::tr("ctx_cut"), this, &MainWindow::cutSelection);
        menu.addAction(i18n::tr("ctx_copy"), this, &MainWindow::copySelection);
        menu.addAction(i18n::tr("ctx_paste"), this, &MainWindow::pasteClipboard);
        menu.addAction(i18n::tr("ctx_paste_special"), this, &MainWindow::pasteSpecial);
        menu.addSeparator();
        menu.addAction(i18n::tr("st_ins_row"), this, [this] {
            int t, l, b, r; if (selectionBox(t, l, b, r)) m_model->insertRows(t, b - t + 1);
        });
        menu.addAction(i18n::tr("st_ins_col"), this, [this] {
            int t, l, b, r; if (selectionBox(t, l, b, r)) m_model->insertColumns(l, r - l + 1);
        });
        menu.addAction(i18n::tr("st_del_row"), this, [this] {
            int t, l, b, r; if (selectionBox(t, l, b, r)) m_model->removeRows(t, b - t + 1);
        });
        menu.addAction(i18n::tr("st_del_col"), this, [this] {
            int t, l, b, r; if (selectionBox(t, l, b, r)) m_model->removeColumns(l, r - l + 1);
        });
        menu.addAction(i18n::tr("ctx_clear"), this, &MainWindow::clearSelection);
        menu.addSeparator();
        menu.addAction(i18n::tr("data_filter_values"), this, &MainWindow::filterByValues);
        menu.addAction(i18n::tr("data_sort_asc"), this, [this] {
            int t, l, b, r; if (!selectionBox(t, l, b, r)) return;
            int kc = m_view->currentIndex().isValid() ? m_view->currentIndex().column() : l;
            m_model->sortRange(t, l, b, r, qBound(l, kc, r), true);
        });
        menu.addAction(i18n::tr("data_sort_desc"), this, [this] {
            int t, l, b, r; if (!selectionBox(t, l, b, r)) return;
            int kc = m_view->currentIndex().isValid() ? m_view->currentIndex().column() : l;
            m_model->sortRange(t, l, b, r, qBound(l, kc, r), false);
        });
        menu.addAction(i18n::tr("ctx_pick_list"), this, &MainWindow::pickFromList);
        menu.exec(m_view->viewport()->mapToGlobal(p));
    });

    // --- Menu trên ô đầu HÀNG (vertical header) ---
    QHeaderView *vh = m_view->verticalHeader();
    vh->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(vh, &QHeaderView::customContextMenuRequested, this, [this, vh](const QPoint &p) {
        int sec = vh->logicalIndexAt(p);
        if (sec < 0) return;
        int t, l, b, r;
        if (!selectionBox(t, l, b, r) || sec < t || sec > b) { t = b = sec; }
        QMenu menu(this);
        menu.addAction(i18n::tr("hdr_insert_row"), this, [this, t, b] { m_model->insertRows(t, b - t + 1); });
        menu.addAction(i18n::tr("hdr_delete_row"), this, [this, t, b] { m_model->removeRows(t, b - t + 1); });
        menu.addAction(i18n::tr("ctx_clear"), this, &MainWindow::clearSelection);
        menu.addSeparator();
        menu.addAction(i18n::tr("hdr_row_height"), this, [this, t, b] {
            bool ok = false;
            int cur = m_view->rowHeight(t);
            int h = QInputDialog::getInt(this, i18n::tr("hdr_row_height"),
                                         i18n::tr("hdr_row_height_prompt"), cur, 1, 2000, 1, &ok);
            if (ok) for (int row = t; row <= b; ++row) m_view->setRowHeight(row, h);
        });
        menu.addAction(i18n::tr("hdr_autofit_row"), this, [this, t, b] {
            for (int row = t; row <= b; ++row) m_view->resizeRowToContents(row);
        });
        menu.addSeparator();
        menu.addAction(i18n::tr("hdr_hide_row"), this, [this, t, b] { viewutil::hideRows(m_view, t, b); });
        QAction *un = menu.addAction(i18n::tr("hdr_unhide_row"), this, [this, t, b] {
            viewutil::unhideRange(m_view, t, 0, b, 0);
        });
        un->setEnabled(headermenu::canUnhide(hiddenFlags(vh, m_model->rowCount()), t, b));
        menu.exec(vh->mapToGlobal(p));
    });

    // --- Menu trên ô đầu CỘT (horizontal header) ---
    QHeaderView *hh = m_view->horizontalHeader();
    hh->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(hh, &QHeaderView::customContextMenuRequested, this, [this, hh](const QPoint &p) {
        int sec = hh->logicalIndexAt(p);
        if (sec < 0) return;
        int t, l, b, r;
        if (!selectionBox(t, l, b, r) || sec < l || sec > r) { l = r = sec; }
        QMenu menu(this);
        menu.addAction(i18n::tr("hdr_insert_col"), this, [this, l, r] { m_model->insertColumns(l, r - l + 1); });
        menu.addAction(i18n::tr("hdr_delete_col"), this, [this, l, r] { m_model->removeColumns(l, r - l + 1); });
        menu.addAction(i18n::tr("ctx_clear"), this, &MainWindow::clearSelection);
        menu.addSeparator();
        menu.addAction(i18n::tr("hdr_col_width"), this, [this, l, r] {
            bool ok = false;
            int cur = m_view->columnWidth(l);
            int w = QInputDialog::getInt(this, i18n::tr("hdr_col_width"),
                                         i18n::tr("hdr_col_width_prompt"), cur, 1, 4000, 1, &ok);
            if (ok) for (int c = l; c <= r; ++c) m_view->setColumnWidth(c, w);
        });
        menu.addAction(i18n::tr("hdr_autofit_col"), this, [this, l, r] {
            for (int c = l; c <= r; ++c) m_view->resizeColumnToContents(c);
        });
        menu.addSeparator();
        menu.addAction(i18n::tr("hdr_hide_col"), this, [this, l, r] { viewutil::hideCols(m_view, l, r); });
        QAction *un = menu.addAction(i18n::tr("hdr_unhide_col"), this, [this, l, r] {
            viewutil::unhideRange(m_view, 0, l, 0, r);
        });
        un->setEnabled(headermenu::canUnhide(hiddenFlags(hh, m_model->columnCount()), l, r));
        menu.exec(hh->mapToGlobal(p));
    });
}

// ---------------------------------------------------------------- thanh công thức
void MainWindow::buildFormulaBar()
{
    auto *bar = new QWidget(this);
    auto *h = new QHBoxLayout(bar);
    h->setContentsMargins(4, 2, 4, 2);
    h->setSpacing(6);
    // Name Box (ô địa chỉ A1) bên trái — giống Excel.
    m_nameBox = new QLineEdit(bar);
    m_nameBox->setFixedWidth(80);
    m_nameBox->setToolTip(QStringLiteral("Ô tên: gõ địa chỉ (vd A1, B2:C5) rồi Enter để nhảy"));
    auto *fx = new QLabel(QStringLiteral("fx"), bar);
    fx->setStyleSheet("font-style: italic; color: #666;");
    m_formulaBar = new QLineEdit(bar);
    m_formulaBar->setPlaceholderText(QStringLiteral("Nội dung / công thức ô đang chọn"));
    m_formulaBar->installEventFilter(this); // bắt F4 để đảo khóa $ tham chiếu (Spec 04)
    h->addWidget(m_nameBox);
    h->addWidget(fx);
    h->addWidget(m_formulaBar, 1);
    connect(m_formulaBar, &QLineEdit::returnPressed, this, &MainWindow::onFormulaBarCommitted);
    connect(m_nameBox, &QLineEdit::returnPressed, this, &MainWindow::onNameBoxCommitted);
    // Chế độ ô: gõ vào thanh công thức -> "Nhập"; xong/đổi ô -> "Sẵn sàng".
    connect(m_formulaBar, &QLineEdit::textEdited, this, [this] { setCellMode(int(cellmode::Mode::Enter)); });
}

void MainWindow::setCellMode(int mode)
{
    if (m_modeLabel) m_modeLabel->setText(cellmode::label(cellmode::Mode(mode)));
}

void MainWindow::onCurrentCellChanged(const QModelIndex &cur, const QModelIndex &)
{
    setCellMode(int(cellmode::Mode::Ready)); // đổi ô -> về trạng thái sẵn sàng
    if (cur.isValid() && m_nameBox && !m_nameBox->hasFocus())
        m_nameBox->setText(SpreadsheetModel::columnLabel(cur.column()) + QString::number(cur.row() + 1));
    if (m_formulaBar->hasFocus()) return; // đang gõ, đừng đè
    if (cur.isValid())
        m_formulaBar->setText(m_model->data(cur, Qt::EditRole).toString());
    else
        m_formulaBar->clear();
    positionFillHandle();
}

// Đặt nút kéo điền ở góc dưới-phải ô cuối của vùng chọn (Spec 05).
void MainWindow::positionFillHandle()
{
    if (!m_fillHandle || m_filling) return;
    int t, l, b, r;
    if (!selectionBox(t, l, b, r)) { m_fillHandle->hide(); return; }
    const QModelIndex br = m_model->index(b, r);
    if (m_view->isRowHidden(b) || m_view->isColumnHidden(r)) { m_fillHandle->hide(); return; }
    const QRect rect = m_view->visualRect(br);
    if (!rect.isValid() || rect.isEmpty()) { m_fillHandle->hide(); return; }
    const int sz = m_fillHandle->width();
    m_fillHandle->move(rect.right() - sz / 2, rect.bottom() - sz / 2);
    m_fillHandle->raise();
    m_fillHandle->show();
}

// Thực hiện điền khi thả chuột tại vị trí `pos` (toạ độ viewport).
void MainWindow::doFillDrag(const QPoint &pos)
{
    const QModelIndex target = m_view->indexAt(pos);
    if (!target.isValid()) return;
    const int tr = target.row(), tc = target.column();
    const int downDelta = tr - m_fillB, rightDelta = tc - m_fillR;
    if (downDelta <= 0 && rightDelta <= 0) return; // chỉ hỗ trợ kéo xuống / sang phải

    if (downDelta >= rightDelta && downDelta > 0) {
        for (int c = m_fillL; c <= m_fillR; ++c) m_model->autofillVertical(c, m_fillT, m_fillB, tr);
        QItemSelection sel(m_model->index(m_fillT, m_fillL), m_model->index(tr, m_fillR));
        m_view->selectionModel()->select(sel, QItemSelectionModel::ClearAndSelect);
    } else {
        for (int row = m_fillT; row <= m_fillB; ++row) m_model->autofillHorizontal(row, m_fillL, m_fillR, tc);
        QItemSelection sel(m_model->index(m_fillT, m_fillL), m_model->index(m_fillB, tc));
        m_view->selectionModel()->select(sel, QItemSelectionModel::ClearAndSelect);
    }
}

// "A1" -> (row,col) 0-based; trả {-1,-1} nếu sai. Phần chữ = cột, phần số = dòng.
static QPoint parseCellRef(const QString &ref)
{
    QString s = ref.trimmed().toUpper();
    int i = 0, col = 0;
    while (i < s.size() && s[i].isLetter()) { col = col * 26 + (s[i].unicode() - 'A' + 1); ++i; }
    if (i == 0 || i >= s.size()) return {-1, -1};
    bool ok = false; int row = s.mid(i).toInt(&ok);
    if (!ok || row < 1) return {-1, -1};
    return {col - 1, row - 1}; // x=col, y=row
}

void MainWindow::onNameBoxCommitted()
{
    const QString text = m_nameBox->text().trimmed();
    if (text.isEmpty()) { m_view->setFocus(); return; }

    // 1) Nếu là vùng đã đặt tên -> chọn vùng đó.
    MergeRange nr;
    if (m_model->lookupName(text, nr)) {
        QModelIndex a0 = m_model->index(nr.top, nr.left);
        QModelIndex b0 = m_model->index(qMin(nr.bottom, m_model->rowCount() - 1),
                                        qMin(nr.right, m_model->columnCount() - 1));
        m_view->setCurrentIndex(a0);
        m_view->selectionModel()->select(QItemSelection(a0, b0), QItemSelectionModel::ClearAndSelect);
        m_view->scrollTo(a0);
        m_view->setFocus();
        return;
    }

    // 1b) Đa vùng "A1:B3,D5,F1:F10" hoặc cả cột/hàng "A:A", "1:1" -> chọn (multi-range).
    if (text.contains(QLatin1Char(',')) || text.contains(QLatin1Char(':'))) {
        const auto ranges = rangeparse::parseMulti(text, m_model->rowCount(), m_model->columnCount());
        if (!ranges.isEmpty()) {
            QItemSelection sel;
            for (const MergeRange &mr : ranges)
                sel.select(m_model->index(mr.top, mr.left), m_model->index(mr.bottom, mr.right));
            m_view->selectionModel()->select(sel, QItemSelectionModel::ClearAndSelect);
            m_view->setCurrentIndex(m_model->index(ranges.first().top, ranges.first().left));
            m_view->scrollTo(m_model->index(ranges.first().top, ranges.first().left));
            m_view->setFocus();
            return;
        }
    }

    QString first = text.section(':', 0, 0), last = text.section(':', 1, 1);
    QPoint a = parseCellRef(first);
    // 2) Không phải địa chỉ ô + bắt đầu bằng chữ -> ĐẶT TÊN cho vùng đang chọn.
    if (a.x() < 0) {
        const QString why = namevalidate::reason(text);
        if (!why.isEmpty()) {
            statusBar()->showMessage(QStringLiteral("Tên không hợp lệ: %1").arg(why), 4000);
        } else {
            int t, l, b, r;
            if (selectionBox(t, l, b, r)) {
                m_model->defineName(text, MergeRange{t, l, b, r});
                statusBar()->showMessage(QStringLiteral("Đã đặt tên \"%1\" cho vùng chọn").arg(text), 3000);
            }
        }
        m_view->setFocus();
        return;
    }
    int rows = m_model->rowCount(), cols = m_model->columnCount();
    if (a.y() >= rows || a.x() >= cols) { m_view->setFocus(); return; }

    QModelIndex anchor = m_model->index(a.y(), a.x());
    if (!last.isEmpty()) {
        QPoint b = parseCellRef(last);
        if (b.x() >= 0 && b.y() < rows && b.x() < cols) {
            // Chọn vùng A:B.
            QModelIndex c2 = m_model->index(qMin(b.y(), rows - 1), qMin(b.x(), cols - 1));
            m_view->setCurrentIndex(anchor);
            m_view->selectionModel()->select(QItemSelection(anchor, c2),
                                             QItemSelectionModel::ClearAndSelect);
            m_view->scrollTo(anchor);
            m_view->setFocus();
            return;
        }
    }
    m_view->setCurrentIndex(anchor);
    m_view->scrollTo(anchor);
    m_view->setFocus();
}

void MainWindow::onFormulaBarCommitted()
{
    QModelIndex idx = m_view->currentIndex();
    if (!idx.isValid()) return;
    m_model->setData(idx, m_formulaBar->text(), Qt::EditRole);
    // Nhập xong Enter -> nhảy xuống ô ngay dưới (giống Excel).
    const int nextRow = idx.row() + 1;
    if (nextRow < m_model->rowCount()) {
        QModelIndex below = m_model->index(nextRow, idx.column());
        m_view->setCurrentIndex(below);
        m_view->scrollTo(below);
    }
    m_view->setFocus();
    setCellMode(int(cellmode::Mode::Ready)); // nhập xong -> sẵn sàng
}

// ---------------------------------------------------------------- menu / toolbar
void MainWindow::buildMenus()
{
    menuBar()->clear(); // cho phép dựng lại khi đổi ngôn ngữ
    QMenu *file = menuBar()->addMenu(i18n::tr("menu_file"));
    file->addAction(i18n::tr("file_new"), QKeySequence::New, this, &MainWindow::newFile);
    file->addAction(i18n::tr("file_open"), QKeySequence::Open, this, &MainWindow::openFile);
    file->addSeparator();
    file->addAction(i18n::tr("file_save"), QKeySequence::Save, this, [this] { saveFile(); });
    file->addAction(i18n::tr("file_saveas"), QKeySequence::SaveAs, this, [this] { saveFileAs(); });
    file->addSeparator();
    file->addAction(i18n::tr("file_quit"), QKeySequence::Quit, this, &QWidget::close);

    QMenu *edit = menuBar()->addMenu(i18n::tr("menu_edit"));
    edit->addAction(i18n::tr("edit_undo"), QKeySequence::Undo, this, [this] {
        if (!m_model->undo()) statusBar()->showMessage(QStringLiteral("Không có gì để hoàn tác"), 2000);
    });
    edit->addAction(i18n::tr("edit_redo"), QKeySequence::Redo, this, [this] {
        if (!m_model->redo()) statusBar()->showMessage(QStringLiteral("Không có gì để làm lại"), 2000);
    });
    edit->addSeparator();
    edit->addAction(i18n::tr("edit_cut"), QKeySequence::Cut, this, &MainWindow::cutSelection);
    edit->addAction(i18n::tr("edit_copy"), QKeySequence::Copy, this, &MainWindow::copySelection);
    edit->addAction(i18n::tr("edit_paste"), QKeySequence::Paste, this, &MainWindow::pasteClipboard);
    edit->addAction(i18n::tr("edit_paste_special"), QKeySequence(QStringLiteral("Ctrl+Alt+V")), this, &MainWindow::pasteSpecial);
    edit->addAction(i18n::tr("edit_clear"), QKeySequence::Delete, this, &MainWindow::clearSelection);
    {
        QMenu *clearSub = edit->addMenu(QStringLiteral("Xóa..."));
        clearSub->addAction(QStringLiteral("Tất cả (nội dung + định dạng)"), this, &MainWindow::clearAllSel);
        clearSub->addAction(QStringLiteral("Chỉ định dạng"), this, &MainWindow::clearFormatsSel);
        clearSub->addAction(QStringLiteral("Chỉ nội dung"), this, &MainWindow::clearSelection);
    }
    edit->addSeparator();
    edit->addAction(i18n::tr("edit_fill_down"), QKeySequence(QStringLiteral("Ctrl+D")), this, &MainWindow::fillDown);
    edit->addAction(i18n::tr("edit_fill_right"), QKeySequence(QStringLiteral("Ctrl+R")), this, &MainWindow::fillRight);
    edit->addAction(QStringLiteral("Điền chuỗi..."), this, &MainWindow::fillSeries);
    {
        QMenu *caseSub = edit->addMenu(QStringLiteral("Đổi chữ hoa/thường"));
        caseSub->addAction(QStringLiteral("CHỮ HOA"), this, [this] { changeCase(0); });
        caseSub->addAction(QStringLiteral("chữ thường"), this, [this] { changeCase(1); });
        caseSub->addAction(QStringLiteral("Viết Hoa Đầu Từ"), this, [this] { changeCase(2); });
    }
    edit->addAction(QStringLiteral("Cắt gọn khoảng trắng"), this, &MainWindow::trimSelection);
    edit->addAction(QStringLiteral("Bỏ ký tự không in được"), this, &MainWindow::cleanSelection);
    edit->addAction(i18n::tr("edit_merge"), this, &MainWindow::toggleMergeSelection);
    edit->addSeparator();
    edit->addAction(i18n::tr("edit_find"), QKeySequence::Find, this, &MainWindow::showFindReplace);
    edit->addAction(i18n::tr("edit_replace"), QKeySequence::Replace, this, &MainWindow::showFindReplace);
    edit->addSeparator();
    edit->addAction(QStringLiteral("Ghi chú ô..."), QKeySequence(QStringLiteral("Shift+F2")), this, [this] {
        QModelIndex idx = m_view->currentIndex();
        if (!idx.isValid()) return;
        bool ok = false;
        QString cur = m_model->note(idx.row(), idx.column());
        QString n = QInputDialog::getMultiLineText(this, QStringLiteral("Ghi chú ô"),
            QStringLiteral("Nội dung ghi chú (để trống = xóa):"), cur, &ok);
        if (ok) m_model->setNote(idx.row(), idx.column(), n);
    });
    edit->addAction(QStringLiteral("Xóa ghi chú ô"), this, [this] {
        QModelIndex idx = m_view->currentIndex();
        if (idx.isValid()) m_model->setNote(idx.row(), idx.column(), QString());
    });
    edit->addAction(QStringLiteral("Đi tới đặc biệt..."), this, [this] {
        QStringList kinds{QStringLiteral("Ô trống"), QStringLiteral("Ô công thức"),
                          QStringLiteral("Ô số"), QStringLiteral("Ô văn bản"),
                          QStringLiteral("Ô có dữ liệu (hằng)")};
        bool ok = false;
        QString pick = QInputDialog::getItem(this, QStringLiteral("Đi tới đặc biệt"),
            QStringLiteral("Chọn loại ô:"), kinds, 0, false, &ok);
        if (!ok) return;
        gotospecial::Kind k = gotospecial::Kind(kinds.indexOf(pick));
        auto cells = gotospecial::find(m_model->grid(), k);
        if (cells.isEmpty()) { statusBar()->showMessage(QStringLiteral("Không có ô nào khớp"), 3000); return; }
        QItemSelection sel;
        for (const auto &p : cells) {
            QModelIndex idx = m_model->index(p.first, p.second);
            sel.select(idx, idx);
        }
        m_view->selectionModel()->select(sel, QItemSelectionModel::ClearAndSelect);
        m_view->setCurrentIndex(m_model->index(cells.first().first, cells.first().second));
        statusBar()->showMessage(QStringLiteral("Đã chọn %1 ô").arg(cells.size()), 3000);
    });
    // Chọn vùng dữ liệu liên tục quanh ô hiện hành (Ctrl+Shift+*).
    edit->addAction(QStringLiteral("Chọn vùng dữ liệu (Ctrl+Shift+*)"),
                    QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Asterisk), this, [this] {
        QModelIndex cur = m_view->currentIndex();
        int row = cur.isValid() ? cur.row() : 0, col = cur.isValid() ? cur.column() : 0;
        auto reg = gotospecial::currentRegion(m_model->grid(), row, col);
        QItemSelection sel(m_model->index(reg.top, reg.left), m_model->index(reg.bottom, reg.right));
        m_view->selectionModel()->select(sel, QItemSelectionModel::ClearAndSelect);
        statusBar()->showMessage(QStringLiteral("Đã chọn vùng dữ liệu %1 hàng × %2 cột")
            .arg(reg.bottom - reg.top + 1).arg(reg.right - reg.left + 1), 3000);
    });
    // Nhảy tới ô cuối cùng có dữ liệu.
    edit->addAction(QStringLiteral("Đi tới ô cuối"), this, [this] {
        auto lc = gotospecial::lastCell(m_model->grid());
        if (lc.first < 0) { statusBar()->showMessage(QStringLiteral("Bảng chưa có dữ liệu"), 2500); return; }
        QModelIndex idx = m_model->index(lc.first, lc.second);
        m_view->setCurrentIndex(idx);
        m_view->scrollTo(idx);
        statusBar()->showMessage(QStringLiteral("Ô cuối: %1%2")
            .arg(SpreadsheetModel::columnLabel(lc.second)).arg(lc.first + 1), 3000);
    });
    // Chọn các ô khác biệt so với ô mốc (cùng hàng / cùng cột) trong vùng chọn.
    auto selectDiffCells = [this](bool byRow) {
        int t, l, b, r;
        if (!selectionBox(t, l, b, r)) { statusBar()->showMessage(QStringLiteral("Hãy chọn vùng cần so sánh"), 2500); return; }
        QModelIndex cur = m_view->currentIndex();
        const int anchorCol = cur.isValid() ? qBound(l, cur.column(), r) : l;
        const int anchorRow = cur.isValid() ? qBound(t, cur.row(), b) : t;
        const auto cells = byRow
            ? gotospecial::rowDifferences(m_model->grid(), t, l, b, r, anchorCol)
            : gotospecial::colDifferences(m_model->grid(), t, l, b, r, anchorRow);
        if (cells.isEmpty()) { statusBar()->showMessage(QStringLiteral("Không có ô nào khác biệt"), 3000); return; }
        QItemSelection sel;
        for (const auto &p : cells) { QModelIndex i = m_model->index(p.first, p.second); sel.select(i, i); }
        m_view->selectionModel()->select(sel, QItemSelectionModel::ClearAndSelect);
        statusBar()->showMessage(QStringLiteral("Đã chọn %1 ô khác biệt").arg(cells.size()), 3000);
    };
    edit->addAction(QStringLiteral("Khác biệt theo hàng"), this, [selectDiffCells] { selectDiffCells(true); });
    edit->addAction(QStringLiteral("Khác biệt theo cột"), this, [selectDiffCells] { selectDiffCells(false); });
    // Sao chép chỉ ô hiện (Alt+;): bỏ qua các hàng đang ẩn (đã lọc).
    edit->addAction(QStringLiteral("Sao chép chỉ ô hiện (Alt+;)"),
                    QKeySequence(Qt::ALT | Qt::Key_Semicolon), this, [this] {
        int t, l, b, r;
        if (!selectionBox(t, l, b, r)) return;
        QVector<QVector<QString>> block;
        QSet<int> hidden;
        for (int row = t; row <= b; ++row) {
            if (m_view->isRowHidden(row)) hidden.insert(row - t);
            QVector<QString> line;
            for (int col = l; col <= r; ++col)
                line.push_back(m_model->data(m_model->index(row, col), Qt::DisplayRole).toString());
            block.push_back(line);
        }
        QApplication::clipboard()->setText(copyutil::toTsvSkipHidden(block, hidden));
        statusBar()->showMessage(QStringLiteral("Đã sao chép %1 hàng hiển thị").arg(b - t + 1 - hidden.size()), 3000);
    });
    // Truy vết ô tham chiếu (precedents) / ô phụ thuộc (dependents) của ô hiện hành.
    auto selectTrace = [this](bool precedents) {
        QModelIndex cur = m_view->currentIndex();
        if (!cur.isValid()) return;
        const auto cells = precedents
            ? m_model->precedents(cur.row(), cur.column(), /*allLevels*/ true)
            : m_model->dependents(cur.row(), cur.column(), /*allLevels*/ true);
        if (cells.isEmpty()) {
            statusBar()->showMessage(precedents ? QStringLiteral("Ô này không tham chiếu ô nào")
                                                : QStringLiteral("Không ô nào phụ thuộc ô này"), 3000);
            return;
        }
        QItemSelection sel;
        for (const auto &p : cells) { QModelIndex i = m_model->index(p.first, p.second); sel.select(i, i); }
        m_view->selectionModel()->select(sel, QItemSelectionModel::ClearAndSelect);
        statusBar()->showMessage(QStringLiteral("Đã chọn %1 ô %2").arg(cells.size())
            .arg(precedents ? QStringLiteral("tham chiếu") : QStringLiteral("phụ thuộc")), 3000);
    };
    edit->addAction(QStringLiteral("Truy vết ô tham chiếu (precedents)"), this, [selectTrace] { selectTrace(true); });
    edit->addAction(QStringLiteral("Truy vết ô phụ thuộc (dependents)"), this, [selectTrace] { selectTrace(false); });
    edit->addSeparator();
    edit->addAction(QStringLiteral("Quản lý tên vùng..."), this, &MainWindow::manageNames);

    QMenu *st = menuBar()->addMenu(i18n::tr("menu_struct"));
    st->addAction(i18n::tr("st_ins_row"), this, [this] {
        int t, l, b, r; if (selectionBox(t, l, b, r)) m_model->insertRows(t, b - t + 1);
    });
    st->addAction(i18n::tr("st_ins_col"), this, [this] {
        int t, l, b, r; if (selectionBox(t, l, b, r)) m_model->insertColumns(l, r - l + 1);
    });
    st->addSeparator();
    st->addAction(i18n::tr("st_del_row"), this, [this] {
        int t, l, b, r; if (selectionBox(t, l, b, r)) m_model->removeRows(t, b - t + 1);
    });
    st->addAction(i18n::tr("st_del_col"), this, [this] {
        int t, l, b, r; if (selectionBox(t, l, b, r)) m_model->removeColumns(l, r - l + 1);
    });
    st->addSeparator();
    st->addAction(i18n::tr("st_hide_row"), this, [this] {
        int t, l, b, r; if (selectionBox(t, l, b, r)) viewutil::hideRows(m_view, t, b);
    });
    st->addAction(i18n::tr("st_hide_col"), this, [this] {
        int t, l, b, r; if (selectionBox(t, l, b, r)) viewutil::hideCols(m_view, l, r);
    });
    st->addAction(i18n::tr("st_unhide"), this, [this] {
        int t, l, b, r; if (selectionBox(t, l, b, r)) viewutil::unhideRange(m_view, t, l, b, r);
    });
    st->addSeparator();
    st->addAction(i18n::tr("st_fit_col"), this, [this] {
        int t, l, b, r;
        if (selectionBox(t, l, b, r)) for (int c = l; c <= r; ++c) m_view->resizeColumnToContents(c);
        else m_view->resizeColumnsToContents();
    });
    st->addAction(i18n::tr("st_fit_row"), this, [this] {
        int t, l, b, r;
        if (selectionBox(t, l, b, r)) for (int row = t; row <= b; ++row) m_view->resizeRowToContents(row);
        else m_view->resizeRowsToContents();
    });

    QMenu *data = menuBar()->addMenu(i18n::tr("menu_data"));
    data->addAction(i18n::tr("data_sort_asc"), this, [this] {
        int t, l, b, r; if (!selectionBox(t, l, b, r)) return;
        int kc = m_view->currentIndex().isValid() ? m_view->currentIndex().column() : l;
        m_model->sortRange(t, l, b, r, qBound(l, kc, r), true);
    });
    data->addAction(i18n::tr("data_sort_desc"), this, [this] {
        int t, l, b, r; if (!selectionBox(t, l, b, r)) return;
        int kc = m_view->currentIndex().isValid() ? m_view->currentIndex().column() : l;
        m_model->sortRange(t, l, b, r, qBound(l, kc, r), false);
    });
    data->addAction(QStringLiteral("Sắp xếp nhiều cấp..."), this, &MainWindow::sortMultiLevel);
    data->addAction(QStringLiteral("Đảo ngược thứ tự hàng"), this, &MainWindow::reverseRowsSelection);
    data->addAction(QStringLiteral("Đảo ngược thứ tự cột"), this, &MainWindow::reverseColsSelection);
    data->addAction(QStringLiteral("Chọn ô trùng giá trị"), this, &MainWindow::selectDuplicates);
    data->addAction(QStringLiteral("Áp phép tính với hằng số..."), this, &MainWindow::applyConstantSelection);
    data->addAction(QStringLiteral("Điền ô trống bằng giá trị trên"), this, &MainWindow::fillBlanksDownSelection);
    data->addAction(QStringLiteral("AutoSum (∑)"), QKeySequence(QStringLiteral("Alt+=")), this, [this] {
        QModelIndex idx = m_view->currentIndex();
        if (!idx.isValid()) return;
        const int r = idx.row(), c = idx.column();
        auto disp = [this](int rr, int cc) {
            return m_model->data(m_model->index(rr, cc), Qt::DisplayRole).toString();
        };
        QString formula;
        QVector<QString> above;
        for (int rr = 0; rr < r; ++rr) above.push_back(disp(rr, c));
        int run = autosum::trailingNumericRun(above);
        if (run > 0) {
            const QString col = SpreadsheetModel::columnLabel(c);
            formula = QStringLiteral("=SUM(%1%2:%1%3)").arg(col).arg(r - run + 1).arg(r);
        } else {
            QVector<QString> left;
            for (int cc = 0; cc < c; ++cc) left.push_back(disp(r, cc));
            run = autosum::trailingNumericRun(left);
            if (run > 0)
                formula = QStringLiteral("=SUM(%1%3:%2%3)")
                              .arg(SpreadsheetModel::columnLabel(c - run))
                              .arg(SpreadsheetModel::columnLabel(c - 1)).arg(r + 1);
        }
        if (!formula.isEmpty()) m_model->setData(idx, formula, Qt::EditRole);
    });
    data->addAction(QStringLiteral("Chèn ngày hôm nay"), QKeySequence(QStringLiteral("Ctrl+;")), this, [this] {
        QModelIndex idx = m_view->currentIndex();
        if (idx.isValid())
            m_model->setData(idx, QDate::currentDate().toString(QStringLiteral("dd/MM/yyyy")), Qt::EditRole);
    });
    data->addAction(QStringLiteral("Chèn giờ hiện tại"), QKeySequence(QStringLiteral("Ctrl+Shift+;")), this, [this] {
        QModelIndex idx = m_view->currentIndex();
        if (idx.isValid())
            m_model->setData(idx, QTime::currentTime().toString(QStringLiteral("HH:mm:ss")), Qt::EditRole);
    });
    data->addSeparator();
    data->addAction(i18n::tr("data_cond"), this, &MainWindow::showCondFormat);
    data->addAction(i18n::tr("data_clear_cond"), this, [this] { m_model->clearCondRules(); });
    data->addSeparator();
    data->addAction(QStringLiteral("Kiểm tra dữ liệu..."), this, &MainWindow::showDataValidation);
    data->addAction(QStringLiteral("Flash Fill (tự điền theo mẫu)"), QKeySequence(QStringLiteral("Ctrl+E")), this, &MainWindow::flashFill);
    data->addAction(QStringLiteral("Xóa kiểm tra dữ liệu"), this, [this] { m_model->clearValidationRules(); });
    data->addSeparator();
    data->addAction(i18n::tr("data_filter"), this, [this] {
        QModelIndex cur = m_view->currentIndex();
        int col = cur.isValid() ? cur.column() : 0;
        bool okIn = false;
        QString text = QInputDialog::getText(this, QStringLiteral("Lọc dữ liệu"),
            QStringLiteral("Hiện các hàng có cột %1 chứa:").arg(SpreadsheetModel::columnLabel(col)),
            QLineEdit::Normal, QString(), &okIn);
        if (!okIn) return;
        QVector<QString> colVals;
        for (int r = 0; r < m_model->rowCount(); ++r)
            colVals.push_back(m_model->data(m_model->index(r, col), Qt::DisplayRole).toString());
        for (int r = 0; r < m_model->rowCount(); ++r) m_view->setRowHidden(r, false); // reset trước
        for (int r : filterutil::rowsToHide(colVals, text)) m_view->setRowHidden(r, true);
        statusBar()->showMessage(QStringLiteral("Đã lọc cột %1").arg(SpreadsheetModel::columnLabel(col)), 3000);
    });
    data->addAction(QStringLiteral("Lọc theo giá trị..."), this, &MainWindow::filterByValues);
    data->addAction(QStringLiteral("Lọc theo số..."), this, &MainWindow::filterByNumber);
    data->addAction(QStringLiteral("Lọc tùy chỉnh (2 điều kiện)..."), this, &MainWindow::customFilter);
    data->addAction(QStringLiteral("Xóa hàng trùng"), this, &MainWindow::removeDuplicates);
    data->addAction(QStringLiteral("Tách cột theo dấu phân cách..."), this, &MainWindow::textToColumns);
    data->addAction(QStringLiteral("Gộp cột thành một..."), this, &MainWindow::joinColumnsSelection);
    data->addAction(QStringLiteral("Tổng phụ theo nhóm..."), this, &MainWindow::subtotalRange);
    data->addSeparator();
    {
        QAction *aGroup = data->addAction(QStringLiteral("Gom nhóm hàng"), this, &MainWindow::groupRows);
        aGroup->setShortcut(QKeySequence(Qt::ALT | Qt::SHIFT | Qt::Key_Right));
        QAction *aUngroup = data->addAction(QStringLiteral("Bỏ nhóm hàng"), this, &MainWindow::ungroupRows);
        aUngroup->setShortcut(QKeySequence(Qt::ALT | Qt::SHIFT | Qt::Key_Left));
        data->addAction(QStringLiteral("Thu gọn/Mở rộng nhóm"), this, &MainWindow::toggleGroupRows);
        data->addSeparator();
        data->addAction(QStringLiteral("Gom nhóm cột"), this, &MainWindow::groupCols);
        data->addAction(QStringLiteral("Bỏ nhóm cột"), this, &MainWindow::ungroupCols);
        data->addAction(QStringLiteral("Thu gọn/Mở rộng nhóm cột"), this, &MainWindow::toggleGroupCols);
    }
    data->addAction(i18n::tr("data_clear_filter"), this, [this] {
        for (int r = 0; r < m_model->rowCount(); ++r) m_view->setRowHidden(r, false);
    });

    QMenu *view = menuBar()->addMenu(i18n::tr("menu_view"));
    view->addAction(i18n::tr("view_zoom_in"), QKeySequence::ZoomIn, this, [this] { m_zoom += 10; applyZoom(); });
    view->addAction(i18n::tr("view_zoom_out"), QKeySequence::ZoomOut, this, [this] { m_zoom -= 10; applyZoom(); });
    view->addAction(i18n::tr("view_zoom_reset"), QKeySequence(QStringLiteral("Ctrl+0")), this, [this] { m_zoom = 100; applyZoom(); });
    view->addSeparator();
    QAction *gl = view->addAction(QStringLiteral("Hiện đường lưới"));
    gl->setCheckable(true);
    gl->setChecked(true);
    connect(gl, &QAction::toggled, this, [this](bool on) { m_view->setShowGrid(on); });
    QAction *wrap = view->addAction(QStringLiteral("Tự xuống dòng trong ô"));
    wrap->setCheckable(true);
    connect(wrap, &QAction::toggled, this, [this](bool on) {
        m_view->setWordWrap(on);
        if (on) {
            m_view->resizeRowsToContents();
        } else {
            const int rh = theme::RowHeight * m_zoom / 100;
            m_view->verticalHeader()->setDefaultSectionSize(rh);
            for (int r = 0; r < m_model->rowCount(); ++r) m_view->setRowHeight(r, rh);
        }
    });
    QAction *sf = view->addAction(i18n::tr("view_show_formulas"));
    sf->setCheckable(true);
    sf->setShortcut(QKeySequence(QStringLiteral("Ctrl+`")));
    connect(sf, &QAction::toggled, this, &MainWindow::toggleShowFormulas);

    QMenu *settings = menuBar()->addMenu(i18n::tr("menu_settings"));
    QMenu *langMenu = settings->addMenu(i18n::tr("menu_lang"));
    langMenu->addAction(i18n::tr("lang_vi"), this, [this] { i18n::setLang(i18n::Lang::Vi); buildMenus(); });
    langMenu->addAction(i18n::tr("lang_en"), this, [this] { i18n::setLang(i18n::Lang::En); buildMenus(); });

    QMenu *help = menuBar()->addMenu(i18n::tr("menu_help"));
    help->addAction(i18n::tr("help_check_update"), this, [this] {
#ifdef EZCEL_VERSION
        const QString ver = QStringLiteral(EZCEL_VERSION);
#else
        const QString ver = QStringLiteral("0.0.0");
#endif
        auto *up = new Updater(ver, this);
        up->checkForUpdates(/*silentIfNone*/ false);
    });
    QAction *shAct = help->addAction(i18n::tr("help_shortcuts"), this, [this] { showShortcuts(); });
    shAct->setShortcut(QKeySequence(Qt::Key_F1));
    help->addAction(i18n::tr("help_stats"), this, [this] { showWorkbookStats(); });
    help->addAction(i18n::tr("help_about"), this, [this] {
#ifdef EZCEL_VERSION
        const QString ver = QStringLiteral(EZCEL_VERSION);
#else
        const QString ver = QStringLiteral("?");
#endif
        QMessageBox::about(this, QStringLiteral("Ezcel"),
            QStringLiteral("Ezcel %1\nBảng tính gọn nhẹ viết bằng C++/Qt6.").arg(ver));
    });
}

// Hộp thoại tra cứu phím tắt (Spec 23) — bảng nhóm + phím + mô tả.
void MainWindow::showShortcuts()
{
    QDialog dlg(this);
    dlg.setWindowTitle(i18n::tr("help_shortcuts"));
    dlg.resize(520, 560);
    auto *lay = new QVBoxLayout(&dlg);

    auto *tbl = new QTableWidget(&dlg);
    const auto rows = shortcuts::all();
    tbl->setColumnCount(2);
    tbl->setRowCount(rows.size());
    tbl->setHorizontalHeaderLabels({i18n::tr("col_shortcut_keys"), i18n::tr("col_shortcut_desc")});
    tbl->verticalHeader()->setVisible(false);
    tbl->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tbl->setSelectionMode(QAbstractItemView::NoSelection);
    tbl->setShowGrid(false);

    QString lastCat;
    for (int i = 0; i < rows.size(); ++i) {
        const auto &e = rows.at(i);
        QString keyText = e.keys;
        if (e.category != lastCat) { // chèn tiêu đề nhóm vào cột phím
            keyText = QStringLiteral("【 %1 】\n%2").arg(e.category, e.keys);
            lastCat = e.category;
        }
        auto *kItem = new QTableWidgetItem(keyText);
        QFont kf = kItem->font(); kf.setBold(true); kItem->setFont(kf);
        tbl->setItem(i, 0, kItem);
        tbl->setItem(i, 1, new QTableWidgetItem(e.desc));
    }
    tbl->resizeColumnsToContents();
    tbl->resizeRowsToContents();
    tbl->horizontalHeader()->setStretchLastSection(true);
    lay->addWidget(tbl);

    auto *box = new QDialogButtonBox(QDialogButtonBox::Close, &dlg);
    connect(box, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    connect(box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    lay->addWidget(box);

    dlg.exec();
}

// Hộp thoại thống kê bảng tính (Spec 57.1): đếm ô/dữ liệu/công thức/từ cho trang hiện tại + cả workbook.
void MainWindow::showWorkbookStats()
{
    auto gather = [](SpreadsheetModel *m) {
        std::vector<QString> texts;
        const int rows = m->rowCount(), cols = m->columnCount();
        texts.reserve(size_t(rows) * cols);
        for (int r = 0; r < rows; ++r)
            for (int c = 0; c < cols; ++c)
                texts.push_back(m->data(m->index(r, c), Qt::EditRole).toString());
        return wbstats::analyze(texts);
    };

    wbstats::Result cur = gather(m_model);
    wbstats::Result wb;
    for (SpreadsheetModel *s : m_sheets) wbstats::add(wb, gather(s));

    int idx = m_sheetTabs ? m_sheetTabs->currentIndex() : 0;
    QString sheetName = (m_sheetTabs && idx >= 0) ? m_sheetTabs->tabText(idx) : QStringLiteral("Trang 1");

    QString html = QStringLiteral(
        "<b>Trang hiện tại: %1</b><table cellpadding='3'>"
        "<tr><td>Số ô có dữ liệu:</td><td align='right'>%2</td></tr>"
        "<tr><td>Số công thức:</td><td align='right'>%3</td></tr>"
        "<tr><td>Số ô là số:</td><td align='right'>%4</td></tr>"
        "<tr><td>Số từ:</td><td align='right'>%5</td></tr></table>"
        "<hr><b>Toàn bộ bảng tính (%6 trang)</b><table cellpadding='3'>"
        "<tr><td>Số ô có dữ liệu:</td><td align='right'>%7</td></tr>"
        "<tr><td>Số công thức:</td><td align='right'>%8</td></tr>"
        "<tr><td>Số ô là số:</td><td align='right'>%9</td></tr>"
        "<tr><td>Số từ:</td><td align='right'>%10</td></tr></table>")
        .arg(sheetName)
        .arg(cur.cellsWithData).arg(cur.formulas).arg(cur.numbers).arg(cur.words)
        .arg(m_sheets.size())
        .arg(wb.cellsWithData).arg(wb.formulas).arg(wb.numbers).arg(wb.words);

    QMessageBox box(this);
    box.setWindowTitle(i18n::tr("help_stats"));
    box.setTextFormat(Qt::RichText);
    box.setText(html);
    box.setStandardButtons(QMessageBox::Ok);
    box.exec();
}

void MainWindow::buildToolbar()
{
    auto ic = [](const QString &name) { return QIcon(QStringLiteral(":/icons/%1.svg").arg(name)); };
    QToolBar *tb = addToolBar(QStringLiteral("Chính"));
    tb->setMovable(false);
    tb->setStyleSheet(theme::toolbarStyle()); // dải kiểu ribbon
    tb->setToolButtonStyle(Qt::ToolButtonIconOnly);
    tb->setIconSize(QSize(18, 18));
    tb->addAction(ic("new"), QStringLiteral("Mới"), this, &MainWindow::newFile);
    tb->addAction(ic("open"), QStringLiteral("Mở"), this, &MainWindow::openFile);
    tb->addAction(ic("save"), QStringLiteral("Lưu"), this, [this] { saveFile(); });
    tb->addSeparator();
    tb->addAction(ic("undo"), QStringLiteral("Hoàn tác"), this, [this] { m_model->undo(); });
    tb->addAction(ic("redo"), QStringLiteral("Làm lại"), this, [this] { m_model->redo(); });
    tb->addSeparator();
    tb->addAction(ic("cut"), QStringLiteral("Cắt"), this, &MainWindow::cutSelection);
    tb->addAction(ic("copy"), QStringLiteral("Sao chép"), this, &MainWindow::copySelection);
    tb->addAction(ic("paste"), QStringLiteral("Dán"), this, &MainWindow::pasteClipboard);
    tb->addSeparator();
    tb->addAction(ic("sort_asc"), QStringLiteral("Sắp xếp tăng"), this, [this] {
        int t, l, b, r; if (!selectionBox(t, l, b, r)) return;
        int kc = m_view->currentIndex().isValid() ? m_view->currentIndex().column() : l;
        m_model->sortRange(t, l, b, r, qBound(l, kc, r), true);
    });
    tb->addAction(ic("sort_desc"), QStringLiteral("Sắp xếp giảm"), this, [this] {
        int t, l, b, r; if (!selectionBox(t, l, b, r)) return;
        int kc = m_view->currentIndex().isValid() ? m_view->currentIndex().column() : l;
        m_model->sortRange(t, l, b, r, qBound(l, kc, r), false);
    });
    tb->addAction(ic("find"), QStringLiteral("Tìm & Thay thế"), this, &MainWindow::showFindReplace);
    tb->addSeparator();
    tb->addAction(ic("merge"), QStringLiteral("Gộp ô"), this, &MainWindow::toggleMergeSelection);
}

// ---------------------------------------------------------------- vùng chọn
bool MainWindow::selectionBox(int &top, int &left, int &bottom, int &right) const
{
    auto idxs = m_view->selectionModel()->selectedIndexes();
    if (idxs.isEmpty()) {
        QModelIndex cur = m_view->currentIndex();
        if (!cur.isValid()) return false;
        top = bottom = cur.row(); left = right = cur.column();
        return true;
    }
    top = left = INT_MAX; bottom = right = INT_MIN;
    for (const QModelIndex &i : idxs) {
        top = qMin(top, i.row());    bottom = qMax(bottom, i.row());
        left = qMin(left, i.column()); right = qMax(right, i.column());
    }
    return true;
}

// ---------------------------------------------------------------- Tệp
void MainWindow::newFile()
{
    m_model->loadGrid(QVector<QVector<QString>>(50, QVector<QString>(26)));
    m_currentPath.clear();
    updateTitle();
}

void MainWindow::openFile()
{
    QString path = QFileDialog::getOpenFileName(this, QStringLiteral("Mở tệp"), QString(), kFileFilter);
    if (path.isEmpty()) return;
    openPath(path);
}

void MainWindow::openPath(const QString &path)
{
    const QString suffix = QFileInfo(path).suffix().toLower();
    bool ok = false;
    if (suffix == "csv" || suffix == "txt" || suffix == "tsv") {
        csvio::Grid rows = csvio::loadCsv(path, &ok);
        if (ok) m_model->loadGrid(rows);
    } else if (suffix == "xlsx" || suffix == "xlsm") {
        QVector<xlsxio::Sheet> sheets = xlsxio::loadAllSheets(path);
        if (!sheets.isEmpty()) {
            ok = true;
            // Dựng lại danh sách trang tính + tab theo các sheet trong file.
            m_sheetTabs->blockSignals(true);
            while (m_sheetTabs->count() > 0) m_sheetTabs->removeTab(0);
            for (auto *old : m_sheets) old->deleteLater();
            m_sheets.clear();
            for (const auto &sh : sheets) {
                auto *m = new SpreadsheetModel(this);
                m->loadGrid(sh.rows);
                for (const auto &mr : sh.merges) m->mergeCells(mr.top, mr.left, mr.bottom, mr.right);
                if (!sh.formats.isEmpty()) m->setCellFormats(sh.formats);
                m_sheets.push_back(m);
                m_sheetTabs->addTab(sh.name.isEmpty()
                    ? QStringLiteral("Trang %1").arg(m_sheets.size()) : sh.name);
            }
            m_sheetTabs->blockSignals(false);
            m_sheetTabs->setCurrentIndex(0);
            switchToSheet(0);
        }
    } else {
        QMessageBox::warning(this, QStringLiteral("Ezcel"),
                             QStringLiteral("Định dạng không hỗ trợ: .%1").arg(suffix));
        return;
    }
    if (!ok) {
        QMessageBox::critical(this, QStringLiteral("Lỗi mở tệp"),
                              QStringLiteral("Không đọc được: %1").arg(path));
        return;
    }
    m_currentPath = path;
    updateTitle();
    statusBar()->showMessage(QStringLiteral("Đã mở: %1").arg(path), 5000);
}

bool MainWindow::saveFile()
{
    if (m_currentPath.isEmpty()) return saveFileAs();
    return saveTo(m_currentPath);
}

bool MainWindow::saveFileAs()
{
    QString suggested = m_currentPath.isEmpty() ? QString() : QFileInfo(m_currentPath).fileName();
    QString path = QFileDialog::getSaveFileName(this, QStringLiteral("Lưu thành"), suggested, kFileFilter);
    if (path.isEmpty()) return false;
    if (QFileInfo(path).suffix().isEmpty()) path += ".csv";
    if (!saveTo(path)) return false;
    m_currentPath = path;
    updateTitle();
    return true;
}

bool MainWindow::saveTo(const QString &path)
{
    const QString suffix = QFileInfo(path).suffix().toLower();
    bool ok = false;
    if (suffix == "csv" || suffix == "txt" || suffix == "tsv") {
        ok = csvio::saveCsv(path, m_model->grid());
    } else if (suffix == "xlsx" || suffix == "xlsm") {
        QVector<xlsxio::Sheet> sheets;
        for (int i = 0; i < m_sheets.size(); ++i) {
            xlsxio::Sheet sh;
            sh.name = m_sheetTabs->tabText(i);
            sh.rows = m_sheets[i]->grid();
            for (const auto &m : m_sheets[i]->merges())
                sh.merges.push_back({m.top, m.left, m.bottom, m.right});
            sh.formats = m_sheets[i]->cellFormats();
            sheets.push_back(sh);
        }
        ok = xlsxio::saveSheets(path, sheets);
    } else {
        QMessageBox::warning(this, QStringLiteral("Ezcel"),
                             QStringLiteral("Định dạng không hỗ trợ: .%1").arg(suffix));
        return false;
    }
    if (!ok)
        QMessageBox::critical(this, QStringLiteral("Lỗi lưu tệp"),
                              QStringLiteral("Không ghi được: %1").arg(path));
    else
        statusBar()->showMessage(QStringLiteral("Đã lưu: %1").arg(path), 5000);
    return ok;
}

// ---------------------------------------------------------------- copy / cut / paste
void MainWindow::copySelection() { doCopy(false); }
void MainWindow::cutSelection()  { doCopy(true); }

void MainWindow::doCopy(bool cut)
{
    int t, l, b, r;
    if (!selectionBox(t, l, b, r)) return;
    // TSV: dùng giá trị hiển thị (như Excel khi dán ra ngoài).
    QString tsv;
    for (int row = t; row <= b; ++row) {
        for (int col = l; col <= r; ++col) {
            if (col > l) tsv += QLatin1Char('\t');
            tsv += m_model->data(m_model->index(row, col), Qt::DisplayRole).toString();
        }
        if (row < b) tsv += QLatin1Char('\n');
    }
    QApplication::clipboard()->setText(tsv);
    if (cut) {
        m_model->clearRange(t, l, b, r);
        statusBar()->showMessage(QStringLiteral("Đã cắt"), 2000);
    } else {
        statusBar()->showMessage(QStringLiteral("Đã sao chép"), 2000);
    }
}

// Phân tích nội dung clipboard (TSV) thành khối ô.
static QVector<QVector<QString>> clipboardBlock()
{
    QString text = QApplication::clipboard()->text();
    if (text.isEmpty()) return {};
    text.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
    text.replace(QLatin1Char('\r'), QLatin1Char('\n'));
    QStringList lines = text.split(QLatin1Char('\n'));
    if (!lines.isEmpty() && lines.last().isEmpty()) lines.removeLast();
    QVector<QVector<QString>> block;
    for (const QString &line : lines) {
        QVector<QString> rowv;
        for (const QString &cell : line.split(QLatin1Char('\t'))) rowv.push_back(cell);
        block.push_back(rowv);
    }
    return block;
}

void MainWindow::pasteClipboard()
{
    QModelIndex idx = m_view->currentIndex();
    if (!idx.isValid()) return;
    QVector<QVector<QString>> block = clipboardBlock();
    if (block.isEmpty()) return;
    m_model->pasteBlock(idx.row(), idx.column(), block);
    statusBar()->showMessage(QStringLiteral("Đã dán"), 2000);
}

void MainWindow::pasteSpecial()
{
    QModelIndex idx = m_view->currentIndex();
    if (!idx.isValid()) return;
    QVector<QVector<QString>> block = clipboardBlock();
    if (block.isEmpty()) {
        statusBar()->showMessage(QStringLiteral("Clipboard trống"), 2000);
        return;
    }

    // Hộp thoại Dán đặc biệt: phép tính (None/Cộng/Trừ/Nhân/Chia) + Bỏ qua ô
    // trống + Chuyển vị (Spec 13 §13.3).
    QDialog dlg(this);
    dlg.setWindowTitle(QStringLiteral("Dán đặc biệt"));
    auto *lay = new QVBoxLayout(&dlg);

    auto *opGroup = new QGroupBox(QStringLiteral("Phép tính"), &dlg);
    auto *opLay = new QVBoxLayout(opGroup);
    auto *bg = new QButtonGroup(&dlg);
    const QStringList opNames{QStringLiteral("Không"), QStringLiteral("Cộng"),
        QStringLiteral("Trừ"), QStringLiteral("Nhân"), QStringLiteral("Chia")};
    for (int i = 0; i < opNames.size(); ++i) {
        auto *rb = new QRadioButton(opNames[i], opGroup);
        if (i == 0) rb->setChecked(true);
        bg->addButton(rb, i);
        opLay->addWidget(rb);
    }
    lay->addWidget(opGroup);

    auto *cbSkip = new QCheckBox(QStringLiteral("Bỏ qua ô trống"), &dlg);
    auto *cbTrans = new QCheckBox(QStringLiteral("Chuyển vị (hàng ↔ cột)"), &dlg);
    lay->addWidget(cbSkip);
    lay->addWidget(cbTrans);

    auto *box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    connect(box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(box, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    lay->addWidget(box);
    if (dlg.exec() != QDialog::Accepted) return;

    if (cbTrans->isChecked()) block = pasteops::transpose(block);

    const auto op = static_cast<pasteops::Op>(bg->checkedId());
    const bool skip = cbSkip->isChecked();
    const int row0 = idx.row(), col0 = idx.column();

    // Nếu có phép tính hoặc Bỏ-qua-ô-trống thì cần đọc giá trị đích hiện tại,
    // gộp với nguồn rồi mới ghi (nếu không thì dán thẳng như cũ).
    if (op != pasteops::Op::None || skip) {
        QVector<QVector<QString>> dest;
        for (int r = 0; r < block.size(); ++r) {
            QVector<QString> drow;
            for (int c = 0; c < block[r].size(); ++c) {
                QModelIndex di = m_model->index(row0 + r, col0 + c);
                drow.push_back(di.isValid() ? m_model->data(di, Qt::EditRole).toString() : QString());
            }
            dest.push_back(drow);
        }
        block = pasteops::applyOperation(dest, block, op, skip);
    }

    m_model->pasteBlock(row0, col0, block);
    statusBar()->showMessage(QStringLiteral("Đã dán đặc biệt"), 2000);
}

void MainWindow::clearSelection()
{
    int t, l, b, r;
    if (selectionBox(t, l, b, r)) m_model->clearRange(t, l, b, r);
}

void MainWindow::clearFormatsSel()
{
    int t, l, b, r;
    if (selectionBox(t, l, b, r)) m_model->clearFormatsRange(t, l, b, r);
}

void MainWindow::clearAllSel()
{
    int t, l, b, r;
    if (selectionBox(t, l, b, r)) m_model->clearAllRange(t, l, b, r);
}

// ---------------------------------------------------------------- fill / merge / view
void MainWindow::fillDown()
{
    int t, l, b, r;
    if (!selectionBox(t, l, b, r) || b <= t) return;
    for (int c = l; c <= r; ++c) m_model->autofillVertical(c, t, t, b);
}

void MainWindow::fillRight()
{
    int t, l, b, r;
    if (!selectionBox(t, l, b, r) || r <= l) return;
    for (int row = t; row <= b; ++row) m_model->autofillHorizontal(row, l, l, r);
}

// Điền chuỗi số (Fill Series, Spec 05): chọn kiểu (cấp số cộng/nhân) + bước nhảy;
// dùng giá trị ô đầu làm số bắt đầu, điền theo cột (nếu cao) hoặc theo hàng.
void MainWindow::fillSeries()
{
    int t, l, b, r;
    if (!selectionBox(t, l, b, r) || (t == b && l == r)) {
        statusBar()->showMessage(QStringLiteral("Hãy chọn vùng (từ 2 ô) để điền chuỗi"), 2500);
        return;
    }
    const bool vertical = (b - t) >= (r - l); // hướng dài hơn

    bool okType = false;
    const QStringList types{QStringLiteral("Cấp số cộng (+ bước)"), QStringLiteral("Cấp số nhân (× bước)")};
    const QString pick = QInputDialog::getItem(this, QStringLiteral("Điền chuỗi"),
        QStringLiteral("Kiểu chuỗi:"), types, 0, false, &okType);
    if (!okType) return;
    const auto type = (types.indexOf(pick) == 1) ? seriesgen::Type::Growth : seriesgen::Type::Linear;

    bool okStep = false;
    const double step = QInputDialog::getDouble(this, QStringLiteral("Điền chuỗi"),
        QStringLiteral("Giá trị bước nhảy:"), type == seriesgen::Type::Growth ? 2 : 1,
        -1e12, 1e12, 6, &okStep);
    if (!okStep) return;

    auto fillLine = [&](int row0, int col0, int count, bool vert) {
        const QString seed = m_model->data(m_model->index(row0, col0), Qt::EditRole).toString();
        bool okSeed = false; double start = seed.trimmed().toDouble(&okSeed);
        if (!okSeed) start = (type == seriesgen::Type::Growth) ? 1.0 : 0.0;
        const auto vals = seriesgen::generate(start, step, count, type);
        for (int i = 0; i < vals.size(); ++i) {
            int rr = vert ? row0 + i : row0;
            int cc = vert ? col0 : col0 + i;
            m_model->setData(m_model->index(rr, cc), QString::number(vals[i], 'g', 15), Qt::EditRole);
        }
    };

    if (vertical) for (int c = l; c <= r; ++c) fillLine(t, c, b - t + 1, true);
    else          for (int row = t; row <= b; ++row) fillLine(row, l, r - l + 1, false);
    statusBar()->showMessage(QStringLiteral("Đã điền chuỗi"), 2500);
}

// Đảo ngược thứ tự hàng trong vùng chọn (Spec 15): hàng đầu thành cuối.
void MainWindow::reverseRowsSelection()
{
    int t, l, b, r;
    if (!selectionBox(t, l, b, r) || b <= t) { statusBar()->showMessage(QStringLiteral("Hãy chọn từ 2 hàng trở lên"), 2500); return; }
    QVector<QVector<QString>> block;
    for (int row = t; row <= b; ++row) {
        QVector<QString> line;
        for (int c = l; c <= r; ++c) line.push_back(m_model->data(m_model->index(row, c), Qt::EditRole).toString());
        block.push_back(line);
    }
    const auto rev = datatools::reverseRows(block);
    for (int i = 0; i < rev.size(); ++i)
        for (int c = l; c <= r; ++c)
            m_model->setData(m_model->index(t + i, c), rev[i][c - l], Qt::EditRole);
    statusBar()->showMessage(QStringLiteral("Đã đảo ngược %1 hàng").arg(rev.size()), 2500);
}

// Áp một phép tính với hằng số lên các ô SỐ trong vùng chọn (Spec 13).
void MainWindow::applyConstantSelection()
{
    int t, l, b, r;
    if (!selectionBox(t, l, b, r)) return;
    bool ok = false;
    const QStringList ops{QStringLiteral("Cộng (+)"), QStringLiteral("Trừ (−)"),
                          QStringLiteral("Nhân (×)"), QStringLiteral("Chia (÷)")};
    const QString pick = QInputDialog::getItem(this, QStringLiteral("Áp phép tính"),
        QStringLiteral("Phép tính:"), ops, 0, false, &ok);
    if (!ok) return;
    const auto op = static_cast<pasteops::Op>(ops.indexOf(pick) + 1); // +1: bỏ Op::None
    const double k = QInputDialog::getDouble(this, QStringLiteral("Áp phép tính"),
        QStringLiteral("Hằng số:"), 0, -1e12, 1e12, 6, &ok);
    if (!ok) return;
    int n = 0;
    for (int row = t; row <= b; ++row)
        for (int c = l; c <= r; ++c) {
            const QString raw = m_model->data(m_model->index(row, c), Qt::EditRole).toString();
            if (raw.startsWith(QLatin1Char('='))) continue; // bỏ công thức
            const QString nw = pasteops::applyConstant(raw, op, k);
            if (nw != raw) { m_model->setData(m_model->index(row, c), nw, Qt::EditRole); ++n; }
        }
    statusBar()->showMessage(QStringLiteral("Đã áp phép tính lên %1 ô").arg(n), 2500);
}

// Đảo ngược thứ tự cột trong vùng chọn (Spec 15): cột đầu thành cuối.
void MainWindow::reverseColsSelection()
{
    int t, l, b, r;
    if (!selectionBox(t, l, b, r) || r <= l) { statusBar()->showMessage(QStringLiteral("Hãy chọn từ 2 cột trở lên"), 2500); return; }
    QVector<QVector<QString>> block;
    for (int row = t; row <= b; ++row) {
        QVector<QString> line;
        for (int c = l; c <= r; ++c) line.push_back(m_model->data(m_model->index(row, c), Qt::EditRole).toString());
        block.push_back(line);
    }
    const auto rev = datatools::reverseCols(block);
    for (int i = 0; i < rev.size(); ++i)
        for (int c = l; c <= r; ++c)
            m_model->setData(m_model->index(t + i, c), rev[i][c - l], Qt::EditRole);
    statusBar()->showMessage(QStringLiteral("Đã đảo ngược %1 cột").arg(r - l + 1), 2500);
}

// Điền ô trống bằng giá trị phía trên cho vùng chọn (Spec 27 Clean Data).
void MainWindow::fillBlanksDownSelection()
{
    int t, l, b, r;
    if (!selectionBox(t, l, b, r) || b <= t) { statusBar()->showMessage(QStringLiteral("Hãy chọn từ 2 hàng trở lên"), 2500); return; }
    QVector<QVector<QString>> block;
    for (int row = t; row <= b; ++row) {
        QVector<QString> line;
        for (int c = l; c <= r; ++c) line.push_back(m_model->data(m_model->index(row, c), Qt::EditRole).toString());
        block.push_back(line);
    }
    const auto filled = datatools::fillBlanksDown(block);
    int n = 0;
    for (int i = 0; i < filled.size(); ++i)
        for (int c = l; c <= r; ++c)
            if (filled[i][c - l] != block[i][c - l]) { m_model->setData(m_model->index(t + i, c), filled[i][c - l], Qt::EditRole); ++n; }
    statusBar()->showMessage(QStringLiteral("Đã điền %1 ô trống").arg(n), 2500);
}

// Chọn các ô trùng giá trị trong cột của vùng chọn (Spec 15/27).
void MainWindow::selectDuplicates()
{
    int t, l, b, r;
    if (!selectionBox(t, l, b, r)) return;
    QModelIndex cur = m_view->currentIndex();
    const int col = cur.isValid() ? qBound(l, cur.column(), r) : l;
    QVector<QString> vals;
    for (int row = t; row <= b; ++row)
        vals.push_back(m_model->data(m_model->index(row, col), Qt::DisplayRole).toString());
    const auto dup = datatools::duplicateValueIndices(vals);
    if (dup.isEmpty()) { statusBar()->showMessage(QStringLiteral("Không có ô trùng giá trị"), 3000); return; }
    QItemSelection sel;
    for (int i : dup) { QModelIndex idx = m_model->index(t + i, col); sel.select(idx, idx); }
    m_view->selectionModel()->select(sel, QItemSelectionModel::ClearAndSelect);
    statusBar()->showMessage(QStringLiteral("Đã chọn %1 ô trùng giá trị").arg(dup.size()), 3000);
}

// Gộp các cột trong vùng chọn thành một cột (Spec 27): ngược với tách cột.
void MainWindow::joinColumnsSelection()
{
    int t, l, b, r;
    if (!selectionBox(t, l, b, r) || r <= l) { statusBar()->showMessage(QStringLiteral("Hãy chọn từ 2 cột trở lên"), 2500); return; }
    bool ok = false;
    const QString sep = QInputDialog::getText(this, QStringLiteral("Gộp cột"),
        QStringLiteral("Dấu ngăn giữa các cột (vd dấu cách, , hoặc -):"),
        QLineEdit::Normal, QStringLiteral(" "), &ok);
    if (!ok) return;

    QVector<QVector<QString>> block;
    for (int row = t; row <= b; ++row) {
        QVector<QString> line;
        for (int c = l; c <= r; ++c) line.push_back(m_model->data(m_model->index(row, c), Qt::EditRole).toString());
        block.push_back(line);
    }
    const QStringList joined = datatools::joinColumns(block, sep, /*skipEmpty*/ true);
    for (int i = 0; i < joined.size(); ++i) {
        m_model->setData(m_model->index(t + i, l), joined[i], Qt::EditRole);          // kết quả vào cột đầu
        for (int c = l + 1; c <= r; ++c) m_model->setData(m_model->index(t + i, c), QString(), Qt::EditRole); // xóa cột còn lại
    }
    statusBar()->showMessage(QStringLiteral("Đã gộp %1 cột vào cột đầu").arg(r - l + 1), 3000);
}

// Đổi chữ hoa/thường cho vùng chọn (Spec 05/30): chỉ đổi ô là văn bản, bỏ qua công thức.
void MainWindow::changeCase(int mode)
{
    int t, l, b, r;
    if (!selectionBox(t, l, b, r)) return;
    const auto m = static_cast<textcase::Mode>(mode);
    int changed = 0;
    for (int row = t; row <= b; ++row)
        for (int c = l; c <= r; ++c) {
            const QString raw = m_model->data(m_model->index(row, c), Qt::EditRole).toString();
            if (raw.isEmpty() || raw.startsWith(QLatin1Char('='))) continue; // bỏ công thức
            const QString nw = textcase::convert(raw, m);
            if (nw != raw) { m_model->setData(m_model->index(row, c), nw, Qt::EditRole); ++changed; }
        }
    statusBar()->showMessage(QStringLiteral("Đã đổi kiểu chữ %1 ô").arg(changed), 2500);
}

// Cắt gọn khoảng trắng cho vùng chọn (Spec 27 Clean Data): bỏ ô công thức.
void MainWindow::trimSelection()
{
    int t, l, b, r;
    if (!selectionBox(t, l, b, r)) return;
    int changed = 0;
    for (int row = t; row <= b; ++row)
        for (int c = l; c <= r; ++c) {
            const QString raw = m_model->data(m_model->index(row, c), Qt::EditRole).toString();
            if (raw.isEmpty() || raw.startsWith(QLatin1Char('='))) continue;
            const QString nw = textcase::trimSpaces(raw);
            if (nw != raw) { m_model->setData(m_model->index(row, c), nw, Qt::EditRole); ++changed; }
        }
    statusBar()->showMessage(QStringLiteral("Đã cắt gọn khoảng trắng %1 ô").arg(changed), 2500);
}

// Bỏ ký tự không in được cho vùng chọn (Spec 27 Clean Data): bỏ ô công thức.
void MainWindow::cleanSelection()
{
    int t, l, b, r;
    if (!selectionBox(t, l, b, r)) return;
    int changed = 0;
    for (int row = t; row <= b; ++row)
        for (int c = l; c <= r; ++c) {
            const QString raw = m_model->data(m_model->index(row, c), Qt::EditRole).toString();
            if (raw.isEmpty() || raw.startsWith(QLatin1Char('='))) continue;
            const QString nw = textcase::removeNonPrintable(raw);
            if (nw != raw) { m_model->setData(m_model->index(row, c), nw, Qt::EditRole); ++changed; }
        }
    statusBar()->showMessage(QStringLiteral("Đã làm sạch %1 ô").arg(changed), 2500);
}

void MainWindow::toggleMergeSelection()
{
    int t, l, b, r;
    if (selectionBox(t, l, b, r)) m_model->toggleMerge(t, l, b, r);
}

// Flash Fill (Spec 05): cột đích = cột ô hiện tại (chứa 1-2 ví dụ ở đầu),
// cột nguồn = cột bên trái. Suy luật rồi điền các ô còn lại.
void MainWindow::flashFill()
{
    QModelIndex cur = m_view->currentIndex();
    if (!cur.isValid()) return;
    int tcol = cur.column(), scol = tcol - 1;
    if (scol < 0) { statusBar()->showMessage(QStringLiteral("Flash Fill cần cột dữ liệu bên trái"), 2500); return; }

    std::vector<QString> sources;
    std::vector<int> rowIdx;
    const int rows = m_model->rowCount();
    for (int r = 0; r < rows; ++r) {
        QString s = m_model->data(m_model->index(r, scol), Qt::EditRole).toString();
        if (s.trimmed().isEmpty()) break; // chỉ lấy khối dữ liệu liền mạch từ trên xuống
        sources.push_back(s);
        rowIdx.push_back(r);
    }
    if (sources.empty()) { statusBar()->showMessage(QStringLiteral("Flash Fill: cột nguồn trống"), 2500); return; }

    std::vector<QString> examples;
    for (size_t i = 0; i < sources.size(); ++i) {
        QString e = m_model->data(m_model->index(rowIdx[i], tcol), Qt::EditRole).toString();
        if (e.trimmed().isEmpty()) break;
        examples.push_back(e);
    }
    if (examples.empty()) { statusBar()->showMessage(QStringLiteral("Flash Fill: hãy gõ 1-2 ví dụ ở đầu cột"), 3000); return; }

    auto out = flashfill::infer(sources, examples);
    if (!out) { statusBar()->showMessage(QStringLiteral("Flash Fill: không nhận ra mẫu"), 3000); return; }

    int filled = 0;
    for (size_t i = examples.size(); i < out->size(); ++i) {
        m_model->setData(m_model->index(rowIdx[i], tcol), (*out)[i], Qt::EditRole);
        ++filled;
    }
    statusBar()->showMessage(QStringLiteral("Flash Fill: đã điền %1 ô").arg(filled), 2500);
}

// Chọn từ danh sách (Spec 05, Alt+Down): liệt kê giá trị text duy nhất trong cột hiện tại.
void MainWindow::pickFromList()
{
    QModelIndex cur = m_view->currentIndex();
    if (!cur.isValid()) return;
    const int col = cur.column();
    std::vector<QString> colVals;
    const int rows = m_model->rowCount();
    for (int r = 0; r < rows; ++r)
        colVals.push_back(m_model->data(m_model->index(r, col), Qt::EditRole).toString());
    const QStringList items = autocomplete::uniqueTexts(colVals);
    if (items.isEmpty()) { statusBar()->showMessage(QStringLiteral("Cột chưa có giá trị văn bản để chọn"), 2500); return; }

    QMenu menu(this);
    for (const QString &it : items)
        menu.addAction(it, this, [this, cur, it] { m_model->setData(cur, it, Qt::EditRole); });
    QRect rect = m_view->visualRect(cur);
    menu.exec(m_view->viewport()->mapToGlobal(rect.bottomLeft()));
}

// Lọc theo giá trị (Spec 15): hộp thoại checklist các giá trị duy nhất trong cột,
// bỏ chọn để ẩn các hàng có giá trị đó.
void MainWindow::filterByValues()
{
    QModelIndex cur = m_view->currentIndex();
    int col = cur.isValid() ? cur.column() : 0;
    QVector<QString> colVals;
    for (int r = 0; r < m_model->rowCount(); ++r)
        colVals.push_back(m_model->data(m_model->index(r, col), Qt::DisplayRole).toString());
    const QVector<QString> values = filterutil::uniqueValues(colVals);
    if (values.isEmpty()) { statusBar()->showMessage(QStringLiteral("Cột không có giá trị để lọc"), 2500); return; }

    QDialog dlg(this);
    dlg.setWindowTitle(QStringLiteral("Lọc cột %1").arg(SpreadsheetModel::columnLabel(col)));
    auto *lay = new QVBoxLayout(&dlg);
    auto *listw = new QListWidget(&dlg);
    for (const QString &v : values) {
        auto *it = new QListWidgetItem(v, listw);
        it->setFlags(it->flags() | Qt::ItemIsUserCheckable);
        it->setCheckState(Qt::Checked);
    }
    lay->addWidget(new QLabel(QStringLiteral("Chọn các giá trị muốn hiện:"), &dlg));
    lay->addWidget(listw);
    auto *box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    connect(box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(box, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    lay->addWidget(box);
    if (dlg.exec() != QDialog::Accepted) return;

    QSet<QString> keep;
    for (int i = 0; i < listw->count(); ++i)
        if (listw->item(i)->checkState() == Qt::Checked) keep.insert(listw->item(i)->text());

    for (int r = 0; r < m_model->rowCount(); ++r) m_view->setRowHidden(r, false);
    const auto hide = filterutil::rowsToHideByValues(colVals, keep);
    for (int r : hide) m_view->setRowHidden(r, true);
    statusBar()->showMessage(QStringLiteral("Đã lọc cột %1: ẩn %2 hàng")
        .arg(SpreadsheetModel::columnLabel(col)).arg(hide.size()), 3000);
}

// Lọc theo điều kiện SỐ (Number Filters, Spec 15): chọn phép so sánh + nhập số.
void MainWindow::filterByNumber()
{
    QModelIndex cur = m_view->currentIndex();
    int col = cur.isValid() ? cur.column() : 0;
    QVector<QString> colVals;
    for (int r = 0; r < m_model->rowCount(); ++r)
        colVals.push_back(m_model->data(m_model->index(r, col), Qt::DisplayRole).toString());

    // Danh sách phép so sánh đồng bộ thứ tự với enum filterutil::NumOp.
    const QStringList opNames{
        QStringLiteral("Bằng (=)"), QStringLiteral("Khác (≠)"),
        QStringLiteral("Lớn hơn (>)"), QStringLiteral("Lớn hơn hoặc bằng (≥)"),
        QStringLiteral("Nhỏ hơn (<)"), QStringLiteral("Nhỏ hơn hoặc bằng (≤)"),
        QStringLiteral("Trong khoảng [a, b]"), QStringLiteral("Ngoài khoảng [a, b]"),
        QStringLiteral("Trên trung bình"), QStringLiteral("Dưới trung bình")};
    bool ok = false;
    QString choice = QInputDialog::getItem(this, QStringLiteral("Lọc theo số"),
        QStringLiteral("Điều kiện:"), opNames, 2, false, &ok);
    if (!ok) return;
    auto op = static_cast<filterutil::NumOp>(qMax(0, opNames.indexOf(choice)));

    double v1 = 0.0, v2 = 0.0;
    if (op != filterutil::NumOp::AboveAvg && op != filterutil::NumOp::BelowAvg) {
        v1 = QInputDialog::getDouble(this, QStringLiteral("Lọc theo số"),
            QStringLiteral("Giá trị%1:").arg(
                (op == filterutil::NumOp::Between || op == filterutil::NumOp::NotBetween)
                    ? QStringLiteral(" a") : QString()),
            0, -1e12, 1e12, 4, &ok);
        if (!ok) return;
        if (op == filterutil::NumOp::Between || op == filterutil::NumOp::NotBetween) {
            v2 = QInputDialog::getDouble(this, QStringLiteral("Lọc theo số"),
                QStringLiteral("Giá trị b:"), 0, -1e12, 1e12, 4, &ok);
            if (!ok) return;
        }
    }

    for (int r = 0; r < m_model->rowCount(); ++r) m_view->setRowHidden(r, false);
    const auto hide = filterutil::rowsToHideByNumber(colVals, op, v1, v2);
    for (int r : hide) m_view->setRowHidden(r, true);
    statusBar()->showMessage(QStringLiteral("Đã lọc số cột %1: ẩn %2 hàng")
        .arg(SpreadsheetModel::columnLabel(col)).arg(hide.size()), 3000);
}

// Lọc tùy chỉnh 2 điều kiện AND/OR (Custom AutoFilter, Spec 15).
void MainWindow::customFilter()
{
    QModelIndex cur = m_view->currentIndex();
    int col = cur.isValid() ? cur.column() : 0;
    QVector<QString> colVals;
    for (int r = 0; r < m_model->rowCount(); ++r)
        colVals.push_back(m_model->data(m_model->index(r, col), Qt::DisplayRole).toString());

    // Tên phép — đồng bộ thứ tự với enum filterutil::FiltOp.
    const QStringList ops{
        QStringLiteral("bằng"), QStringLiteral("khác"),
        QStringLiteral("lớn hơn"), QStringLiteral("lớn hơn hoặc bằng"),
        QStringLiteral("nhỏ hơn"), QStringLiteral("nhỏ hơn hoặc bằng"),
        QStringLiteral("chứa"), QStringLiteral("không chứa"),
        QStringLiteral("bắt đầu bằng"), QStringLiteral("kết thúc bằng")};

    QDialog dlg(this);
    dlg.setWindowTitle(QStringLiteral("Lọc tùy chỉnh cột %1").arg(SpreadsheetModel::columnLabel(col)));
    auto *lay = new QVBoxLayout(&dlg);

    auto *cb1 = new QComboBox(&dlg); cb1->addItems(ops); cb1->setCurrentIndex(2);
    auto *ed1 = new QLineEdit(&dlg);
    auto *row1 = new QHBoxLayout(); row1->addWidget(cb1); row1->addWidget(ed1);
    lay->addLayout(row1);

    auto *rbAnd = new QRadioButton(QStringLiteral("VÀ (AND)"), &dlg);
    auto *rbOr = new QRadioButton(QStringLiteral("HOẶC (OR)"), &dlg);
    rbAnd->setChecked(true);
    auto *rowLogic = new QHBoxLayout(); rowLogic->addWidget(rbAnd); rowLogic->addWidget(rbOr);
    lay->addLayout(rowLogic);

    auto *cb2 = new QComboBox(&dlg); cb2->addItems(ops); cb2->setCurrentIndex(4);
    auto *ed2 = new QLineEdit(&dlg);
    auto *row2 = new QHBoxLayout(); row2->addWidget(cb2); row2->addWidget(ed2);
    lay->addLayout(row2);
    lay->addWidget(new QLabel(QStringLiteral("(Để trống ô giá trị thứ hai nếu chỉ dùng 1 điều kiện)"), &dlg));

    auto *box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    connect(box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(box, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    lay->addWidget(box);
    if (dlg.exec() != QDialog::Accepted) return;

    auto op1 = static_cast<filterutil::FiltOp>(cb1->currentIndex());
    auto op2 = static_cast<filterutil::FiltOp>(cb2->currentIndex());
    const bool hasSecond = !ed2->text().isEmpty();
    const auto hide = filterutil::rowsToHideCustom(colVals, op1, ed1->text(),
                                                   rbAnd->isChecked(), hasSecond, op2, ed2->text());
    for (int r = 0; r < m_model->rowCount(); ++r) m_view->setRowHidden(r, false);
    for (int r : hide) m_view->setRowHidden(r, true);
    statusBar()->showMessage(QStringLiteral("Đã lọc tùy chỉnh cột %1: ẩn %2 hàng")
        .arg(SpreadsheetModel::columnLabel(col)).arg(hide.size()), 3000);
}

// Quản lý vùng đặt tên (Spec 31): danh sách tên + địa chỉ; Đi tới / Xóa.
void MainWindow::manageNames()
{
    QStringList names = m_model->definedNames();
    names.sort(Qt::CaseInsensitive);
    if (names.isEmpty()) { statusBar()->showMessage(QStringLiteral("Chưa có vùng đặt tên nào"), 3000); return; }

    QDialog dlg(this);
    dlg.setWindowTitle(QStringLiteral("Quản lý tên vùng"));
    auto *lay = new QVBoxLayout(&dlg);
    auto *listw = new QListWidget(&dlg);
    auto refill = [this, listw] {
        listw->clear();
        QStringList ns = m_model->definedNames();
        ns.sort(Qt::CaseInsensitive);
        for (const QString &n : ns) {
            MergeRange r;
            if (m_model->lookupName(n, r))
                new QListWidgetItem(QStringLiteral("%1  →  %2").arg(n, SpreadsheetModel::rangeRef(r)), listw);
        }
    };
    refill();
    lay->addWidget(new QLabel(QStringLiteral("Các vùng đã đặt tên:"), &dlg));
    lay->addWidget(listw);

    auto *btnRow = new QHBoxLayout();
    auto *goBtn = new QPushButton(QStringLiteral("Đi tới"), &dlg);
    auto *delBtn = new QPushButton(QStringLiteral("Xóa"), &dlg);
    btnRow->addWidget(goBtn); btnRow->addWidget(delBtn); btnRow->addStretch();
    lay->addLayout(btnRow);
    auto *box = new QDialogButtonBox(QDialogButtonBox::Close, &dlg);
    connect(box, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    connect(box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    lay->addWidget(box);

    // Tên ứng với dòng đang chọn.
    auto currentName = [listw]() -> QString {
        QListWidgetItem *it = listw->currentItem();
        return it ? it->text().section(QStringLiteral("  →  "), 0, 0) : QString();
    };
    connect(delBtn, &QPushButton::clicked, &dlg, [this, currentName, refill] {
        const QString n = currentName();
        if (!n.isEmpty() && m_model->removeName(n)) { refill(); statusBar()->showMessage(QStringLiteral("Đã xóa tên \"%1\"").arg(n), 2500); }
    });
    connect(goBtn, &QPushButton::clicked, &dlg, [this, currentName, &dlg] {
        const QString n = currentName();
        MergeRange r;
        if (!n.isEmpty() && m_model->lookupName(n, r)) {
            QModelIndex a0 = m_model->index(r.top, r.left);
            QModelIndex b0 = m_model->index(qMin(r.bottom, m_model->rowCount() - 1),
                                            qMin(r.right, m_model->columnCount() - 1));
            m_view->setCurrentIndex(a0);
            m_view->selectionModel()->select(QItemSelection(a0, b0), QItemSelectionModel::ClearAndSelect);
            m_view->scrollTo(a0);
            dlg.accept();
        }
    });
    dlg.exec();
}

// Sắp xếp nhiều cấp (Spec 15): tối đa 3 cấp, mỗi cấp chọn cột + chiều tăng/giảm.
void MainWindow::sortMultiLevel()
{
    int t, l, b, r;
    if (!selectionBox(t, l, b, r)) { statusBar()->showMessage(QStringLiteral("Hãy chọn vùng cần sắp xếp"), 2500); return; }

    // Danh sách cột trong vùng chọn (nhãn A, B, ... + "(không)").
    QStringList colNames; colNames << QStringLiteral("(không)");
    QVector<int> colIdx; colIdx << -1;
    for (int c = l; c <= r; ++c) { colNames << SpreadsheetModel::columnLabel(c); colIdx << c; }

    QDialog dlg(this);
    dlg.setWindowTitle(QStringLiteral("Sắp xếp nhiều cấp"));
    auto *lay = new QVBoxLayout(&dlg);
    QVector<QComboBox *> cbCol; QVector<QComboBox *> cbOrd;
    const int kLevels = 3;
    for (int lv = 0; lv < kLevels; ++lv) {
        auto *rowLay = new QHBoxLayout();
        rowLay->addWidget(new QLabel(lv == 0 ? QStringLiteral("Sắp theo") : QStringLiteral("rồi theo"), &dlg));
        auto *cc = new QComboBox(&dlg); cc->addItems(colNames);
        if (lv == 0) cc->setCurrentIndex(1); // cấp 1 mặc định cột đầu
        auto *oo = new QComboBox(&dlg);
        oo->addItems({QStringLiteral("Tăng dần"), QStringLiteral("Giảm dần")});
        rowLay->addWidget(cc); rowLay->addWidget(oo);
        lay->addLayout(rowLay);
        cbCol << cc; cbOrd << oo;
    }
    auto *box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    connect(box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(box, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    lay->addWidget(box);
    if (dlg.exec() != QDialog::Accepted) return;

    QVector<QPair<int, bool>> keys;
    for (int lv = 0; lv < kLevels; ++lv) {
        int ci = colIdx[cbCol[lv]->currentIndex()];
        if (ci < 0) continue; // bỏ cấp "(không)"
        keys.push_back({ci, cbOrd[lv]->currentIndex() == 0}); // true = tăng
    }
    if (keys.isEmpty()) { statusBar()->showMessage(QStringLiteral("Chưa chọn cột để sắp xếp"), 2500); return; }
    m_model->sortRangeMulti(t, l, b, r, keys);
    statusBar()->showMessage(QStringLiteral("Đã sắp xếp %1 cấp").arg(keys.size()), 3000);
}

// Xóa hàng trùng (Spec 27): dùng toàn bộ cột làm khóa, giữ lần xuất hiện đầu, có tiêu đề.
void MainWindow::removeDuplicates()
{
    const int rows = m_model->rowCount(), cols = m_model->columnCount();
    // Chỉ xét khối dữ liệu liền mạch từ trên xuống.
    int lastRow = -1;
    for (int r = 0; r < rows; ++r) {
        bool empty = true;
        for (int c = 0; c < cols; ++c)
            if (!m_model->data(m_model->index(r, c), Qt::EditRole).toString().trimmed().isEmpty()) { empty = false; break; }
        if (empty) break;
        lastRow = r;
    }
    if (lastRow < 1) { statusBar()->showMessage(QStringLiteral("Không đủ dữ liệu để xóa hàng trùng"), 2500); return; }

    std::vector<std::vector<QString>> grid;
    for (int r = 0; r <= lastRow; ++r) {
        std::vector<QString> row;
        for (int c = 0; c < cols; ++c) row.push_back(m_model->data(m_model->index(r, c), Qt::EditRole).toString());
        grid.push_back(std::move(row));
    }
    QVector<int> keyCols;
    for (int c = 0; c < cols; ++c) keyCols.push_back(c);
    auto dup = datatools::duplicateRowIndices(grid, keyCols, /*hasHeader*/ true);
    if (dup.isEmpty()) { statusBar()->showMessage(QStringLiteral("Không tìm thấy hàng trùng"), 2500); return; }

    // Xóa từ dưới lên để giữ chỉ số hợp lệ.
    for (int i = dup.size() - 1; i >= 0; --i) m_model->removeRows(dup[i], 1);
    statusBar()->showMessage(QStringLiteral("Đã xóa %1 hàng trùng").arg(dup.size()), 3000);
}

// Tách cột theo dấu phân cách (Spec 27): cột hiện tại tách thành nhiều cột bên phải.
void MainWindow::textToColumns()
{
    QModelIndex cur = m_view->currentIndex();
    int col = cur.isValid() ? cur.column() : 0;
    bool okIn = false;
    QString delim = QInputDialog::getText(this, QStringLiteral("Tách cột"),
        QStringLiteral("Dấu phân cách (vd , hoặc ; hoặc dấu cách):"),
        QLineEdit::Normal, QStringLiteral(","), &okIn);
    if (!okIn || delim.isEmpty()) return;

    const int rows = m_model->rowCount(), cols = m_model->columnCount();
    int filled = 0;
    for (int r = 0; r < rows; ++r) {
        QString text = m_model->data(m_model->index(r, col), Qt::EditRole).toString();
        if (text.trimmed().isEmpty()) continue;
        const QStringList parts = datatools::splitDelimited(text, delim);
        for (int k = 0; k < parts.size() && col + k < cols; ++k)
            m_model->setData(m_model->index(r, col + k), parts[k], Qt::EditRole);
        ++filled;
    }
    statusBar()->showMessage(QStringLiteral("Đã tách %1 hàng theo '%2'").arg(filled).arg(delim), 3000);
}

// Tổng phụ (Subtotal, Spec 27.6): chèn dòng tổng sau mỗi nhóm + dòng tổng cộng.
// Dữ liệu cần đã sắp xếp theo cột nhóm. Cột nhóm = cột ô hiện hành.
void MainWindow::subtotalRange()
{
    const int rows = m_model->rowCount(), cols = m_model->columnCount();
    int lastRow = -1;
    for (int r = 0; r < rows; ++r) {
        bool empty = true;
        for (int c = 0; c < cols; ++c)
            if (!m_model->data(m_model->index(r, c), Qt::EditRole).toString().trimmed().isEmpty()) { empty = false; break; }
        if (empty) break;
        lastRow = r;
    }
    if (lastRow < 1) { statusBar()->showMessage(QStringLiteral("Không đủ dữ liệu để tính tổng phụ"), 2500); return; }

    QModelIndex cur = m_view->currentIndex();
    int groupCol = cur.isValid() ? cur.column() : 0;

    bool ok = false;
    const QStringList fnNames{QStringLiteral("Tổng (Sum)"), QStringLiteral("Đếm (Count)"),
        QStringLiteral("Trung bình (Average)"), QStringLiteral("Lớn nhất (Max)"), QStringLiteral("Nhỏ nhất (Min)")};
    QString fnChoice = QInputDialog::getItem(this, QStringLiteral("Tổng phụ"),
        QStringLiteral("Hàm tổng hợp:"), fnNames, 0, false, &ok);
    if (!ok) return;
    datatools::Agg fn = datatools::Agg::Sum;
    int fi = fnNames.indexOf(fnChoice);
    fn = static_cast<datatools::Agg>(qMax(0, fi));

    int aggCol = QInputDialog::getInt(this, QStringLiteral("Tổng phụ"),
        QStringLiteral("Cột cần tổng hợp (số thứ tự, A=1):"),
        qMin(groupCol + 2, cols), 1, cols, 1, &ok) - 1;
    if (!ok) return;

    // Lấy khối DỮ LIỆU (bỏ dòng tiêu đề hàng 0).
    QVector<QVector<QString>> data;
    for (int r = 1; r <= lastRow; ++r) {
        QVector<QString> row;
        for (int c = 0; c < cols; ++c) row.push_back(m_model->data(m_model->index(r, c), Qt::EditRole).toString());
        data.push_back(row);
    }
    auto result = datatools::subtotal(data, groupCol, {aggCol}, fn,
                                      QStringLiteral("Tổng"), QStringLiteral("Tổng cộng"));
    if (result.isEmpty()) return;

    // Chèn thêm dòng cho phần dôi ra rồi ghi đè kết quả bắt đầu từ hàng 1.
    int extra = result.size() - data.size();
    if (extra > 0) m_model->insertRows(lastRow + 1, extra);
    for (int r = 0; r < result.size(); ++r)
        for (int c = 0; c < cols && c < result[r].size(); ++c)
            m_model->setData(m_model->index(1 + r, c), result[r][c], Qt::EditRole);
    statusBar()->showMessage(QStringLiteral("Đã chèn tổng phụ theo nhóm"), 3000);
}

// --- Gom nhóm / phác thảo hàng (Spec 09.4) ---
// Áp ẩn/hiện hàng theo các nhóm đang thu gọn: hiện hết rồi ẩn lại phần thu gọn.
void MainWindow::applyRowOutline()
{
    const QSet<int> hidden = m_rowOutline.hiddenRows();
    for (int r = 0; r < m_model->rowCount(); ++r)
        m_view->setRowHidden(r, hidden.contains(r));
}

void MainWindow::groupRows()
{
    int t, l, b, r;
    if (!selectionBox(t, l, b, r)) return;
    m_rowOutline.add(t, b);
    statusBar()->showMessage(QStringLiteral("Đã gom nhóm hàng %1–%2 (Alt+Shift+→ để thu gọn)").arg(t + 1).arg(b + 1), 3000);
}

void MainWindow::ungroupRows()
{
    QModelIndex cur = m_view->currentIndex();
    int row = cur.isValid() ? cur.row() : 0;
    if (m_rowOutline.remove(row)) {
        applyRowOutline();
        statusBar()->showMessage(QStringLiteral("Đã bỏ nhóm hàng"), 2500);
    } else {
        statusBar()->showMessage(QStringLiteral("Ô hiện tại không nằm trong nhóm nào"), 2500);
    }
}

void MainWindow::toggleGroupRows()
{
    QModelIndex cur = m_view->currentIndex();
    int row = cur.isValid() ? cur.row() : 0;
    if (m_rowOutline.isEmpty()) { statusBar()->showMessage(QStringLiteral("Chưa có nhóm hàng nào"), 2500); return; }
    bool collapsed = m_rowOutline.toggle(row);
    applyRowOutline();
    statusBar()->showMessage(collapsed ? QStringLiteral("Đã thu gọn nhóm") : QStringLiteral("Đã mở rộng nhóm"), 2500);
}

// Nhóm cột — đối xứng với nhóm hàng, dùng chung mô hình outline::Outline.
void MainWindow::applyColOutline()
{
    const QSet<int> hidden = m_colOutline.hiddenRows(); // "rows" ở đây là chỉ số cột
    for (int c = 0; c < m_model->columnCount(); ++c)
        m_view->setColumnHidden(c, hidden.contains(c));
}

void MainWindow::groupCols()
{
    int t, l, b, r;
    if (!selectionBox(t, l, b, r)) return;
    m_colOutline.add(l, r);
    statusBar()->showMessage(QStringLiteral("Đã gom nhóm cột %1–%2").arg(l + 1).arg(r + 1), 3000);
}

void MainWindow::ungroupCols()
{
    QModelIndex cur = m_view->currentIndex();
    int col = cur.isValid() ? cur.column() : 0;
    if (m_colOutline.remove(col)) {
        applyColOutline();
        statusBar()->showMessage(QStringLiteral("Đã bỏ nhóm cột"), 2500);
    } else {
        statusBar()->showMessage(QStringLiteral("Ô hiện tại không nằm trong nhóm cột nào"), 2500);
    }
}

void MainWindow::toggleGroupCols()
{
    QModelIndex cur = m_view->currentIndex();
    int col = cur.isValid() ? cur.column() : 0;
    if (m_colOutline.isEmpty()) { statusBar()->showMessage(QStringLiteral("Chưa có nhóm cột nào"), 2500); return; }
    bool collapsed = m_colOutline.toggle(col);
    applyColOutline();
    statusBar()->showMessage(collapsed ? QStringLiteral("Đã thu gọn nhóm cột") : QStringLiteral("Đã mở rộng nhóm cột"), 2500);
}

void MainWindow::toggleShowFormulas(bool on)
{
    m_model->setShowFormulas(on);
}

// ---------------------------------------------------------------- thu phóng
void MainWindow::applyZoom()
{
    m_zoom = zoom::clamp(m_zoom);
    QFont f = theme::cellFont();
    f.setPointSizeF(11.0 * m_zoom / 100.0);
    m_view->setFont(f);
    const int rh = theme::RowHeight * m_zoom / 100, cw = theme::ColWidth * m_zoom / 100;
    m_view->verticalHeader()->setDefaultSectionSize(rh);
    m_view->horizontalHeader()->setDefaultSectionSize(cw);
    for (int r = 0; r < m_model->rowCount(); ++r) m_view->setRowHeight(r, rh);
    for (int c = 0; c < m_model->columnCount(); ++c) m_view->setColumnWidth(c, cw);
    syncZoomWidgets();
}

// Cụm thu phóng bên phải thanh trạng thái: nút −, thanh trượt, nút +, nhãn %.
void MainWindow::buildStatusBarZoom()
{
    auto *minus = new QToolButton(this);
    minus->setText(QStringLiteral("−"));
    minus->setAutoRaise(true);
    connect(minus, &QToolButton::clicked, this, [this] { m_zoom = zoom::stepped(m_zoom, -zoom::kStep); applyZoom(); });

    m_zoomSlider = new QSlider(Qt::Horizontal, this);
    m_zoomSlider->setRange(zoom::kMin, zoom::kMax);
    m_zoomSlider->setValue(m_zoom);
    m_zoomSlider->setFixedWidth(120);
    connect(m_zoomSlider, &QSlider::valueChanged, this, [this](int v) {
        if (v == m_zoom) return;
        m_zoom = zoom::clamp(v); applyZoom();
    });

    auto *plus = new QToolButton(this);
    plus->setText(QStringLiteral("+"));
    plus->setAutoRaise(true);
    connect(plus, &QToolButton::clicked, this, [this] { m_zoom = zoom::stepped(m_zoom, zoom::kStep); applyZoom(); });

    m_zoomLabel = new QLabel(this);
    m_zoomLabel->setMinimumWidth(44);
    m_zoomLabel->setAlignment(Qt::AlignCenter);
    m_zoomLabel->setCursor(Qt::PointingHandCursor);
    m_zoomLabel->setToolTip(QStringLiteral("Bấm để chọn mức thu phóng"));
    m_zoomLabel->installEventFilter(this); // bấm nhãn -> hộp thoại chọn mức

    statusBar()->addPermanentWidget(minus);
    statusBar()->addPermanentWidget(m_zoomSlider);
    statusBar()->addPermanentWidget(plus);
    statusBar()->addPermanentWidget(m_zoomLabel);
    syncZoomWidgets();
}

void MainWindow::syncZoomWidgets()
{
    if (m_zoomSlider) {
        QSignalBlocker block(m_zoomSlider); // tránh vòng lặp valueChanged
        m_zoomSlider->setValue(m_zoom);
    }
    if (m_zoomLabel) m_zoomLabel->setText(QStringLiteral("%1%").arg(m_zoom));
}

bool MainWindow::eventFilter(QObject *obj, QEvent *ev)
{
    if (obj == m_view->viewport() && ev->type() == QEvent::Wheel) {
        auto *we = static_cast<QWheelEvent *>(ev);
        if (we->modifiers() & Qt::ControlModifier) {
            m_zoom = zoom::stepped(m_zoom, we->angleDelta().y() > 0 ? zoom::kStep : -zoom::kStep);
            applyZoom();
            return true; // nuốt sự kiện, không cuộn
        }
    }
    // Kéo nút điền (fill handle) để tự điền chuỗi xuống/sang phải (Spec 05).
    if (obj == m_fillHandle) {
        if (ev->type() == QEvent::MouseButtonPress) {
            if (selectionBox(m_fillT, m_fillL, m_fillB, m_fillR)) m_filling = true;
            return true;
        }
        if (ev->type() == QEvent::MouseButtonRelease && m_filling) {
            auto *me = static_cast<QMouseEvent *>(ev);
            const QPoint vp = m_view->viewport()->mapFromGlobal(m_fillHandle->mapToGlobal(me->pos()));
            m_filling = false;
            doFillDrag(vp);
            positionFillHandle();
            return true;
        }
    }
    // F4 trên thanh công thức: đảo trạng thái khóa $ của tham chiếu tại con trỏ (Spec 04).
    if (obj == m_formulaBar && ev->type() == QEvent::KeyPress) {
        auto *ke = static_cast<QKeyEvent *>(ev);
        if (ke->key() == Qt::Key_F4) {
            const int pos = m_formulaBar->cursorPosition();
            const QString cycled = refcycle::cycleAt(m_formulaBar->text(), pos);
            if (cycled != m_formulaBar->text()) {
                m_formulaBar->setText(cycled);
                m_formulaBar->setCursorPosition(qMin(pos + 2, cycled.size())); // bù dấu $ thêm vào
            }
            return true; // nuốt F4
        }
    }
    if (obj == m_zoomLabel && ev->type() == QEvent::MouseButtonRelease) {
        QStringList items;
        for (int p : zoom::presets()) items << QStringLiteral("%1%").arg(p);
        bool ok = false;
        int idx = zoom::presets().indexOf(m_zoom);
        QString pick = QInputDialog::getItem(this, QStringLiteral("Thu phóng"),
            QStringLiteral("Chọn mức thu phóng:"), items, idx < 0 ? 3 : idx, false, &ok);
        if (ok && !pick.isEmpty()) {
            m_zoom = zoom::clamp(pick.left(pick.size() - 1).toInt());
            applyZoom();
        }
        return true;
    }
    return QMainWindow::eventFilter(obj, ev);
}

// ---------------------------------------------------------------- thống kê vùng chọn
void MainWindow::updateStats()
{
    auto idxs = m_view->selectionModel()->selectedIndexes();
    if (idxs.size() < 2) { m_statsLabel->clear(); return; } // 1 ô thì khỏi hiện
    QVector<QString> vals;
    vals.reserve(idxs.size());
    for (const QModelIndex &i : idxs)
        vals.push_back(m_model->data(i, Qt::DisplayRole).toString());
    stats::Result r = stats::compute(vals);
    if (r.count == 0) { m_statsLabel->clear(); return; }
    QString s = QStringLiteral("Đếm: %1").arg(r.count);
    if (r.numCount > 0) {
        s += QStringLiteral("   Đếm số: %1   Tổng: %2   TB: %3   Trung vị: %4   Nhỏ nhất: %5   Lớn nhất: %6")
                 .arg(r.numCount)
                 .arg(r.sum, 0, 'g', 10).arg(r.avg, 0, 'g', 10).arg(r.median, 0, 'g', 10)
                 .arg(r.min, 0, 'g', 10).arg(r.max, 0, 'g', 10);
        if (r.numCount >= 2) s += QStringLiteral("   Độ lệch chuẩn: %1").arg(r.stdev, 0, 'g', 10);
    }
    m_statsLabel->setText(s);
}

// ---------------------------------------------------------------- tiêu đề
void MainWindow::updateTitle()
{
    QString name = m_currentPath.isEmpty() ? QStringLiteral("Chưa đặt tên")
                                           : QFileInfo(m_currentPath).fileName();
    setWindowTitle(QStringLiteral("Ezcel — %1").arg(name));
}
