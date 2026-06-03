#pragma once
#include <QMainWindow>

class QTableView;
class SpreadsheetModel;

// Cửa sổ chính. Hiện mới là khung tối thiểu (lưới + model rỗng).
// Ribbon/menu/formula-bar/find-replace... sẽ port ở P5.
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    // Mở file từ CLI (CSV/XLSX) — cài đặt thật ở P4.
    void openPath(const QString &path);

private:
    QTableView *m_view = nullptr;
    SpreadsheetModel *m_model = nullptr;
};
