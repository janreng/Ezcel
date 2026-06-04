#pragma once
#include <QString>
#include <QPair>
#include <QVector>
#include <functional>
#include <optional>

// Tìm & thay thế văn bản (port _replace_substr / replace_text / _find_text).
// Thuần logic, không dính GUI nên test headless được.
namespace textsearch {

// Thay mọi lần xuất hiện của find -> repl trong text. matchCase=false: bỏ qua hoa/thường.
QString replaceSubstr(const QString &text, const QString &find, const QString &repl, bool matchCase);

// Quét tuần tự từ NGAY SAU ô (startRow,startCol) rồi vòng lại, tìm ô chứa needle.
// cellText(r,c) trả văn bản hiển thị của ô. Trả (row,col) hoặc rỗng nếu không thấy.
std::optional<QPair<int, int>> findNext(
    int rows, int cols, int startRow, int startCol,
    const QString &needle, bool matchCase,
    const std::function<QString(int, int)> &cellText);

// Tìm TẤT CẢ ô chứa needle, quét theo thứ tự hàng-rồi-cột (Find All, Spec 32).
// needle rỗng -> trả rỗng. Trả danh sách (row,col).
QVector<QPair<int, int>> findAll(
    int rows, int cols, const QString &needle, bool matchCase,
    const std::function<QString(int, int)> &cellText);

} // namespace textsearch
