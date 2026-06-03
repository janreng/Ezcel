// Tìm & Thay thế (P5): hộp thoại modeless + nhảy ô khớp + thay 1 / thay tất cả.
#include "MainWindow.h"
#include "model/SpreadsheetModel.h"
#include "model/TextSearch.h"
#include "model/CondFormat.h"
#include "model/Validation.h"

#include <QTableView>
#include <QDialog>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QGridLayout>
#include <QStatusBar>
#include <QComboBox>
#include <QColorDialog>
#include <QDialogButtonBox>

void MainWindow::showFindReplace()
{
    if (!m_findDialog) {
        m_findDialog = new QDialog(this);
        m_findDialog->setWindowTitle(QStringLiteral("Tìm & Thay thế"));
        auto *g = new QGridLayout(m_findDialog);

        g->addWidget(new QLabel(QStringLiteral("Tìm:"), m_findDialog), 0, 0);
        m_findField = new QLineEdit(m_findDialog);
        g->addWidget(m_findField, 0, 1, 1, 2);

        g->addWidget(new QLabel(QStringLiteral("Thay bằng:"), m_findDialog), 1, 0);
        m_replaceField = new QLineEdit(m_findDialog);
        g->addWidget(m_replaceField, 1, 1, 1, 2);

        m_matchCase = new QCheckBox(QStringLiteral("Phân biệt hoa/thường"), m_findDialog);
        g->addWidget(m_matchCase, 2, 1, 1, 2);

        auto *btnFind = new QPushButton(QStringLiteral("Tìm tiếp"), m_findDialog);
        auto *btnRepl = new QPushButton(QStringLiteral("Thay"), m_findDialog);
        auto *btnReplAll = new QPushButton(QStringLiteral("Thay tất cả"), m_findDialog);
        g->addWidget(btnFind, 3, 0);
        g->addWidget(btnRepl, 3, 1);
        g->addWidget(btnReplAll, 3, 2);

        connect(btnFind, &QPushButton::clicked, this, &MainWindow::findNextFromDialog);
        connect(btnRepl, &QPushButton::clicked, this, &MainWindow::replaceOne);
        connect(btnReplAll, &QPushButton::clicked, this, &MainWindow::replaceAllFromDialog);
        connect(m_findField, &QLineEdit::returnPressed, this, &MainWindow::findNextFromDialog);
    }
    m_findDialog->show();
    m_findDialog->raise();
    m_findDialog->activateWindow();
    m_findField->setFocus();
    m_findField->selectAll();
}

bool MainWindow::findNext()
{
    if (!m_findField) return false;
    const QString needle = m_findField->text();
    const bool mc = m_matchCase && m_matchCase->isChecked();
    QModelIndex cur = m_view->currentIndex();
    int sr = cur.isValid() ? cur.row() : 0;
    int sc = cur.isValid() ? cur.column() : -1;

    auto hit = textsearch::findNext(
        m_model->rowCount(), m_model->columnCount(), sr, sc, needle, mc,
        [this](int r, int c) { return m_model->data(m_model->index(r, c), Qt::DisplayRole).toString(); });

    if (!hit) {
        statusBar()->showMessage(QStringLiteral("Không tìm thấy"), 3000);
        return false;
    }
    QModelIndex idx = m_model->index(hit->first, hit->second);
    m_view->setCurrentIndex(idx);
    m_view->scrollTo(idx);
    statusBar()->showMessage(
        QStringLiteral("Thấy ở %1%2").arg(SpreadsheetModel::columnLabel(hit->second)).arg(hit->first + 1), 4000);
    return true;
}

void MainWindow::findNextFromDialog() { findNext(); }

void MainWindow::replaceOne()
{
    const QString find = m_findField->text();
    if (find.isEmpty()) return;
    const bool mc = m_matchCase && m_matchCase->isChecked();
    QModelIndex idx = m_view->currentIndex();
    if (idx.isValid()) {
        QString raw = m_model->data(idx, Qt::EditRole).toString();
        QString nw = textsearch::replaceSubstr(raw, find, m_replaceField->text(), mc);
        if (nw != raw) m_model->setData(idx, nw, Qt::EditRole);
    }
    findNext();
}

void MainWindow::replaceAllFromDialog()
{
    const QString find = m_findField->text();
    if (find.isEmpty()) return;
    const bool mc = m_matchCase && m_matchCase->isChecked();
    int n = m_model->replaceAll(find, m_replaceField->text(), mc);
    statusBar()->showMessage(QStringLiteral("Đã thay %1 ô").arg(n), 4000);
}

