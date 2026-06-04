#include "model/TableFormat.h"

namespace tbl {

QString stripeColorAt(const Table &t, int row, int col)
{
    if (row < t.top || row > t.bottom || col < t.left || col > t.right)
        return QString();
    if (t.header && row == t.top)
        return t.headerColor;
    const int dataTop = t.header ? t.top + 1 : t.top;
    return tableStripeColor(row - dataTop, t.band1, t.band2);
}

} // namespace tbl
