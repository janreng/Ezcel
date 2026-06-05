#pragma once
#include <QString>
#include <QStringList>

// Danh sách tệp gần đây (cho trang Tệp / Backstage, Spec 51). Logic thuần.
namespace recentfiles {

// Thêm path lên đầu danh sách: bỏ trùng (đưa lên đầu), giới hạn `max` mục.
inline QStringList add(QStringList list, const QString &path, int max = 8)
{
    if (path.isEmpty()) return list;
    list.removeAll(path);
    list.prepend(path);
    while (list.size() > max) list.removeLast();
    return list;
}

} // namespace recentfiles
