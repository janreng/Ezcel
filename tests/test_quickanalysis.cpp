// Test thuần cho Phân tích nhanh (Quick Analysis, Spec 40).
#include "model/QuickAnalysis.h"
#include <cstdio>

static int g_pass = 0, g_fail = 0;
static void ok(bool c, const char *n) { if (c) ++g_pass; else { ++g_fail; std::printf("FAIL %s\n", n); } }

static QVector<QString> row(std::initializer_list<const char *> xs) {
    QVector<QString> r; for (auto x : xs) r << QString::fromUtf8(x); return r;
}

int main() {
    QVector<QVector<QString>> g = { row({"Tên", "Điểm"}), row({"An", "8"}), row({"Bình", "9"}) };

    ok(quickanalysis::hasNumbers(g, 1, 1, 2, 1), "cot diem co so");
    ok(!quickanalysis::hasNumbers(g, 0, 0, 2, 0), "cot ten khong so");

    const QStringList sNum = quickanalysis::suggest(g, 1, 1, 2, 1);
    ok(sNum.size() == 4, "co so -> 4 goi y");
    ok(sNum.contains(QStringLiteral("Tổng cuối vùng")), "co Tong");
    ok(sNum.contains(QStringLiteral("Định dạng là bảng")), "co Bang");

    const QStringList sText = quickanalysis::suggest(g, 0, 0, 2, 0);
    ok(sText.size() == 1 && sText.first() == QStringLiteral("Định dạng là bảng"), "khong so -> chi Bang");

    std::printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
