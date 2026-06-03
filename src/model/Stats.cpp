#include "model/Stats.h"

namespace stats {

Result compute(const QVector<QString> &values) {
    Result r;
    for (const QString &v : values) {
        if (v.isEmpty()) continue;
        ++r.count;
        bool ok = false;
        double d = v.toDouble(&ok);
        if (ok) {
            if (r.numCount == 0) { r.min = d; r.max = d; }
            else { if (d < r.min) r.min = d; if (d > r.max) r.max = d; }
            ++r.numCount; r.sum += d;
        }
    }
    if (r.numCount > 0) r.avg = r.sum / r.numCount;
    return r;
}

} // namespace stats
