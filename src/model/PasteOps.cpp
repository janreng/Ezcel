#include "model/PasteOps.h"
#include <QLocale>

namespace pasteops {

namespace {
// Đọc số kiểu C-locale (chấp nhận cả dấu chấm thập phân). Trả false nếu không phải số.
bool toNum(const QString &s, double &out) {
    const QString t = s.trimmed();
    if (t.isEmpty()) return false;
    bool ok = false;
    out = t.toDouble(&ok);
    return ok;
}

// Định dạng số gọn (bỏ đuôi 0 thừa) để ghi lại dạng chuỗi.
QString numStr(double v) {
    return QString::number(v, 'g', 15);
}
} // namespace

QVector<QVector<QString>> applyOperation(const QVector<QVector<QString>> &dest,
                                         const QVector<QVector<QString>> &src,
                                         Op op, bool skipBlanks) {
    QVector<QVector<QString>> out = src;
    for (int r = 0; r < src.size(); ++r) {
        for (int c = 0; c < src[r].size(); ++c) {
            const QString sCell = src[r][c];
            const QString dCell = (r < dest.size() && c < dest[r].size()) ? dest[r][c] : QString();
            const bool srcBlank = sCell.trimmed().isEmpty();

            if (srcBlank && skipBlanks) { out[r][c] = dCell; continue; }
            if (op == Op::None) { out[r][c] = sCell; continue; }

            double dv = 0.0, sv = 0.0;
            const bool sNum = toNum(sCell, sv);
            const bool dNum = toNum(dCell, dv);
            if (!sNum) { out[r][c] = sCell; continue; } // nguồn không phải số -> dán nguyên
            if (!dNum) dv = 0.0;                          // đích trống/không số -> coi như 0

            double res = dv;
            switch (op) {
            case Op::Add:      res = dv + sv; break;
            case Op::Subtract: res = dv - sv; break;
            case Op::Multiply: res = dv * sv; break;
            case Op::Divide:
                if (sv == 0.0) { out[r][c] = dCell; continue; } // chia 0 -> giữ đích
                res = dv / sv; break;
            default: break;
            }
            out[r][c] = numStr(res);
        }
    }
    return out;
}

QString applyConstant(const QString &cell, Op op, double k) {
    double x = 0.0;
    if (!toNum(cell, x)) return cell;       // không phải số -> giữ nguyên
    double res = x;
    switch (op) {
    case Op::Add:      res = x + k; break;
    case Op::Subtract: res = x - k; break;
    case Op::Multiply: res = x * k; break;
    case Op::Divide:   if (k == 0.0) return cell; res = x / k; break;
    default:           return cell;
    }
    return numStr(res);
}

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
