// Điều hướng trang tính (Spec 10).
#include "ui/SheetNav.h"

namespace sheetnav {

int wrapIndex(int cur, int count, int delta)
{
    if (count <= 0) return cur;
    int next = (cur + delta) % count;
    if (next < 0) next += count;
    return next;
}

} // namespace sheetnav
