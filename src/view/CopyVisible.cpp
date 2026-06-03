#include "view/CopyVisible.h"

namespace copyutil {

QString toTsvSkipHidden(const QVector<QVector<QString>> &block, const QSet<int> &hiddenRows) {
    QString tsv;
    bool firstRow = true;
    for (int i = 0; i < block.size(); ++i) {
        if (hiddenRows.contains(i)) continue; // bỏ hàng ẩn
        if (!firstRow) tsv += QLatin1Char('\n');
        firstRow = false;
        for (int c = 0; c < block[i].size(); ++c) {
            if (c > 0) tsv += QLatin1Char('\t');
            tsv += block[i][c];
        }
    }
    return tsv;
}

} // namespace copyutil
