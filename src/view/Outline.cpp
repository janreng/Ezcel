#include "view/Outline.h"
#include <QtGlobal>

namespace outline {

void Outline::add(int first, int last)
{
    if (first > last) qSwap(first, last);
    if (first < 0 || last < 0) return;
    for (const Group &g : m_groups)
        if (g.first == first && g.last == last) return; // trùng khít -> bỏ qua
    m_groups.push_back({first, last, false});
}

int Outline::innermostAt(int row) const
{
    int best = -1, bestSpan = 0;
    for (int i = 0; i < m_groups.size(); ++i) {
        const Group &g = m_groups[i];
        if (row < g.first || row > g.last) continue;
        const int span = g.last - g.first;
        if (best == -1 || span < bestSpan) { best = i; bestSpan = span; }
    }
    return best;
}

bool Outline::remove(int row)
{
    int i = innermostAt(row);
    if (i < 0) return false;
    m_groups.remove(i);
    return true;
}

bool Outline::toggle(int row)
{
    int i = innermostAt(row);
    if (i < 0) return false;
    m_groups[i].collapsed = !m_groups[i].collapsed;
    return m_groups[i].collapsed;
}

QSet<int> Outline::hiddenRows() const
{
    QSet<int> hidden;
    for (const Group &g : m_groups)
        if (g.collapsed)
            for (int r = g.first; r <= g.last; ++r) hidden.insert(r);
    return hidden;
}

int Outline::levelOf(int groupIndex) const
{
    if (groupIndex < 0 || groupIndex >= m_groups.size()) return 0;
    const Group &g = m_groups[groupIndex];
    int level = 1;
    for (int i = 0; i < m_groups.size(); ++i) {
        if (i == groupIndex) continue;
        const Group &o = m_groups[i];
        // o chứa thực sự g (bao ngoài) -> tăng độ sâu.
        if (o.first <= g.first && o.last >= g.last &&
            (o.first < g.first || o.last > g.last))
            ++level;
    }
    return level;
}

int Outline::maxLevel() const
{
    int mx = 0;
    for (int i = 0; i < m_groups.size(); ++i) mx = qMax(mx, levelOf(i));
    return mx;
}

} // namespace outline
