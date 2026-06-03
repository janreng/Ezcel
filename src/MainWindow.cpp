#include "MainWindow.h"
#include "model/SpreadsheetModel.h"
#include "io/Csv.h"
#include "io/Xlsx.h"

#include <QTableView>
#include <QHeaderView>
#include <QFileInfo>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Ezcel");

    m_model = new SpreadsheetModel(this);
    m_model->resizeGrid(200, 50); // lưới khởi tạo, như bản Python

    m_view = new QTableView(this);
    m_view->setModel(m_model);
    m_view->horizontalHeader()->setDefaultSectionSize(90);
    m_view->verticalHeader()->setDefaultSectionSize(22);
    setCentralWidget(m_view);
}

MainWindow::~MainWindow() = default;

void MainWindow::openPath(const QString &path)
{
    const QString suffix = QFileInfo(path).suffix().toLower();
    const QString title = QStringLiteral("Ezcel — %1").arg(QFileInfo(path).fileName());
    if (suffix == "csv" || suffix == "txt" || suffix == "tsv") {
        bool ok = false;
        csvio::Grid rows = csvio::loadCsv(path, &ok);
        if (ok) { m_model->loadGrid(rows); setWindowTitle(title); }
    } else if (suffix == "xlsx" || suffix == "xlsm") {
        xlsxio::Sheet sh;
        if (xlsxio::loadXlsx(path, sh)) {
            m_model->loadGrid(sh.rows);
            QVector<MergeRange> mr;
            for (const auto &m : sh.merges) mr.push_back({m.top, m.left, m.bottom, m.right});
            for (const auto &m : mr) m_model->mergeCells(m.top, m.left, m.bottom, m.right);
            setWindowTitle(title);
        }
    }
}
