#pragma once
#include <QMainWindow>
#include <QString>

#include <QVector>
#include "view/Outline.h"
#include "model/SheetView.h"

namespace freeze { class FreezePanes; }

class QTableView;
class QLineEdit;
class QLabel;
class QSlider;
class QCheckBox;
class QDialog;
class QTabBar;
class QMenu;
class QToolButton;
class QModelIndex;
class QSplitter;
class SpreadsheetModel;
class CellBorderDelegate;
class RibbonBar;

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
    void printSheet();    // in lưới hiện hành qua QPrinter + QPrintDialog (Ctrl+P, Spec 24)
    void copySelection();
    void cutSelection();
    void pasteClipboard();
    void pasteSpecial();      // dán đặc biệt (chuyển vị…)
    void clearSelection();
    void clearFormatsSel();   // xóa chỉ định dạng vùng chọn (Spec 09)
    void clearAllSel();       // xóa cả nội dung + định dạng vùng chọn (Spec 09)
    void fillDown();
    void fillRight();
    void fillSeries();        // điền chuỗi số có bước nhảy (Spec 05)
    void changeCase(int mode);// đổi chữ hoa/thường vùng chọn (Spec 05/30)
    void trimSelection();     // cắt gọn khoảng trắng vùng chọn (Spec 27)
    void cleanSelection();    // bỏ ký tự không in được vùng chọn (Spec 27)
    void reverseRowsSelection(); // đảo ngược thứ tự hàng vùng chọn (Spec 15)
    void reverseColsSelection(); // đảo ngược thứ tự cột vùng chọn (Spec 15)
    void applyConstantSelection(); // áp phép tính với hằng số lên vùng chọn (Spec 13)
    void fillBlanksDownSelection(); // điền ô trống bằng giá trị trên (Spec 27)
    void joinColumnsSelection(); // gộp các cột thành một (Spec 27)
    void selectDuplicates();  // chọn ô trùng giá trị trong cột (Spec 15/27)
    void flashFill();         // tự điền theo mẫu (Spec 05)
    void pickFromList();      // chọn từ danh sách trong cột (Alt+Down, Spec 05)
    void filterByValues();    // lọc theo giá trị chọn trong cột (Spec 15)
    void filterByNumber();    // lọc theo điều kiện số (Spec 15)
    void customFilter();      // lọc tùy chỉnh 2 điều kiện AND/OR (Spec 15)
    void sortMultiLevel();    // sắp xếp nhiều cấp (Spec 15)
    void manageNames();       // quản lý vùng đặt tên (Spec 31)
    void removeDuplicates();  // xóa hàng trùng (Spec 27)
    void textToColumns();     // tách cột theo dấu phân cách (Spec 27)
    void subtotalRange();     // tổng phụ theo nhóm (Spec 27.6)
    void consolidateRanges(); // gộp dữ liệu nhiều vùng theo nhãn (Spec 27)
    void forecastSheet();     // dự báo xu hướng tuyến tính + TB trượt (Spec 27)
    void toggleSplitView();   // bật/tắt chia đôi cửa sổ theo chiều dọc (Spec 14)
    void goalSeekDialog();    // dò mục tiêu: đổi ô nhập để ô công thức đạt giá trị (Spec 28)
    void quickPivot();        // bảng tổng hợp nhanh (pivot, Spec 18)
    void applyCellStyle(const QString &name); // áp kiểu ô dựng sẵn cho vùng chọn (Spec 30)
    void changeDecimals(int delta);           // tăng/giảm chữ số thập phân vùng chọn (Spec 08)
    void formatAsTable();     // định dạng vùng chọn thành bảng sọc xen kẽ (Spec 16)
    void addTableTotalRow();  // thêm hàng tổng (SUM theo cột) dưới bảng hiện hành (Spec 16)
    void saveSheetView();     // lưu khung xem hiện tại (trạng thái lọc) (Spec 56)
    void applySheetView();    // chọn + áp lại một khung xem đã lưu (Spec 56)
    void showSlicer();        // panel lọc trực quan theo giá trị cột (Slicer, Spec 54)
    void autoSum();           // AutoSum (∑) ô hiện hành (Alt+=)
    void sortSelection(bool ascending); // sắp xếp vùng theo cột hiện hành
    void insertToday();       // chèn ngày hôm nay vào ô hiện hành (Ctrl+;)
    void insertNow();         // chèn giờ hiện tại vào ô hiện hành (Ctrl+Shift+;)
    void insertSparkline(int type); // chèn sparkline (sparkline::Type) vào ô hiện hành
    void addDataBarDialog();  // thêm thanh dữ liệu cho vùng chọn
    void addColorScaleDialog(); // thêm thang màu cho vùng chọn
    void insertCellsDialog(); // chèn ô có dịch chuyển phải/xuống (Spec 09)
    void deleteCellsDialog(); // xóa ô có dịch chuyển trái/lên (Spec 09)
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
    void checkAccessibility();// kiểm tra trợ năng: quét vấn đề trình đọc màn hình (Spec 41)
    void quickAnalysis();     // phân tích nhanh: gợi ý thao tác cho vùng chọn (Spec 40)
    void insertCheckbox();    // chèn hộp kiểm (TRUE/FALSE) vào ô hiện hành (Spec 37)
    void showFindReplace();   // định nghĩa ở MainWindowFind.cpp
    void showCondFormat();    // định dạng có điều kiện (MainWindowFind.cpp)
    void showDataValidation();// kiểm tra dữ liệu (MainWindowFind.cpp)
    void findNextFromDialog();
    void findAllFromDialog();    // tìm tất cả ô khớp (Spec 32)
    void replaceOne();
    void replaceAllFromDialog();
    void onCurrentCellChanged(const QModelIndex &cur, const QModelIndex &prev);
    void onFormulaBarCommitted();
    void onNameBoxCommitted();   // gõ ref ô (A1 / A1:B3) -> nhảy/chọn

