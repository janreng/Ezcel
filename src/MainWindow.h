#pragma once
#include <QMainWindow>
#include <QString>

class QTableView;
class QLineEdit;
class QModelIndex;
class SpreadsheetModel;

// Cửa sổ chính: menu (Tệp/Sửa/Xem) + thanh công cụ + thanh công thức + lưới.
// P5 batch 1: mở/lưu file (CSV/XLSX), undo/redo, copy/cut/paste, fill, gộp ô,
// hiện công thức. Ribbon định dạng đầy đủ + tìm/thay thế sẽ port ở đợt sau.
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    // Mở file từ CLI hoặc hộp thoại (CSV/TXT/TSV/XLSX/XLSM).
    void openPath(const QString &path);

private slots:
    void newFile();
    void openFile();
    bool saveFile();      // trả false nếu hủy/lỗi
    bool saveFileAs();
    void copySelection();
    void cutSelection();
    void pasteClipboard();
    void clearSelection();
    void fillDown();
    void fillRight();
    void toggleMergeSelection();
    void toggleShowFormulas(bool on);
    void onCurrentCellChanged(const QModelIndex &cur, const QModelIndex &prev);
    void onFormulaBarCommitted();

private:
    void buildMenus();
    void buildToolbar();
    void buildFormatToolbar();   // định nghĩa ở MainWindowFormat.cpp
    void buildFormulaBar();
    void updateTitle();
    bool saveTo(const QString &path);          // ghi theo phần mở rộng
    void doCopy(bool cut);                      // copy/cut chung một đường
    // Vùng chọn hiện tại dạng [top,left,bottom,right]; false nếu không có.
    bool selectionBox(int &top, int &left, int &bottom, int &right) const;

    // --- định dạng (MainWindowFormat.cpp) ---
    void applyFormatAttr(const QString &key, const QVariant &value); // áp 1 thuộc tính cho vùng chọn
    void toggleFormatAttr(const QString &key);  // bật/tắt bool theo ô hiện hành
    void pickColor(const QString &key);         // "color" (chữ) hoặc "bg" (nền)

    QTableView *m_view = nullptr;
    SpreadsheetModel *m_model = nullptr;
    QLineEdit *m_formulaBar = nullptr;
    QString m_currentPath; // rỗng = chưa lưu lần nào
};
