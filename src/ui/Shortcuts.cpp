// Bảng phím tắt (Spec 23) — danh sách tĩnh, tiếng Việt.
#include "ui/Shortcuts.h"

namespace shortcuts {

static const char *kNav   = "Di chuyển";
static const char *kSel   = "Chọn";
static const char *kEdit  = "Soạn thảo";
static const char *kFmt   = "Định dạng";
static const char *kForm  = "Công thức";
static const char *kFile  = "Tệp & Trang tính";

QList<Entry> all()
{
    return {
        // Di chuyển
        {kNav, QStringLiteral("Phím mũi tên"),       QStringLiteral("Di chuyển 1 ô")},
        {kNav, QStringLiteral("Ctrl + Mũi tên"),     QStringLiteral("Nhảy tới mép vùng dữ liệu")},
        {kNav, QStringLiteral("Home"),               QStringLiteral("Về đầu hàng")},
        {kNav, QStringLiteral("Ctrl + Home"),        QStringLiteral("Về ô A1")},
        {kNav, QStringLiteral("Ctrl + End"),         QStringLiteral("Về ô cuối có dữ liệu")},
        {kNav, QStringLiteral("F5 / Ctrl + G"),      QStringLiteral("Mở hộp thoại Đi tới")},
        {kNav, QStringLiteral("Ctrl + F"),           QStringLiteral("Tìm kiếm")},
        {kNav, QStringLiteral("Ctrl + H"),           QStringLiteral("Tìm và thay thế")},
        {kNav, QStringLiteral("Tab / Shift + Tab"),  QStringLiteral("Sang phải / sang trái")},

        // Chọn
        {kSel, QStringLiteral("Shift + Mũi tên"),        QStringLiteral("Mở rộng vùng chọn 1 ô")},
        {kSel, QStringLiteral("Ctrl + Shift + Mũi tên"), QStringLiteral("Mở rộng tới mép dữ liệu")},
        {kSel, QStringLiteral("Shift + Home"),           QStringLiteral("Mở rộng về đầu hàng")},
        {kSel, QStringLiteral("Ctrl + A"),               QStringLiteral("Chọn toàn bộ trang")},
        {kSel, QStringLiteral("Ctrl + Dấu cách"),        QStringLiteral("Chọn cả cột")},
        {kSel, QStringLiteral("Shift + Dấu cách"),       QStringLiteral("Chọn cả hàng")},

        // Soạn thảo
        {kEdit, QStringLiteral("F2"),               QStringLiteral("Vào chế độ sửa ô")},
        {kEdit, QStringLiteral("Delete"),           QStringLiteral("Xóa nội dung, giữ định dạng")},
        {kEdit, QStringLiteral("Ctrl + Z / Y"),     QStringLiteral("Hoàn tác / Làm lại")},
        {kEdit, QStringLiteral("Ctrl + X / C / V"), QStringLiteral("Cắt / Sao chép / Dán")},
        {kEdit, QStringLiteral("Ctrl + Alt + V"),   QStringLiteral("Dán đặc biệt")},
        {kEdit, QStringLiteral("Ctrl + D"),         QStringLiteral("Điền xuống")},
        {kEdit, QStringLiteral("Ctrl + R"),         QStringLiteral("Điền sang phải")},
        {kEdit, QStringLiteral("Ctrl + ;"),         QStringLiteral("Chèn ngày hôm nay")},
        {kEdit, QStringLiteral("Ctrl + Shift + ;"), QStringLiteral("Chèn giờ hiện tại")},
        {kEdit, QStringLiteral("Ctrl + `"),         QStringLiteral("Bật/tắt hiện công thức")},

        // Định dạng
        {kFmt, QStringLiteral("Ctrl + B"),         QStringLiteral("In đậm")},
        {kFmt, QStringLiteral("Ctrl + I"),         QStringLiteral("In nghiêng")},
        {kFmt, QStringLiteral("Ctrl + U"),         QStringLiteral("Gạch chân")},
        {kFmt, QStringLiteral("Ctrl + 5"),         QStringLiteral("Gạch ngang")},
        {kFmt, QStringLiteral("Ctrl + Shift + $"), QStringLiteral("Định dạng tiền tệ")},
        {kFmt, QStringLiteral("Ctrl + Shift + %"), QStringLiteral("Định dạng phần trăm")},
        {kFmt, QStringLiteral("Ctrl + Shift + ^"), QStringLiteral("Định dạng khoa học")},

        // Công thức
        {kForm, QStringLiteral("="),         QStringLiteral("Bắt đầu nhập công thức")},
        {kForm, QStringLiteral("Alt + ="),   QStringLiteral("Tự động tính tổng (AutoSum)")},
        {kForm, QStringLiteral("Shift + F3"), QStringLiteral("Chèn hàm")},
        {kForm, QStringLiteral("F4"),        QStringLiteral("Đổi kiểu tham chiếu ($)")},

        // Tệp & Trang tính
        {kFile, QStringLiteral("Ctrl + N"),         QStringLiteral("Tạo mới")},
        {kFile, QStringLiteral("Ctrl + O"),         QStringLiteral("Mở tệp")},
        {kFile, QStringLiteral("Ctrl + S"),         QStringLiteral("Lưu")},
        {kFile, QStringLiteral("Ctrl + Shift + S"), QStringLiteral("Lưu thành")},
        {kFile, QStringLiteral("Ctrl + P"),         QStringLiteral("In")},
        {kFile, QStringLiteral("Ctrl + Page Up/Down"), QStringLiteral("Trang tính trước / sau")},
        {kFile, QStringLiteral("F1"),               QStringLiteral("Trợ giúp — bảng phím tắt")},
    };
}

QStringList categories()
{
    QStringList out;
    for (const Entry &e : all())
        if (!out.contains(e.category)) out.append(e.category);
    return out;
}

} // namespace shortcuts