private:
    void buildMenus();
    void buildContextMenus();    // menu chuột phải ô + đầu hàng/cột (Spec 06)
    void buildRibbon();          // dải lệnh kiểu Excel (Trang đầu/Chèn/Công thức/Dữ liệu/Xem)
    void refreshRibbonDropdowns(); // gắn lại QMenu cho các nút thả menu trên ribbon (sau khi dựng/đổi ngôn ngữ)
    void buildTellMe();          // ô "Bạn muốn làm gì?" tìm & chạy lệnh (Spec 55)
    void buildFormulaBar();
    void updateTitle();
    void applyZoom();                          // áp mức thu phóng hiện tại
    // --- nhiều trang tính (multi-sheet) ---
    void addSheet(const QString &name = QString());
    void installCrossSheet(SpreadsheetModel *m); // gắn tham chiếu chéo sheet (Sheet1!A1)
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

    RibbonBar *m_ribbon = nullptr;  // dải lệnh trên cùng (kiểu Excel)
    // Các menu cấp 1 (dựng trong buildMenus) — tái dùng làm nút thả menu trên ribbon.
    QMenu *m_mFile = nullptr, *m_mEdit = nullptr, *m_mStruct = nullptr, *m_mData = nullptr,
          *m_mView = nullptr, *m_mSettings = nullptr, *m_mHelp = nullptr;
    // Nút thả menu tương ứng trên ribbon (giữ để gắn lại menu khi đổi ngôn ngữ).
    QToolButton *m_dbFile = nullptr, *m_dbEdit = nullptr, *m_dbStruct = nullptr,
                *m_dbData = nullptr, *m_dbView = nullptr, *m_dbSettings = nullptr, *m_dbHelp = nullptr;
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

    // Nút kéo điền (fill handle) ở góc dưới-phải vùng chọn (Spec 05).
    QWidget *m_fillHandle = nullptr;
    bool m_filling = false;
    int m_fillT = 0, m_fillL = 0, m_fillB = 0, m_fillR = 0; // vùng nguồn khi bắt đầu kéo
    void positionFillHandle();       // đặt lại vị trí nút theo vùng chọn
    void doFillDrag(const QPoint &releaseViewportPos); // thực hiện điền khi thả chuột

    freeze::FreezePanes *m_freeze = nullptr; // cố định dòng/cột (Spec 14)
    CellBorderDelegate *m_cellDelegate = nullptr; // để "point mode" chèn địa chỉ ô (Spec 12)
    QSplitter *m_splitter = nullptr;  // bọc lưới để chia đôi cửa sổ (Spec 14)
    QTableView *m_splitView = nullptr; // pane thứ 2 khi chia đôi (chung model + vùng chọn)

    outline::Outline m_rowOutline;   // nhóm/phác thảo hàng (Spec 09.4)
    void applyRowOutline();          // áp ẩn/hiện hàng theo nhóm đang thu gọn
    outline::Outline m_colOutline;   // nhóm/phác thảo cột (Spec 09.4)
    void applyColOutline();          // áp ẩn/hiện cột theo nhóm đang thu gọn
    sheetview::Store m_sheetViews;   // các khung xem đã lưu (Spec 56)
};
