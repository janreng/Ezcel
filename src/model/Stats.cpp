#include "model/Stats.h"
#include <algorithm>

namespace stats {

Result compute(const QVector<QString> &values) {
    Result r;
    std::vector<double> nums;
    for (const QString &v : values) {
        if (v.isEmpty()) continue;
        ++r.count;
        bool ok = false;
        double d = v.toDouble(&ok);
        if (ok) {
            if (r.numCount == 0) { r.min = d; r.max = d; }
            else { if (d < r.min) r.min = d; if (d > r.max) r.max = d; }
            ++r.numCount; r.sum += d;
            nums.push_back(d);
        }
    }
    if (r.numCount > 0) {
        r.avg = r.sum / r.numCount;
        std::sort(nums.begin(), nums.end());
        const int n = int(nums.size());
        r.median = (n % 2 == 1) ? nums[n/2] : (nums[n/2 - 1] + nums[n/2]) / 2.0;
    }
    return r;
}

} // namespace stats
