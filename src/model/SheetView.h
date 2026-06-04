#pragma once
#include <QString>
#include <QStringList>
#include <QVector>

// Khung xem (Sheet View, Spec 56): lưu/khôi phục trạng thái lọc của trang tính
// (tập hàng đang ẩn) theo tên, để chuyển nhanh giữa các góc nhìn. Thuần, test được.
namespace sheetview {

struct View {
    QString name;
    QVector<int> hiddenRows; // các hàng bị ẩn trong khung xem này
};

class Store {
public:
    // Lưu khung xem; trùng tên thì ghi đè.
    void save(const QString &name, const QVector<int> &hidden) {
        for (View &v : m_views)
            if (v.name == name) { v.hiddenRows = hidden; return; }
        m_views.push_back(View{name, hidden});
    }
    QStringList names() const {
        QStringList out;
        for (const View &v : m_views) out << v.name;
        return out;
    }
    QVector<int> hiddenOf(const QString &name) const {
        for (const View &v : m_views)
            if (v.name == name) return v.hiddenRows;
        return {};
    }
    bool contains(const QString &name) const {
        for (const View &v : m_views)
            if (v.name == name) return true;
        return false;
    }
    bool remove(const QString &name) {
        for (int i = 0; i < m_views.size(); ++i)
            if (m_views[i].name == name) { m_views.remove(i); return true; }
        return false;
    }
    int count() const { return int(m_views.size()); }

private:
    QVector<View> m_views;
};

} // namespace sheetview
