// Bảng phím tắt (Spec 23) — dữ liệu thuần, không phụ thuộc GUI.
// Dùng cho hộp thoại "Phím tắt" (Help/F1) và có thể test headless.
#pragma once
#include <QString>
#include <QList>

namespace shortcuts {

// Một dòng phím tắt: nhóm + tổ hợp phím + mô tả hành vi.
struct Entry {
    QString category; // nhóm: Di chuyển, Chọn, Soạn thảo, Định dạng, Công thức, Tệp & Trang tính
    QString keys;     // ví dụ "Ctrl + S"
    QString desc;     // mô tả tiếng Việt
};

// Toàn bộ phím tắt, đã nhóm sẵn theo thứ tự hiển thị.
QList<Entry> all();

// Danh sách nhóm theo thứ tự xuất hiện (không trùng).
QStringList categories();

} // namespace shortcuts
