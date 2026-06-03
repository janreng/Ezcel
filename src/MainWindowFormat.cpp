// Thanh định dạng (P5 batch 2): in đậm/nghiêng/gạch, font, cỡ, màu chữ-nền,
// căn lề, định dạng số — nối vào SpreadsheetModel::setFormat.
// Viền ô cần delegate vẽ nên để ở đợt view/delegate sau.
#include "MainWindow.h"
#include "model/SpreadsheetModel.h"
#include "model/CellStyles.h"
#include "ui/Theme.h"

#include <QTableView>
#include <QToolBar>
#include <QFontComboBox>
#include <QComboBox>
#include <QColorDialog>
#include <QColor>
#include <QAction>
#include <QIcon>
#include <QSize>

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

// ---------------------------------------------------------------- dựng thanh
void MainWindow::buildFormatToolbar()
{
    QToolBar *tb = addToolBar(QStringLiteral("Định dạng"));
    tb->setMovable(false);
    tb->setStyleSheet(theme::toolbarStyle()); // dải kiểu ribbon
    tb->setToolButtonStyle(Qt::ToolButtonIconOnly);
    tb->setIconSize(QSize(18, 18));

    // Font + cỡ chữ.
    auto *fontBox = new QFontComboBox(tb);
    fontBox->setMaximumWidth(160);
    tb->addWidget(fontBox);
    connect(fontBox, &QFontComboBox::currentFontChanged, this,
            [this](const QFont &f) { applyFormatAttr(QStringLiteral("font"), f.family()); });

    auto *sizeBox = new QComboBox(tb);
    sizeBox->setEditable(true);
    for (int s : {8, 9, 10, 11, 12, 14, 16, 18, 20, 24, 28, 36, 48})
        sizeBox->addItem(QString::number(s));
    sizeBox->setCurrentText(QStringLiteral("11"));
    sizeBox->setMaximumWidth(60);
    tb->addWidget(sizeBox);
    connect(sizeBox, &QComboBox::currentTextChanged, this, [this](const QString &t) {
        bool ok = false; int s = t.toInt(&ok);
        if (ok && s > 0) applyFormatAttr(QStringLiteral("size"), s);
    });

    tb->addSeparator();

    // Kiểu chữ (toggle) — icon Lucide.
    auto ic = [](const QString &name) { return QIcon(QStringLiteral(":/icons/%1.svg").arg(name)); };
    auto addToggle = [&](const QString &icon, const QString &tip, const QString &key) {
        QAction *a = tb->addAction(ic(icon), tip, this, [this, key] { toggleFormatAttr(key); });
        a->setToolTip(tip);
        return a;
    };
    addToggle(QStringLiteral("bold"), QStringLiteral("In đậm"), QStringLiteral("bold"));
    addToggle(QStringLiteral("italic"), QStringLiteral("In nghiêng"), QStringLiteral("italic"));
    addToggle(QStringLiteral("underline"), QStringLiteral("Gạch chân"), QStringLiteral("underline"));
    addToggle(QStringLiteral("strike"), QStringLiteral("Gạch ngang"), QStringLiteral("strike"));

    tb->addSeparator();

    // Màu chữ / nền.
    tb->addAction(ic(QStringLiteral("font_color")), QStringLiteral("Màu chữ"), this, [this] { pickColor(QStringLiteral("color")); });
    tb->addAction(ic(QStringLiteral("fill_color")), QStringLiteral("Màu nền"), this, [this] { pickColor(QStringLiteral("bg")); });

    tb->addSeparator();

    // Căn lề.
    tb->addAction(ic(QStringLiteral("align_left")), QStringLiteral("Căn trái"), this, [this] { applyFormatAttr(QStringLiteral("halign"), QStringLiteral("left")); });
    tb->addAction(ic(QStringLiteral("align_center")), QStringLiteral("Căn giữa"), this, [this] { applyFormatAttr(QStringLiteral("halign"), QStringLiteral("center")); });
    tb->addAction(ic(QStringLiteral("align_right")), QStringLiteral("Căn phải"), this, [this] { applyFormatAttr(QStringLiteral("halign"), QStringLiteral("right")); });

    tb->addSeparator();

    // Định dạng số (preset).
    auto *numBox = new QComboBox(tb);
    numBox->addItem(QStringLiteral("Chung"), QString());
    numBox->addItem(QStringLiteral("Số 1,234.00"), QStringLiteral("#,##0.00"));
    numBox->addItem(QStringLiteral("Phần trăm %"), QStringLiteral("0.00%"));
    numBox->addItem(QStringLiteral("Ngày dd/mm/yyyy"), QStringLiteral("dd/mm/yyyy"));
    numBox->addItem(QStringLiteral("Ngày yyyy-mm-dd"), QStringLiteral("yyyy-mm-dd"));
    numBox->setMaximumWidth(150);
    tb->addWidget(numBox);
    connect(numBox, QOverload<int>::of(&QComboBox::activated), this, [this, numBox](int i) {
        QString code = numBox->itemData(i).toString();
        applyFormatAttr(QStringLiteral("number_format"), code.isEmpty() ? QVariant() : code);
    });

    tb->addSeparator();

    // Kiểu ô dựng sẵn (Cell Styles).
    auto *styleBox = new QComboBox(tb);
    styleBox->addItem(QStringLiteral("Kiểu ô…")); // mục mặc định, không áp
    for (const QString &nm : cellstyles::names()) styleBox->addItem(nm);
    styleBox->setMaximumWidth(120);
    tb->addWidget(styleBox);
    connect(styleBox, QOverload<int>::of(&QComboBox::activated), this, [this, styleBox](int i) {
        if (i <= 0) return;
        int t, l, b, r;
        if (selectionBox(t, l, b, r)) {
            SpreadsheetModel::Format f;
            const auto attrs = cellstyles::style(styleBox->itemText(i));
            for (auto it = attrs.constBegin(); it != attrs.constEnd(); ++it) f.insert(it.key(), it.value());
            m_model->setFormat(t, l, b, r, f);
        }
        styleBox->setCurrentIndex(0); // về lại "Kiểu ô…"
    });
}
