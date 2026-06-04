#pragma once
#include <QObject>

class QTableView;
class QAbstractItemModel;
class QItemSelectionModel;

// Cố định dòng/cột (Freeze Panes, Spec 14) cho một QTableView chính.
// Dựng 3 bảng phụ chồng lên vùng dữ liệu: góc (cố định), hàng trên (cuộn ngang theo
// bảng chính), cột trái (cuộn dọc theo bảng chính). Tất cả dùng chung model + selection.
namespace freeze {

class FreezePanes : public QObject {
    Q_OBJECT
public:
    explicit FreezePanes(QTableView *main, QObject *parent = nullptr);

    // Cố định `rows` hàng đầu + `cols` cột đầu. (0,0) = bỏ cố định.
    void apply(int rows, int cols);
    int frozenRows() const { return m_rows; }
    int frozenCols() const { return m_cols; }

    // Khi đổi trang tính (model/selection thay đổi) -> gắn lại cho các bảng phủ.
    void rebind(QAbstractItemModel *model, QItemSelectionModel *sel);

    // Cập nhật lại vị trí/kích thước (gọi khi resize cửa sổ, đổi section, cuộn).
    void relayout();

protected:
    bool eventFilter(QObject *obj, QEvent *ev) override; // theo dõi resize/scroll bảng chính

private:
    QTableView *makeOverlay();
    void syncSections();   // đồng bộ độ rộng cột / cao hàng từ bảng chính
    void destroyOverlays();

    QTableView *m_main = nullptr;
    QTableView *m_top = nullptr;     // hàng cố định, cuộn ngang
    QTableView *m_left = nullptr;    // cột cố định, cuộn dọc
    QTableView *m_corner = nullptr;  // giao điểm, cố định hoàn toàn
    int m_rows = 0;
    int m_cols = 0;
};

} // namespace freeze
