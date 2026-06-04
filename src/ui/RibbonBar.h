#pragma once
#include <QTabWidget>
#include <QStringList>
#include <QHash>
#include <functional>

class QWidget;
class QHBoxLayout;
class QGridLayout;
class QToolButton;
class QMenu;

// RibbonBar — dải lệnh kiểu Microsoft Office: thanh tab ngang (Trang đầu, Chèn,
// Công thức, Dữ liệu, Xem); mỗi tab chứa các NHÓM lệnh có tiêu đề nhỏ bên dưới,
// ngăn cách bằng vạch dọc. Thay cho dãy menu/thanh công cụ phẳng dài.
//
// Dùng theo lối dựng tuần tự:
//   beginTab("Trang đầu");
//     beginGroup("Phông");
//       addButton("bold", "Đậm", []{...});
//       addWidget(fontCombo);
//   finish();
class RibbonBar : public QTabWidget
{
    Q_OBJECT
public:
    explicit RibbonBar(QWidget *parent = nullptr);

    void beginTab(const QString &title);   // mở tab mới; nhóm/nút tiếp theo vào tab này
    void beginGroup(const QString &title); // mở nhóm trong tab hiện hành
    // Nút lệnh LỚN icon-trên-chữ (chiếm trọn 3 hàng 1 cột — kiểu nút nổi bật Excel).
    QToolButton *addButton(const QString &iconName, const QString &text,
                           const std::function<void()> &onClick);
    // Nút lệnh NHỎ icon-cạnh-chữ, xếp tối đa 3 hàng/cột rồi sang cột mới (kiểu Excel).
    QToolButton *addSmallButton(const QString &iconName, const QString &text,
                                const std::function<void()> &onClick);
    // Nút thả menu LỚN kiểu Excel: bấm bung QMenu (Tệp/Sửa/…). menu đổi sau qua setMenu.
    QToolButton *addMenuButton(const QString &iconName, const QString &text, QMenu *menu);
    void addWidget(QWidget *w);            // chèn widget (combo phông/cỡ...) chiếm 1 hàng nhỏ
    void addSpacer(int px = 6);            // khoảng đệm nhỏ trong nhóm
    void finish();                         // hoàn tất tab cuối (gọi sau khi dựng xong)

    // --- truy vấn cấu trúc (phục vụ kiểm thử) ---
    QStringList tabTitles() const;             // nhãn các tab theo thứ tự
    QStringList groupTitles(int tabIndex) const; // tiêu đề các nhóm trong 1 tab

private:
    void closeCurrentTab(); // thêm đệm co giãn cuối tab đang dựng

    void closeSmallColumn();         // nếu cột nhỏ đang dở -> sang cột mới (trước khi đặt nút lớn)

    QWidget *m_tabContent = nullptr; // widget nội dung tab đang dựng
    QHBoxLayout *m_tabLay = nullptr; // layout ngang chứa các nhóm + separator
    int m_tabIndex = -1;             // chỉ số tab đang dựng
    bool m_groupOpen = false;        // đã có ít nhất 1 nhóm trong tab hiện hành
    QGridLayout *m_groupGrid = nullptr; // lưới 3 hàng chứa nút/widget của nhóm hiện hành
    int m_col = 0;                   // cột đang đặt trong nhóm
    int m_smallRow = 0;              // hàng nhỏ kế tiếp (0..2) trong cột hiện hành
    QHash<int, QStringList> m_groupTitles; // tabIndex -> danh sách tiêu đề nhóm
};
