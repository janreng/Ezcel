#include "MainWindow.h"
#include "model/SpreadsheetModel.h"

#include <QTableView>
#include <QHeaderView>

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
    // P4: đọc CSV/XLSX vào model. Tạm thời chưa làm gì.
    Q_UNUSED(path);
}
