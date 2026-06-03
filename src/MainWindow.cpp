#include "MainWindow.h"
#include "model/SpreadsheetModel.h"
#include "io/Csv.h"
#include "io/Xlsx.h"
#include "view/MergeSpans.h"
#include "view/CellBorderDelegate.h"
#include "view/Visibility.h"
#include "update/Updater.h"
#include "ui/Theme.h"

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
#include <QApplication>
#include <QClipboard>
#include <QFileInfo>
#include <QKeySequence>
#include <QItemSelectionModel>
#include <QItemSelection>
#include <QPoint>

// Bộ lọc file dùng chung cho mở/lưu.
static const char *kFileFilter =
    "Bảng tính (*.xlsx *.xlsm *.csv *.txt *.tsv);;Excel (*.xlsx *.xlsm);;CSV/Text (*.csv *.txt *.tsv);;Tất cả (*)";

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    m_model = new SpreadsheetModel(this);
    m_model->resizeGrid(200, 50); // lưới khởi tạo, như bản Python

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

    buildFormulaBar();

    auto *central = new QWidget(this);
    auto *lay = new QVBoxLayout(central);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(0);
    lay->addWidget(m_formulaBar->parentWidget()); // dải thanh công thức
    lay->addWidget(m_view);
    setCentralWidget(central);

    buildMenus();
    buildToolbar();
    buildFormatToolbar();
    statusBar();

    connect(m_view->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &MainWindow::onCurrentCellChanged);
    connect(m_model, &SpreadsheetModel::contentChanged, this, [this] {
        // Cập nhật thanh công thức khi nội dung ô hiện tại đổi (vd undo/redo).
        onCurrentCellChanged(m_view->currentIndex(), QModelIndex());
    });
    // Ô gộp đổi -> cập nhật span của lưới (hiện gộp trực quan).
    connect(m_model, &SpreadsheetModel::mergesChanged, this, [this] {
        viewutil::applyMergeSpans(m_view, m_model->merges());
    });

    updateTitle();
}

