#include "view/Visibility.h"
#include <QTableView>

namespace viewutil {

void hideRows(QTableView *view, int top, int bottom) {
    if (!view) return;
    for (int r = top; r <= bottom; ++r) view->setRowHidden(r, true);
}

void hideCols(QTableView *view, int left, int right) {
    if (!view) return;
    for (int c = left; c <= right; ++c) view->setColumnHidden(c, true);
}

void unhideRange(QTableView *view, int top, int left, int bottom, int right) {
    if (!view) return;
    for (int r = top; r <= bottom; ++r)
        if (view->isRowHidden(r)) view->setRowHidden(r, false);
    for (int c = left; c <= right; ++c)
        if (view->isColumnHidden(c)) view->setColumnHidden(c, false);
}

} // namespace viewutil