void MainWindow::showCondFormat()
{
    int t, l, b, r;
    if (!selectionBox(t, l, b, r)) return;

    QDialog dlg(this);
    dlg.setWindowTitle(QStringLiteral("Định dạng có điều kiện"));
    auto *g = new QGridLayout(&dlg);

    g->addWidget(new QLabel(QStringLiteral("Điều kiện:"), &dlg), 0, 0);
    auto *opBox = new QComboBox(&dlg);
    opBox->addItem(QStringLiteral("Lớn hơn"), int(cond::Op::GreaterThan));
    opBox->addItem(QStringLiteral("Nhỏ hơn"), int(cond::Op::LessThan));
    opBox->addItem(QStringLiteral("Bằng"), int(cond::Op::Equal));
    opBox->addItem(QStringLiteral("Khác"), int(cond::Op::NotEqual));
    opBox->addItem(QStringLiteral("Nằm giữa"), int(cond::Op::Between));
    opBox->addItem(QStringLiteral("Chứa chữ"), int(cond::Op::Contains));
    g->addWidget(opBox, 0, 1, 1, 2);

    g->addWidget(new QLabel(QStringLiteral("Giá trị:"), &dlg), 1, 0);
    auto *v1 = new QLineEdit(&dlg);
    auto *v2 = new QLineEdit(&dlg);
    v2->setPlaceholderText(QStringLiteral("đến (chỉ cho Nằm giữa)"));
    g->addWidget(v1, 1, 1);
    g->addWidget(v2, 1, 2);

    QString chosenBg = QStringLiteral("#FFC7CE"); // mặc định đỏ nhạt
    auto *colorBtn = new QPushButton(QStringLiteral("Chọn màu nền…"), &dlg);
    auto setBtnColor = [&](const QString &c) {
        colorBtn->setStyleSheet(QStringLiteral("background:%1;").arg(c));
    };
    setBtnColor(chosenBg);
    QObject::connect(colorBtn, &QPushButton::clicked, &dlg, [&] {
        QColor c = QColorDialog::getColor(QColor(chosenBg), &dlg, QStringLiteral("Màu nền"));
        if (c.isValid()) { chosenBg = c.name(); setBtnColor(chosenBg); }
    });
    g->addWidget(colorBtn, 2, 1, 1, 2);

    auto *bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    QObject::connect(bb, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(bb, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    g->addWidget(bb, 3, 0, 1, 3);

    if (dlg.exec() != QDialog::Accepted) return;

    cond::Rule rule;
    rule.top = t; rule.left = l; rule.bottom = b; rule.right = r;
    rule.op = cond::Op(opBox->currentData().toInt());
    rule.v1 = v1->text().toDouble();
    rule.v2 = v2->text().toDouble();
    rule.text = v1->text();
    rule.bg = chosenBg;
    m_model->addCondRule(rule);
    statusBar()->showMessage(QStringLiteral("Đã thêm định dạng có điều kiện"), 3000);
}

void MainWindow::showDataValidation()
{
    int t, l, b, r;
    if (!selectionBox(t, l, b, r)) return;

    QDialog dlg(this);
    dlg.setWindowTitle(QStringLiteral("Kiểm tra dữ liệu"));
    auto *g = new QGridLayout(&dlg);

    g->addWidget(new QLabel(QStringLiteral("Cho phép:"), &dlg), 0, 0);
    auto *allowBox = new QComboBox(&dlg);
    allowBox->addItem(QStringLiteral("Bất kỳ"), int(validation::Allow::Any));
    allowBox->addItem(QStringLiteral("Số nguyên"), int(validation::Allow::WholeNumber));
    allowBox->addItem(QStringLiteral("Số thập phân"), int(validation::Allow::Decimal));
    allowBox->addItem(QStringLiteral("Độ dài văn bản"), int(validation::Allow::TextLength));
    g->addWidget(allowBox, 0, 1, 1, 2);

    g->addWidget(new QLabel(QStringLiteral("Điều kiện:"), &dlg), 1, 0);
    auto *opBox = new QComboBox(&dlg);
    opBox->addItem(QStringLiteral("Nằm giữa"), int(validation::Op::Between));
    opBox->addItem(QStringLiteral("Không nằm giữa"), int(validation::Op::NotBetween));
    opBox->addItem(QStringLiteral("Bằng"), int(validation::Op::Equal));
    opBox->addItem(QStringLiteral("Khác"), int(validation::Op::NotEqual));
    opBox->addItem(QStringLiteral("Lớn hơn"), int(validation::Op::Greater));
    opBox->addItem(QStringLiteral("Nhỏ hơn"), int(validation::Op::Less));
    opBox->addItem(QStringLiteral("Lớn hơn hoặc bằng"), int(validation::Op::GreaterEqual));
    opBox->addItem(QStringLiteral("Nhỏ hơn hoặc bằng"), int(validation::Op::LessEqual));
    g->addWidget(opBox, 1, 1, 1, 2);

    g->addWidget(new QLabel(QStringLiteral("Giá trị:"), &dlg), 2, 0);
    auto *v1 = new QLineEdit(&dlg);
    auto *v2 = new QLineEdit(&dlg);
    v2->setPlaceholderText(QStringLiteral("đến (cho Nằm giữa)"));
    g->addWidget(v1, 2, 1);
    g->addWidget(v2, 2, 2);

    auto *bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    QObject::connect(bb, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(bb, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    g->addWidget(bb, 3, 0, 1, 3);

    if (dlg.exec() != QDialog::Accepted) return;
    validation::Rule rule;
    rule.top = t; rule.left = l; rule.bottom = b; rule.right = r;
    rule.allow = validation::Allow(allowBox->currentData().toInt());
    rule.op = validation::Op(opBox->currentData().toInt());
    rule.v1 = v1->text().toDouble();
    rule.v2 = v2->text().toDouble();
    m_model->addValidationRule(rule);
    statusBar()->showMessage(QStringLiteral("Đã đặt kiểm tra dữ liệu cho vùng chọn"), 3000);
}