MainWindow::~MainWindow() = default;

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
    QMenu *file = menuBar()->addMenu(QStringLiteral("&Tệp"));
    file->addAction(QStringLiteral("&Mới"), QKeySequence::New, this, &MainWindow::newFile);
    file->addAction(QStringLiteral("&Mở..."), QKeySequence::Open, this, &MainWindow::openFile);
    file->addSeparator();
    file->addAction(QStringLiteral("&Lưu"), QKeySequence::Save, this, [this] { saveFile(); });
    file->addAction(QStringLiteral("Lưu &thành..."), QKeySequence::SaveAs, this, [this] { saveFileAs(); });
    file->addSeparator();
    file->addAction(QStringLiteral("T&hoát"), QKeySequence::Quit, this, &QWidget::close);

    QMenu *edit = menuBar()->addMenu(QStringLiteral("&Sửa"));
    edit->addAction(QStringLiteral("&Hoàn tác"), QKeySequence::Undo, this, [this] {
        if (!m_model->undo()) statusBar()->showMessage(QStringLiteral("Không có gì để hoàn tác"), 2000);
    });
    edit->addAction(QStringLiteral("Làm &lại"), QKeySequence::Redo, this, [this] {
        if (!m_model->redo()) statusBar()->showMessage(QStringLiteral("Không có gì để làm lại"), 2000);
    });
    edit->addSeparator();
    edit->addAction(QStringLiteral("Cắt"), QKeySequence::Cut, this, &MainWindow::cutSelection);
    edit->addAction(QStringLiteral("Sao chép"), QKeySequence::Copy, this, &MainWindow::copySelection);
    edit->addAction(QStringLiteral("Dán"), QKeySequence::Paste, this, &MainWindow::pasteClipboard);
    edit->addAction(QStringLiteral("Xóa nội dung"), QKeySequence::Delete, this, &MainWindow::clearSelection);
    edit->addSeparator();
    edit->addAction(QStringLiteral("Điền xuống"), QKeySequence(QStringLiteral("Ctrl+D")), this, &MainWindow::fillDown);
    edit->addAction(QStringLiteral("Điền phải"), QKeySequence(QStringLiteral("Ctrl+R")), this, &MainWindow::fillRight);
    edit->addAction(QStringLiteral("Gộp / bỏ gộp ô"), this, &MainWindow::toggleMergeSelection);
    edit->addSeparator();
    edit->addAction(QStringLiteral("Tìm && Thay thế..."), QKeySequence::Find, this, &MainWindow::showFindReplace);
    edit->addAction(QStringLiteral("Thay thế..."), QKeySequence::Replace, this, &MainWindow::showFindReplace);

    QMenu *st = menuBar()->addMenu(QStringLiteral("&Cấu trúc"));
    st->addAction(QStringLiteral("Chèn hàng trên"), this, [this] {
        int t, l, b, r; if (selectionBox(t, l, b, r)) m_model->insertRows(t, b - t + 1);
    });
    st->addAction(QStringLiteral("Chèn cột trái"), this, [this] {
        int t, l, b, r; if (selectionBox(t, l, b, r)) m_model->insertColumns(l, r - l + 1);
    });
    st->addSeparator();
    st->addAction(QStringLiteral("Xóa hàng"), this, [this] {
        int t, l, b, r; if (selectionBox(t, l, b, r)) m_model->removeRows(t, b - t + 1);
    });
    st->addAction(QStringLiteral("Xóa cột"), this, [this] {
        int t, l, b, r; if (selectionBox(t, l, b, r)) m_model->removeColumns(l, r - l + 1);
    });
    st->addSeparator();
    st->addAction(QStringLiteral("Ẩn hàng"), this, [this] {
        int t, l, b, r; if (selectionBox(t, l, b, r)) viewutil::hideRows(m_view, t, b);
    });
    st->addAction(QStringLiteral("Ẩn cột"), this, [this] {
        int t, l, b, r; if (selectionBox(t, l, b, r)) viewutil::hideCols(m_view, l, r);
    });
    st->addAction(QStringLiteral("Hiện lại (bỏ ẩn)"), this, [this] {
        int t, l, b, r; if (selectionBox(t, l, b, r)) viewutil::unhideRange(m_view, t, l, b, r);
    });

    QMenu *data = menuBar()->addMenu(QStringLiteral("&Dữ liệu"));
    data->addAction(QStringLiteral("Sắp xếp tăng dần"), this, [this] {
        int t, l, b, r; if (!selectionBox(t, l, b, r)) return;
        int kc = m_view->currentIndex().isValid() ? m_view->currentIndex().column() : l;
        m_model->sortRange(t, l, b, r, qBound(l, kc, r), true);
    });
    data->addAction(QStringLiteral("Sắp xếp giảm dần"), this, [this] {
        int t, l, b, r; if (!selectionBox(t, l, b, r)) return;
        int kc = m_view->currentIndex().isValid() ? m_view->currentIndex().column() : l;
        m_model->sortRange(t, l, b, r, qBound(l, kc, r), false);
    });

    QMenu *view = menuBar()->addMenu(QStringLiteral("&Xem"));
    QAction *sf = view->addAction(QStringLiteral("Hiện công thức"));
    sf->setCheckable(true);
    sf->setShortcut(QKeySequence(QStringLiteral("Ctrl+`")));
    connect(sf, &QAction::toggled, this, &MainWindow::toggleShowFormulas);

    QMenu *help = menuBar()->addMenu(QStringLiteral("&Trợ giúp"));
    help->addAction(QStringLiteral("Kiểm tra cập nhật"), this, [this] {
#ifdef EZCEL_VERSION
        const QString ver = QStringLiteral(EZCEL_VERSION);
#else
        const QString ver = QStringLiteral("0.0.0");
#endif
        auto *up = new Updater(ver, this);
        up->checkForUpdates(/*silentIfNone*/ false);
    });
    help->addAction(QStringLiteral("Giới thiệu Ezcel"), this, [this] {
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
    QToolBar *tb = addToolBar(QStringLiteral("Chính"));
    tb->setMovable(false);
    tb->addAction(QStringLiteral("Mới"), this, &MainWindow::newFile);
    tb->addAction(QStringLiteral("Mở"), this, &MainWindow::openFile);
    tb->addAction(QStringLiteral("Lưu"), this, [this] { saveFile(); });
    tb->addSeparator();
    tb->addAction(QStringLiteral("Hoàn tác"), this, [this] { m_model->undo(); });
    tb->addAction(QStringLiteral("Làm lại"), this, [this] { m_model->redo(); });
    tb->addSeparator();
    tb->addAction(QStringLiteral("Gộp ô"), this, &MainWindow::toggleMergeSelection);
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
        xlsxio::Sheet sh;
        if ((ok = xlsxio::loadXlsx(path, sh))) {
            m_model->loadGrid(sh.rows);
            for (const auto &m : sh.merges)
                m_model->mergeCells(m.top, m.left, m.bottom, m.right);
            if (!sh.formats.isEmpty()) m_model->setCellFormats(sh.formats);
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
        QVector<xlsxio::Merge> merges;
        for (const auto &m : m_model->merges()) merges.push_back({m.top, m.left, m.bottom, m.right});
        ok = xlsxio::saveXlsx(path, QFileInfo(path).completeBaseName(), m_model->grid(),
                              merges, m_model->cellFormats());
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

void MainWindow::pasteClipboard()
{
    QModelIndex idx = m_view->currentIndex();
    if (!idx.isValid()) return;
    QString text = QApplication::clipboard()->text();
    if (text.isEmpty()) return;
    QString norm = text;
    norm.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
    norm.replace(QLatin1Char('\r'), QLatin1Char('\n'));
    QStringList lines = norm.split(QLatin1Char('\n'));
    if (!lines.isEmpty() && lines.last().isEmpty()) lines.removeLast();
    QVector<QVector<QString>> block;
    for (const QString &line : lines) {
        QVector<QString> rowv;
        for (const QString &cell : line.split(QLatin1Char('\t'))) rowv.push_back(cell);
        block.push_back(rowv);
    }
    if (block.isEmpty()) return;
    m_model->pasteBlock(idx.row(), idx.column(), block);
    statusBar()->showMessage(QStringLiteral("Đã dán"), 2000);
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

// ---------------------------------------------------------------- tiêu đề
void MainWindow::updateTitle()
{
    QString name = m_currentPath.isEmpty() ? QStringLiteral("Chưa đặt tên")
                                           : QFileInfo(m_currentPath).fileName();
    setWindowTitle(QStringLiteral("Ezcel — %1").arg(name));
}
