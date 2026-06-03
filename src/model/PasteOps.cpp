#include "model/PasteOps.h"

namespace pasteops {

QVector<QVector<QString>> transpose(const QVector<QVector<QString>> &block) {
    if (block.isEmpty()) return {};
    int cols = 0;
    for (const auto &row : block) cols = qMax(cols, int(row.size()));
    QVector<QVector<QString>> out(cols, QVector<QString>(block.size()));
    for (int r = 0; r < block.size(); ++r)
        for (int c = 0; c < block[r].size(); ++c)
            out[c][r] = block[r][c];
    return out;
}

} // namespace pasteops
