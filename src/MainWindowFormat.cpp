// Thanh định dạng (P5 batch 2): in đậm/nghiêng/gạch, font, cỡ, màu chữ-nền,
// căn lề, định dạng số — nối vào SpreadsheetModel::setFormat.
// Viền ô cần delegate vẽ nên để ở đợt view/delegate sau.
#include "MainWindow.h"
#include "model/SpreadsheetModel.h"

#include <QTableView>
#include <QToolBar>
#include <QFontComboBox>
#include <QComboBox>
#include <QColorDialog>
#include <QColor>
#include <QAction>

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

    // Kiểu chữ (toggle).
    auto addToggle = [&](const QString &label, const QString &key) {
        QAction *a = tb->addAction(label, this, [this, key] { toggleFormatAttr(key); });
        a->setToolTip(label);
        return a;
    };
    addToggle(QStringLiteral("B"), QStringLiteral("bold"));
    addToggle(QStringLiteral("I"), QStringLiteral("italic"));
    addToggle(QStringLiteral("U"), QStringLiteral("underline"));
    addToggle(QStringLiteral("S"), QStringLiteral("strike"));

    tb->addSeparator();

    // Màu chữ / nền.
    tb->addAction(QStringLiteral("Màu chữ"), this, [this] { pickColor(QStringLiteral("color")); });
    tb->addAction(QStringLiteral("Màu nền"), this, [this] { pickColor(QStringLiteral("bg")); });

    tb->addSeparator();

    // Căn lề.
    tb->addAction(QStringLiteral("◧"), this, [this] { applyFormatAttr(QStringLiteral("halign"), QStringLiteral("left")); })
        ->setToolTip(QStringLiteral("Căn trái"));
    tb->addAction(QStringLiteral("◫"), this, [this] { applyFormatAttr(QStringLiteral("halign"), QStringLiteral("center")); })
        ->setToolTip(QStringLiteral("Căn giữa"));
    tb->addAction(QStringLiteral("◨"), this, [this] { applyFormatAttr(QStringLiteral("halign"), QStringLiteral("right")); })
        ->setToolTip(QStringLiteral("Căn phải"));

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
}
