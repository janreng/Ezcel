#include "view/HeaderMenu.h"

namespace headermenu {

QVector<int> hiddenSections(const QVector<bool> &hidden, int lo, int hi)
{
    QVector<int> out;
    const int n = hidden.size();
    if (lo < 0) lo = 0;
    if (hi >= n) hi = n - 1;
    for (int i = lo; i <= hi; ++i)
        if (hidden[i]) out.push_back(i);
    return out;
}

bool canUnhide(const QVector<bool> &hidden, int lo, int hi)
{
    return !hiddenSections(hidden, lo, hi).isEmpty();
}

} // namespace headermenu
