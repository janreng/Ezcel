#include "model/AutoSum.h"

namespace autosum {

int trailingNumericRun(const QVector<QString> &cells) {
    int run = 0;
    for (int i = cells.size() - 1; i >= 0; --i) {
        bool ok = false;
        cells[i].toDouble(&ok);
        if (!ok) break;
        ++run;
    }
    return run;
}

} // namespace autosum
