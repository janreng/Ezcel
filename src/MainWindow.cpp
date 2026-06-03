#include "MainWindow.h"
#include "model/SpreadsheetModel.h"
#include "io/Csv.h"
#include "io/Xlsx.h"
#include "view/MergeSpans.h"
#include "view/CellBorderDelegate.h"
#include "view/Visibility.h"
#include "model/Filter.h"
#include "model/PasteOps.h"
#include "ui/I18n.h"
#include "update/Updater.h"
#include "ui/Theme.h"
#include "model/Stats.h"
#include "model/AutoSum.h"

#include <QTableView>
#include <QHeaderView>
#include <QLineEdit>
#include <QLabel>
#include <QWidget>
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
#include <QInputDialog>
#include <QApplication>
#include <QClipboard>
#include <QFileInfo>
#include <QKeySequence>
#include <QItemSelectionModel>
#include <QItemSelection>
#include <QPoint>
#include <QWheelEvent>
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
    m_view->viewport()->installEventFilter(this); // bắt Ctrl+wheel để zoom

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
    connect(m_sheetTabs, &QTabBar::tabBarDoubleClicked, this, [this](int i) {
        bool ok = false;
        QString n = QInputDialog::getText(this, QStringLiteral("Đổi tên trang"),
            QStringLiteral("Tên trang:"), QLineEdit::Normal, m_sheetTabs->tabText(i), &ok);
        if (ok && !n.isEmpty()) m_sheetTabs->setTabText(i, n);
    });
    connect(m_sheetTabs, &QTabBar::tabCloseRequested, this, [this](int i) {
        if (m_sheets.size() <= 1) return; // luôn giữ ít nhất 1 trang
        m_sheetTabs->blockSignals(true);
        m_sheets[i]->deleteLater();
        m_sheets.remove(i);
        m_sheetTabs->removeTab(i);
        int cur = qBound(0, m_sheetTabs->currentIndex(), m_sheets.size() - 1);
        m_sheetTabs->setCurrentIndex(cur);
        m_sheetTabs->blockSignals(false);
        switchToSheet(cur);
    });

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
    m_statsLabel = new QLabel(this);
    statusBar()->addPermanentWidget(m_statsLabel);

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
                            this, [this] { updateStats(); });
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
    h->addWidget(m_nameBox);
    h->addWidget(fx);
    h->addWidget(m_formulaBar, 1);
    connect(m_formulaBar, &QLineEdit::returnPressed, this, &MainWindow::onFormulaBarCommitted);
    connect(m_nameBox, &QLineEdit::returnPressed, this, &MainWindow::onNameBoxCommitted);
}

void MainWindow::onCurrentCellChanged(const QModelIndex &cur, const QModelIndex &)
{
    if (cur.isValid() && m_nameBox && !m_nameBox->hasFocus())
        m_nameBox->setText(SpreadsheetModel::columnLabel(cur.column()) + QString::number(cur.row() + 1));
    if (m_formulaBar->hasFocus()) return; // đang gõ, đừng đè
    if (cur.isValid())
        m_formulaBar->setText(m_model->data(cur, Qt::EditRole).toString());
    else
        m_formulaBar->clear();
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
    QString first = text.section(':', 0, 0), last = text.section(':', 1, 1);
    QPoint a = parseCellRef(first);
    if (a.x() < 0) { m_view->setFocus(); return; }
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
    m_view->setFocus();
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
    edit->addSeparator();
    edit->addAction(i18n::tr("edit_fill_down"), QKeySequence(QStringLiteral("Ctrl+D")), this, &MainWindow::fillDown);
    edit->addAction(i18n::tr("edit_fill_right"), QKeySequence(QStringLiteral("Ctrl+R")), this, &MainWindow::fillRight);
    edit->addAction(i18n::tr("edit_merge"), this, &MainWindow::toggleMergeSelection);
    edit->addSeparator();
    edit->addAction(i18n::tr("edit_find"), QKeySequence::Find, this, &MainWindow::showFindReplace);
    edit->addAction(i18n::tr("edit_replace"), QKeySequence::Replace, this, &MainWindow::showFindReplace);

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
    bool ok = false;
    QStringList opts{QStringLiteral("Dán bình thường"), QStringLiteral("Chuyển vị (hàng ↔ cột)")};
    QString choice = QInputDialog::getItem(this, QStringLiteral("Dán đặc biệt"),
        QStringLiteral("Kiểu dán:"), opts, 0, false, &ok);
    if (!ok) return;
    if (choice == opts[1]) block = pasteops::transpose(block);
    m_model->pasteBlock(idx.row(), idx.column(), block);
    statusBar()->showMessage(QStringLiteral("Đã dán đặc biệt"), 2000);
}

void MainWindow::clearSelection()
{
    int t, l, b, r;
    if (selectionBox(t, l, b, r)) m_model->clearRange(t, l, b, r);
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

void MainWindow::toggleMergeSelection()
{
    int t, l, b, r;
    if (selectionBox(t, l, b, r)) m_model->toggleMerge(t, l, b, r);
}

void MainWindow::toggleShowFormulas(bool on)
{
    m_model->setShowFormulas(on);
}

// ---------------------------------------------------------------- thu phóng
void MainWindow::applyZoom()
{
    m_zoom = qBound(50, m_zoom, 400);
    QFont f = theme::cellFont();
    f.setPointSizeF(11.0 * m_zoom / 100.0);
    m_view->setFont(f);
    const int rh = theme::RowHeight * m_zoom / 100, cw = theme::ColWidth * m_zoom / 100;
    m_view->verticalHeader()->setDefaultSectionSize(rh);
    m_view->horizontalHeader()->setDefaultSectionSize(cw);
    for (int r = 0; r < m_model->rowCount(); ++r) m_view->setRowHeight(r, rh);
    for (int c = 0; c < m_model->columnCount(); ++c) m_view->setColumnWidth(c, cw);
    statusBar()->showMessage(QStringLiteral("Thu phóng: %1%").arg(m_zoom), 1500);
}

bool MainWindow::eventFilter(QObject *obj, QEvent *ev)
{
    if (obj == m_view->viewport() && ev->type() == QEvent::Wheel) {
        auto *we = static_cast<QWheelEvent *>(ev);
        if (we->modifiers() & Qt::ControlModifier) {
            m_zoom += (we->angleDelta().y() > 0 ? 10 : -10);
            applyZoom();
            return true; // nuốt sự kiện, không cuộn
        }
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
    if (r.numCount > 0)
        s += QStringLiteral("   Tổng: %1   TB: %2")
                 .arg(r.sum, 0, 'g', 10).arg(r.avg, 0, 'g', 10);
    m_statsLabel->setText(s);
}

// ---------------------------------------------------------------- tiêu đề
void MainWindow::updateTitle()
{
    QString name = m_currentPath.isEmpty() ? QStringLiteral("Chưa đặt tên")
                                           : QFileInfo(m_currentPath).fileName();
    setWindowTitle(QStringLiteral("Ezcel — %1").arg(name));
}
