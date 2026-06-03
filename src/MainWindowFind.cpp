// Tìm & Thay thế (P5): hộp thoại modeless + nhảy ô khớp + thay 1 / thay tất cả.
#include "MainWindow.h"
#include "model/SpreadsheetModel.h"
#include "model/TextSearch.h"

#include <QTableView>
#include <QDialog>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QGridLayout>
#include <QStatusBar>

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
