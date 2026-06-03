#pragma once
#include <QMainWindow>
#include <QString>

#include <QVector>
#include "view/Outline.h"

class QTableView;
class QLineEdit;
class QLabel;
class QSlider;
class QCheckBox;
class QDialog;
class QTabBar;
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
    void pasteSpecial();      // dán đặc biệt (chuyển vị…)
    void clearSelection();
    void clearFormatsSel();   // xóa chỉ định dạng vùng chọn (Spec 09)
    void clearAllSel();       // xóa cả nội dung + định dạng vùng chọn (Spec 09)
    void fillDown();
    void fillRight();
    void flashFill();         // tự điền theo mẫu (Spec 05)
    void pickFromList();      // chọn từ danh sách trong cột (Alt+Down, Spec 05)
    void filterByValues();    // lọc theo giá trị chọn trong cột (Spec 15)
    void filterByNumber();    // lọc theo điều kiện số (Spec 15)
    void customFilter();      // lọc tùy chỉnh 2 điều kiện AND/OR (Spec 15)
    void sortMultiLevel();    // sắp xếp nhiều cấp (Spec 15)
    void removeDuplicates();  // xóa hàng trùng (Spec 27)
    void textToColumns();     // tách cột theo dấu phân cách (Spec 27)
    void subtotalRange();     // tổng phụ theo nhóm (Spec 27.6)
    void groupRows();         // gom nhóm hàng đang chọn (Spec 09.4)
    void ungroupRows();       // bỏ nhóm hàng tại ô hiện hành
    void toggleGroupRows();   // thu gọn/mở rộng nhóm tại ô hiện hành
    void groupCols();         // gom nhóm cột đang chọn (Spec 09.4)
    void ungroupCols();       // bỏ nhóm cột tại ô hiện hành
    void toggleGroupCols();   // thu gọn/mở rộng nhóm cột tại ô hiện hành
    void toggleMergeSelection();
    void toggleShowFormulas(bool on);
    void showShortcuts();     // hộp thoại bảng phím tắt (Spec 23)
    void showWorkbookStats(); // hộp thoại thống kê bảng tính (Spec 57)
    void showFindReplace();   // định nghĩa ở MainWindowFind.cpp
    void showCondFormat();    // định dạng có điều kiện (MainWindowFind.cpp)
    void showDataValidation();// kiểm tra dữ liệu (MainWindowFind.cpp)
    void findNextFromDialog();
    void replaceOne();
    void replaceAllFromDialog();
    void onCurrentCellChanged(const QModelIndex &cur, const QModelIndex &prev);
    void onFormulaBarCommitted();
    void onNameBoxCommitted();   // gõ ref ô (A1 / A1:B3) -> nhảy/chọn

private:
    void buildMenus();
    void buildContextMenus();    // menu chuột phải ô + đầu hàng/cột (Spec 06)
    void buildToolbar();
    void buildFormatToolbar();   // định nghĩa ở MainWindowFormat.cpp
    void buildFormulaBar();
    void updateTitle();
    void applyZoom();                          // áp mức thu phóng hiện tại
    // --- nhiều trang tính (multi-sheet) ---
    void addSheet(const QString &name = QString());
    void switchToSheet(int i);
    void gotoSheetRelative(int delta); // Ctrl+PageDown/Up (Spec 10)
    void renameSheet(int i);
    void closeSheet(int i);
    void bindActiveModel();                    // nối tín hiệu cho model đang hoạt động
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
    QLabel *m_statsLabel = nullptr; // thống kê vùng chọn ở thanh trạng thái
    QLabel *m_modeLabel = nullptr;  // chỉ báo chế độ ô (Sẵn sàng/Nhập)
    void setCellMode(int mode);     // cập nhật nhãn chế độ ô (cellmode::Mode)
    void updateStats();             // tính lại thống kê vùng chọn
    QString m_currentPath; // rỗng = chưa lưu lần nào
    int m_zoom = 100;      // mức thu phóng (%)
    QSlider *m_zoomSlider = nullptr; // thanh trượt thu phóng (thanh trạng thái)
    QLabel *m_zoomLabel = nullptr;   // nhãn % thu phóng, bấm được
    void buildStatusBarZoom();       // dựng cụm zoom phải thanh trạng thái
    void syncZoomWidgets();          // đồng bộ slider + nhãn theo m_zoom
    QVector<SpreadsheetModel *> m_sheets;      // các trang tính
    QTabBar *m_sheetTabs = nullptr;            // thanh tab trang tính
    QVector<QMetaObject::Connection> m_modelConns; // kết nối tín hiệu model đang hoạt động

    // Hộp thoại Tìm & Thay thế (modeless, dựng lười).
    QDialog *m_findDialog = nullptr;
    QLineEdit *m_findField = nullptr;
    QLineEdit *m_replaceField = nullptr;
    QCheckBox *m_matchCase = nullptr;
    bool findNext(); // nhảy tới ô khớp kế tiếp; trả false nếu không thấy

    outline::Outline m_rowOutline;   // nhóm/phác thảo hàng (Spec 09.4)
    void applyRowOutline();          // áp ẩn/hiện hàng theo nhóm đang thu gọn
    outline::Outline m_colOutline;   // nhóm/phác thảo cột (Spec 09.4)
    void applyColOutline();          // áp ẩn/hiện cột theo nhóm đang thu gọn
};
