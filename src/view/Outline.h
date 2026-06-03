#pragma once
#include <QVector>
#include <QSet>

// Gom nhóm / phác thảo hàng (Group/Outline, Spec 09.4) — mô hình thuần, test
// headless được. Mỗi nhóm là một khoảng hàng [first, last]; khi "thu gọn" thì
// các hàng trong khoảng bị ẩn. Nhóm có thể lồng nhau (level theo độ sâu lồng).
namespace outline {

struct Group {
    int first = 0;
    int last = 0;
    bool collapsed = false;
};

class Outline {
public:
    // Thêm nhóm cho khoảng [first, last] (chuẩn hóa nếu first>last). Bỏ qua
    // khoảng rỗng/âm và nhóm trùng khít đã có.
    void add(int first, int last);

    // Bỏ nhóm trong cùng (nhỏ nhất) chứa `row`. Trả true nếu có gỡ.
    bool remove(int row);

    // Đảo trạng thái thu gọn của nhóm trong cùng chứa `row`.
    // Trả true nếu sau khi đảo nhóm đó đang thu gọn; false nếu không có nhóm.
    bool toggle(int row);

    // Tập hợp hàng bị ẩn do các nhóm đang thu gọn (hợp của mọi khoảng collapsed).
    QSet<int> hiddenRows() const;

    // Độ sâu lồng của một nhóm (1 = ngoài cùng). Index theo `groups()`.
    int levelOf(int groupIndex) const;

    // Độ sâu lồng lớn nhất hiện có (0 nếu chưa có nhóm).
    int maxLevel() const;

    const QVector<Group> &groups() const { return m_groups; }
    bool isEmpty() const { return m_groups.isEmpty(); }

private:
    // Chỉ số nhóm trong cùng (khoảng nhỏ nhất) chứa `row`; -1 nếu không có.
    int innermostAt(int row) const;

    QVector<Group> m_groups;
};

} // namespace outline
