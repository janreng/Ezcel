// Áp định dạng cho vùng chọn (đậm/nghiêng/màu/căn lề/định dạng số…).
// Các điều khiển (font, cỡ, màu, định dạng số, kiểu ô) nay nằm trên dải lệnh
// Ribbon (buildRibbon ở MainWindow.cpp); file này giữ phần áp định dạng dùng chung.
#include "MainWindow.h"
#include "model/SpreadsheetModel.h"
#include "model/BorderOps.h"
#include "model/HighlightRule.h"
#include "ui/Theme.h"

#include <QTableView>
#include <QColorDialog>
#include <QColor>
#include <QDialog>
#include <QTabWidget>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QFontComboBox>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QSharedPointer>
#include <QLineEdit>
#include <QLabel>
#include <QStatusBar>
#include <QSpinBox>

using Format = SpreadsheetModel::Format;

// ---------------------------------------------------------------- áp định dạng
void MainWindow::applyFormatAttr(const QString &key, const QVariant &value)
{
    int t, l, b, r;
    if (!selectionBox(t, l, b, r)) return;
    Format attrs;
    attrs.insert(key, value); // value null -> model xóa key
    m_model->setFormat(t, l, b, r, attrs);
}

// Hộp thoại Định dạng ô (Format Cells) — gộp phông/căn lề/số/viền vào 1 nơi (như Excel, Ctrl+1).
void MainWindow::formatCellsDialog()
{
    int t, l, b, r;
    if (!selectionBox(t, l, b, r)) return;
    const Format cur = m_model->formatAt(t, l);

    QDialog dlg(this);
    dlg.setWindowTitle(QStringLiteral("Định dạng ô"));
    dlg.resize(420, 360);
    auto *lay = new QVBoxLayout(&dlg);
    auto *tabs = new QTabWidget(&dlg);
    lay->addWidget(tabs, 1);

    // ---- Tab Phông ----
    auto *fontTab = new QWidget(&dlg);
    auto *ff = new QFormLayout(fontTab);
    auto *fontBox = new QFontComboBox(fontTab);
    if (cur.contains(QStringLiteral("font"))) fontBox->setCurrentFont(QFont(cur.value(QStringLiteral("font")).toString()));
    auto *sizeBox = new QComboBox(fontTab); sizeBox->setEditable(true);
    for (int s : {8,9,10,11,12,14,16,18,20,24,28,36,48}) sizeBox->addItem(QString::number(s));
    sizeBox->setCurrentText(cur.contains(QStringLiteral("size")) ? cur.value(QStringLiteral("size")).toString() : QStringLiteral("11"));
    auto *bold = new QCheckBox(QStringLiteral("Đậm"), fontTab); bold->setChecked(cur.value(QStringLiteral("bold")).toBool());
    auto *italic = new QCheckBox(QStringLiteral("Nghiêng"), fontTab); italic->setChecked(cur.value(QStringLiteral("italic")).toBool());
    auto *underline = new QCheckBox(QStringLiteral("Gạch chân"), fontTab); underline->setChecked(cur.value(QStringLiteral("underline")).toBool());
    auto fg = QSharedPointer<QString>::create(cur.value(QStringLiteral("color")).toString());
    auto bg = QSharedPointer<QString>::create(cur.value(QStringLiteral("bg")).toString());
    auto *fgBtn = new QPushButton(QStringLiteral("Màu chữ..."), fontTab);
    auto *bgBtn = new QPushButton(QStringLiteral("Màu nền..."), fontTab);
    auto tint = [](QPushButton *btn, const QString &c) {
        if (!c.isEmpty()) btn->setStyleSheet(QStringLiteral("background:%1;").arg(c));
    };
    tint(fgBtn, *fg); tint(bgBtn, *bg);
    connect(fgBtn, &QPushButton::clicked, &dlg, [&dlg, fg, fgBtn, tint] {
        QColor c = QColorDialog::getColor(fg->isEmpty() ? QColor("#000000") : QColor(*fg), &dlg, QStringLiteral("Màu chữ"));
        if (c.isValid()) { *fg = c.name(); tint(fgBtn, *fg); }
    });
    connect(bgBtn, &QPushButton::clicked, &dlg, [&dlg, bg, bgBtn, tint] {
        QColor c = QColorDialog::getColor(bg->isEmpty() ? QColor("#ffffff") : QColor(*bg), &dlg, QStringLiteral("Màu nền"));
        if (c.isValid()) { *bg = c.name(); tint(bgBtn, *bg); }
    });
    ff->addRow(QStringLiteral("Phông chữ:"), fontBox);
    ff->addRow(QStringLiteral("Cỡ chữ:"), sizeBox);
    ff->addRow(bold); ff->addRow(italic); ff->addRow(underline);
    ff->addRow(fgBtn); ff->addRow(bgBtn);
    tabs->addTab(fontTab, QStringLiteral("Phông"));

    // ---- Tab Căn lề ----
    auto *alignTab = new QWidget(&dlg);
    auto *af = new QFormLayout(alignTab);
    auto *hAlign = new QComboBox(alignTab);
    hAlign->addItem(QStringLiteral("Mặc định"), QString());
    hAlign->addItem(QStringLiteral("Trái"), QStringLiteral("left"));
    hAlign->addItem(QStringLiteral("Giữa"), QStringLiteral("center"));
    hAlign->addItem(QStringLiteral("Phải"), QStringLiteral("right"));
    hAlign->setCurrentIndex(qMax(0, hAlign->findData(cur.value(QStringLiteral("halign")).toString())));
    auto *vAlign = new QComboBox(alignTab);
    vAlign->addItem(QStringLiteral("Mặc định"), QString());
    vAlign->addItem(QStringLiteral("Trên"), QStringLiteral("top"));
    vAlign->addItem(QStringLiteral("Giữa"), QStringLiteral("middle"));
    vAlign->addItem(QStringLiteral("Dưới"), QStringLiteral("bottom"));
    vAlign->setCurrentIndex(qMax(0, vAlign->findData(cur.value(QStringLiteral("valign")).toString())));
    auto *wrap = new QCheckBox(QStringLiteral("Tự xuống dòng"), alignTab); wrap->setChecked(cur.value(QStringLiteral("wrap")).toBool());
    af->addRow(QStringLiteral("Căn ngang:"), hAlign);
    af->addRow(QStringLiteral("Căn dọc:"), vAlign);
    af->addRow(wrap);
    tabs->addTab(alignTab, QStringLiteral("Căn lề"));

    // ---- Tab Số ----
    auto *numTab = new QWidget(&dlg);
    auto *nf = new QFormLayout(numTab);
    auto *numBox = new QComboBox(numTab);
    numBox->addItem(QStringLiteral("Chung"), QString());
    numBox->addItem(QStringLiteral("Số 1,234.00"), QStringLiteral("#,##0.00"));
    numBox->addItem(QStringLiteral("Phần trăm %"), QStringLiteral("0.00%"));
    numBox->addItem(QStringLiteral("Ngày dd/mm/yyyy"), QStringLiteral("dd/mm/yyyy"));
    numBox->addItem(QStringLiteral("Tiền tệ $1,234.00"), QStringLiteral("$#,##0.00"));
    numBox->addItem(QStringLiteral("Khoa học 0.00E+00"), QStringLiteral("0.00E+00"));
    numBox->setCurrentIndex(qMax(0, numBox->findData(cur.value(QStringLiteral("number_format")).toString())));
    nf->addRow(QStringLiteral("Kiểu số:"), numBox);
    tabs->addTab(numTab, QStringLiteral("Số"));

    // ---- Tab Viền ----
    auto *bdTab = new QWidget(&dlg);
    auto *bf = new QFormLayout(bdTab);
    auto *bdBox = new QComboBox(bdTab);
    bdBox->addItem(QStringLiteral("(giữ nguyên)"), QString());
    bdBox->addItem(QStringLiteral("Viền tất cả"), QStringLiteral("all"));
    bdBox->addItem(QStringLiteral("Viền ngoài"), QStringLiteral("outline"));
    bdBox->addItem(QStringLiteral("Bỏ viền"), QStringLiteral("none"));
    bf->addRow(QStringLiteral("Kiểu viền:"), bdBox);
    tabs->addTab(bdTab, QStringLiteral("Viền"));

    auto *box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    box->button(QDialogButtonBox::Ok)->setText(QStringLiteral("Áp dụng"));
    box->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("Hủy"));
    connect(box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(box, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    lay->addWidget(box);
    if (dlg.exec() != QDialog::Accepted) return;

    // Gom thuộc tính rồi áp cho vùng chọn.
    Format attrs;
    attrs.insert(QStringLiteral("font"), fontBox->currentFont().family());
    bool okSz = false; int sz = sizeBox->currentText().toInt(&okSz);
    if (okSz && sz > 0) attrs.insert(QStringLiteral("size"), sz);
    attrs.insert(QStringLiteral("bold"), bold->isChecked() ? QVariant(true) : QVariant());
    attrs.insert(QStringLiteral("italic"), italic->isChecked() ? QVariant(true) : QVariant());
    attrs.insert(QStringLiteral("underline"), underline->isChecked() ? QVariant(true) : QVariant());
    if (!fg->isEmpty()) attrs.insert(QStringLiteral("color"), *fg);
    if (!bg->isEmpty()) attrs.insert(QStringLiteral("bg"), *bg);
    attrs.insert(QStringLiteral("halign"), hAlign->currentData().toString().isEmpty() ? QVariant() : hAlign->currentData());
    attrs.insert(QStringLiteral("valign"), vAlign->currentData().toString().isEmpty() ? QVariant() : vAlign->currentData());
    attrs.insert(QStringLiteral("wrap"), wrap->isChecked() ? QVariant(true) : QVariant());
    const QString nfCode = numBox->currentData().toString();
    attrs.insert(QStringLiteral("number_format"), nfCode.isEmpty() ? QVariant() : nfCode);
    m_model->setFormat(t, l, b, r, attrs);

    const QString bd = bdBox->currentData().toString();
    if (!bd.isEmpty()) applyBorder(bd); // viền áp riêng (đặt theo từng ô)
    m_view->viewport()->update();
}

// Tô nổi bật ô theo quy tắc (Highlight Cells Rules, Spec 13): tô nền các ô trong vùng chọn
// thỏa điều kiện (>, <, =, nằm giữa, chứa chữ).
void MainWindow::highlightCellsDialog()
{
    int t, l, b, r;
    if (!selectionBox(t, l, b, r)) return;

    QDialog dlg(this);
    dlg.setWindowTitle(QStringLiteral("Tô nổi bật ô theo điều kiện"));
    auto *form = new QFormLayout(&dlg);
    auto *opBox = new QComboBox(&dlg);
    opBox->addItem(QStringLiteral("Lớn hơn"), int(hlrule::Op::Greater));
    opBox->addItem(QStringLiteral("Nhỏ hơn"), int(hlrule::Op::Less));
    opBox->addItem(QStringLiteral("Bằng"), int(hlrule::Op::Equal));
    opBox->addItem(QStringLiteral("Nằm giữa"), int(hlrule::Op::Between));
    opBox->addItem(QStringLiteral("Chứa chữ"), int(hlrule::Op::Contains));
    auto *v1 = new QLineEdit(&dlg);
    auto *v2 = new QLineEdit(&dlg); v2->setEnabled(false);
    auto *lblV1 = new QLabel(QStringLiteral("Giá trị:"), &dlg);
    auto *lblV2 = new QLabel(QStringLiteral("Đến:"), &dlg);
    auto refresh = [opBox, v2, lblV1, v1](int){
        const auto op = hlrule::Op(opBox->currentData().toInt());
        v2->setEnabled(op == hlrule::Op::Between);
        v1->setPlaceholderText(op == hlrule::Op::Contains ? QStringLiteral("chuỗi cần tìm")
                                                          : QStringLiteral("số"));
    };
    connect(opBox, QOverload<int>::of(&QComboBox::currentIndexChanged), &dlg, refresh);
    refresh(0);
    auto bg = QSharedPointer<QString>::create(QStringLiteral("#FFEB9C")); // vàng nhạt như Excel
    auto *colorBtn = new QPushButton(QStringLiteral("Màu nền..."), &dlg);
    colorBtn->setStyleSheet(QStringLiteral("background:%1;").arg(*bg));
    connect(colorBtn, &QPushButton::clicked, &dlg, [&dlg, bg, colorBtn]{
        QColor c = QColorDialog::getColor(QColor(*bg), &dlg, QStringLiteral("Màu nền tô"));
        if (c.isValid()) { *bg = c.name(); colorBtn->setStyleSheet(QStringLiteral("background:%1;").arg(*bg)); }
    });
    form->addRow(QStringLiteral("Điều kiện:"), opBox);
    form->addRow(lblV1, v1);
    form->addRow(lblV2, v2);
    form->addRow(colorBtn);
    auto *box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    box->button(QDialogButtonBox::Ok)->setText(QStringLiteral("Áp dụng"));
    box->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("Hủy"));
    connect(box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(box, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    form->addRow(box);
    if (dlg.exec() != QDialog::Accepted) return;

    const auto op = hlrule::Op(opBox->currentData().toInt());
    const double a = v1->text().trimmed().toDouble();
    const double bb = v2->text().trimmed().toDouble();
    const QString text = v1->text();
    int hit = 0;
    for (int row = t; row <= b; ++row)
        for (int col = l; col <= r; ++col) {
            const QString cellTxt = m_model->data(m_model->index(row, col), Qt::DisplayRole).toString();
            if (hlrule::matches(cellTxt, op, a, bb, text)) {
                Format f; f.insert(QStringLiteral("bg"), *bg);
                m_model->setFormat(row, col, row, col, f);
                ++hit;
            }
        }
    statusBar()->showMessage(QStringLiteral("Đã tô %1 ô thỏa điều kiện").arg(hit), 4000);
}

// Tô N ô số lớn/nhỏ nhất trong vùng chọn (Top/Bottom N, Spec 13).
void MainWindow::highlightTopBottom()
{
    int t, l, b, r;
    if (!selectionBox(t, l, b, r)) return;
    QStringList cells; QVector<QPair<int,int>> pos;
    for (int row = t; row <= b; ++row)
        for (int col = l; col <= r; ++col) {
            cells << m_model->data(m_model->index(row, col), Qt::DisplayRole).toString();
            pos << qMakePair(row, col);
        }

    QDialog dlg(this);
    dlg.setWindowTitle(QStringLiteral("Tô Top/Bottom N"));
    auto *form = new QFormLayout(&dlg);
    auto *nSpin = new QSpinBox(&dlg); nSpin->setRange(1, 9999); nSpin->setValue(10);
    auto *modeBox = new QComboBox(&dlg);
    modeBox->addItem(QStringLiteral("Lớn nhất (Top)"), true);
    modeBox->addItem(QStringLiteral("Nhỏ nhất (Bottom)"), false);
    auto bg = QSharedPointer<QString>::create(QStringLiteral("#C6EFCE")); // xanh lá nhạt như Excel
    auto *colorBtn = new QPushButton(QStringLiteral("Màu nền..."), &dlg);
    colorBtn->setStyleSheet(QStringLiteral("background:%1;").arg(*bg));
    connect(colorBtn, &QPushButton::clicked, &dlg, [&dlg, bg, colorBtn]{
        QColor c = QColorDialog::getColor(QColor(*bg), &dlg, QStringLiteral("Màu nền"));
        if (c.isValid()) { *bg = c.name(); colorBtn->setStyleSheet(QStringLiteral("background:%1;").arg(*bg)); }
    });
    form->addRow(QStringLiteral("Số ô N:"), nSpin);
    form->addRow(QStringLiteral("Kiểu:"), modeBox);
    form->addRow(colorBtn);
    auto *box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    box->button(QDialogButtonBox::Ok)->setText(QStringLiteral("Áp dụng"));
    box->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("Hủy"));
    connect(box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(box, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    form->addRow(box);
    if (dlg.exec() != QDialog::Accepted) return;

    const QSet<int> sel = hlrule::topN(cells, nSpin->value(), modeBox->currentData().toBool());
    for (int i : sel) {
        Format f; f.insert(QStringLiteral("bg"), *bg);
        m_model->setFormat(pos[i].first, pos[i].second, pos[i].first, pos[i].second, f);
    }
    statusBar()->showMessage(QStringLiteral("Đã tô %1 ô").arg(sel.size()), 4000);
}

// Tô các ô có giá trị TRÙNG LẶP trong vùng chọn (Spec 13).
void MainWindow::highlightDuplicates()
{
    int t, l, b, r;
    if (!selectionBox(t, l, b, r)) return;
    QStringList cells; QVector<QPair<int,int>> pos;
    for (int row = t; row <= b; ++row)
        for (int col = l; col <= r; ++col) {
            cells << m_model->data(m_model->index(row, col), Qt::DisplayRole).toString();
            pos << qMakePair(row, col);
        }
    const QColor c = QColorDialog::getColor(QColor(QStringLiteral("#FFC7CE")), this,
        QStringLiteral("Màu nền cho ô trùng lặp")); // đỏ nhạt như Excel
    if (!c.isValid()) return;
    const QSet<int> sel = hlrule::duplicates(cells);
    for (int i : sel) {
        Format f; f.insert(QStringLiteral("bg"), c.name());
        m_model->setFormat(pos[i].first, pos[i].second, pos[i].first, pos[i].second, f);
    }
    statusBar()->showMessage(QStringLiteral("Đã tô %1 ô trùng lặp").arg(sel.size()), 4000);
}

// Kẻ viền cho vùng chọn (Spec 06). mode: all/outline/top/bottom/left/right/none.
void MainWindow::applyBorder(const QString &mode)
{
    int t, l, b, r;
    if (!selectionBox(t, l, b, r)) return;
    using namespace borderops;

    if (mode == QLatin1String("none")) {
        Format f; f.insert(QStringLiteral("border"), QVariant()); // null -> xóa
        m_model->setFormat(t, l, b, r, f);
        m_view->viewport()->update();
        return;
    }

    int uniform = -1; // -1 = mỗi ô khác nhau (outline)
    if (mode == QLatin1String("all")) uniform = All;
    else if (mode == QLatin1String("top")) uniform = Top;
    else if (mode == QLatin1String("bottom")) uniform = Bottom;
    else if (mode == QLatin1String("left")) uniform = Left;
    else if (mode == QLatin1String("right")) uniform = Right;

    if (uniform >= 0) {
        Format f; f.insert(QStringLiteral("border"), toString(uniform));
        m_model->setFormat(t, l, b, r, f);
    } else { // outline: mỗi ô nhận cạnh biên riêng
        for (int rr = t; rr <= b; ++rr)
            for (int cc = l; cc <= r; ++cc) {
                const int e = outlineEdges(rr, cc, t, l, b, r);
                if (!e) continue;
                Format f; f.insert(QStringLiteral("border"), toString(e));
                m_model->setFormat(rr, cc, rr, cc, f);
            }
    }
    m_view->viewport()->update();
}

void MainWindow::toggleFormatAttr(const QString &key)
{
    QModelIndex cur = m_view->currentIndex();
    bool on = cur.isValid() && m_model->formatAt(cur.row(), cur.column()).value(key).toBool();
    applyFormatAttr(key, on ? QVariant() : QVariant(true)); // null = tắt
}

void MainWindow::pickColor(const QString &key)
{
    int t, l, b, r;
    if (!selectionBox(t, l, b, r)) return;
    QString cur = m_model->formatAt(t, l).value(key).toString();
    QColor init = cur.isEmpty() ? QColor(key == QLatin1String("bg") ? "#ffffff" : "#000000")
                                : QColor(cur);
    QColor c = QColorDialog::getColor(init, this,
        key == QLatin1String("bg") ? QStringLiteral("Màu nền") : QStringLiteral("Màu chữ"));
    if (c.isValid()) applyFormatAttr(key, c.name());
}
