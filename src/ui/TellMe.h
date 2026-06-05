#pragma once
#include <QString>
#include <QStringList>

// "Bạn muốn làm gì?" (Tell Me, Spec 55): tìm nhanh lệnh theo từ khóa.
// Khớp KHÔNG liên tục (subsequence) không phân biệt hoa/thường — gõ "đậm" hay
// "ddm" đều ra "In đậm". Logic thuần để kiểm thử.
namespace tellme {

// Tất cả ký tự của query xuất hiện theo đúng thứ tự trong name (không cần liền nhau)?
inline bool matchSubsequence(const QString &name, const QString &query)
{
    if (query.isEmpty()) return true;
    const QString n = name.toLower();
    const QString q = query.toLower();
    int j = 0;
    for (int i = 0; i < n.size() && j < q.size(); ++i)
        if (n.at(i) == q.at(j)) ++j;
    return j == q.size();
}

// Lọc các lệnh khớp query, GIỮ nguyên thứ tự đầu vào; tối đa `limit` kết quả.
inline QStringList rank(const QString &query, const QStringList &names, int limit = 12)
{
    QStringList out;
    for (const QString &nm : names) {
        if (matchSubsequence(nm, query)) out << nm;
        if (out.size() >= limit) break;
    }
    return out;
}

} // namespace tellme
