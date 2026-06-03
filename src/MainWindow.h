#pragma once
#include <QMainWindow>
#include <QString>

class QTableView;
class QLineEdit;
class QCheckBox;
class QDialog;
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

protected:
    bool eventFilter(QObject *obj, QEvent *ev) override; // Ctrl+wheel -> zoom

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
    void showFindReplace();   // định nghĩa ở MainWindowFind.cpp
    void showCondFormat();    // định dạng có điều kiện (MainWindowFind.cpp)
    void findNextFromDialog();
    void replaceOne();
    void replaceAllFromDialog();
    void onCurrentCellChanged(const QModelIndex &cur, const QModelIndex &prev);
    void onFormulaBarCommitted();
    void onNameBoxCommitted();   // gõ ref ô (A1 / A1:B3) -> nhảy/chọn

private:
    void buildMenus();
    void buildToolbar();
    void buildFormatToolbar();   // định nghĩa ở MainWindowFormat.cpp
    void buildFormulaBar();
    void updateTitle();
    void applyZoom();                          // áp mức thu phóng hiện tại
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
    QLineEdit *m_nameBox = nullptr; // ô địa chỉ (A1) cạnh thanh công thức
    QString m_currentPath; // rỗng = chưa lưu lần nào
    int m_zoom = 100;      // mức thu phóng (%)

    // Hộp thoại Tìm & Thay thế (modeless, dựng lười).
    QDialog *m_findDialog = nullptr;
    QLineEdit *m_findField = nullptr;
    QLineEdit *m_replaceField = nullptr;
    QCheckBox *m_matchCase = nullptr;
    bool findNext(); // nhảy tới ô khớp kế tiếp; trả false nếu không thấy
};
