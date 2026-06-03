#include "view/MergeSpans.h"
#include <QTableView>

namespace viewutil {

void applyMergeSpans(QTableView *view, const QVector<MergeRange> &merges)
{
    if (!view) return;
    view->clearSpans(); // bỏ hết span cũ trước khi dựng lại
    for (const MergeRange &m : merges) {
        int rowSpan = m.bottom - m.top + 1;
        int colSpan = m.right - m.left + 1;
        if (rowSpan > 1 || colSpan > 1)
            view->setSpan(m.top, m.left, rowSpan, colSpan);
    }
}

} // namespace viewutil
