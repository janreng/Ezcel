#include "update/VersionCompare.h"
#include <QRegularExpression>

namespace updater {

QVector<int> parseVersion(const QString &text) {
    QVector<int> nums;
    static const QRegularExpression re(QStringLiteral("\\d+"));
    auto it = re.globalMatch(text);
    while (it.hasNext()) nums.push_back(it.next().captured(0).toInt());
    if (nums.isEmpty()) nums.push_back(0);
    return nums;
}

bool isNewer(const QString &latest, const QString &current) {
    QVector<int> a = parseVersion(latest), b = parseVersion(current);
    int n = qMax(a.size(), b.size());
    for (int i = 0; i < n; ++i) {
        int x = i < a.size() ? a[i] : 0;
        int y = i < b.size() ? b[i] : 0;
        if (x != y) return x > y;
    }
    return false;
}

} // namespace updater
