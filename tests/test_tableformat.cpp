// Test thuần cho định dạng bảng sọc xen kẽ (Spec 16) — không cần GUI.
#include "model/TableFormat.h"
#include <cstdio>

static int g_pass = 0, g_fail = 0;
static void ok(bool c, const char *n) { if (c) ++g_pass; else { ++g_fail; std::printf("FAIL %s\n", n); } }

int main() {
    tbl::Table t;
    t.top = 1; t.left = 1; t.bottom = 5; t.right = 3; t.header = true;
    t.headerColor = QStringLiteral("#4472C4");
    t.band1 = QStringLiteral("#FFFFFF");
    t.band2 = QStringLiteral("#D9E1F2");

    ok(tbl::stripeColorAt(t, 1, 2) == QLatin1String("#4472C4"), "hang tieu de");
    ok(tbl::stripeColorAt(t, 2, 2) == QLatin1String("#FFFFFF"), "du lieu offset0 = band1");
    ok(tbl::stripeColorAt(t, 3, 2) == QLatin1String("#D9E1F2"), "du lieu offset1 = band2");
    ok(tbl::stripeColorAt(t, 4, 2) == QLatin1String("#FFFFFF"), "du lieu offset2 = band1");
    ok(tbl::stripeColorAt(t, 0, 0).isEmpty(), "ngoai vung -> rong");
    ok(tbl::stripeColorAt(t, 6, 2).isEmpty(), "duoi vung -> rong");
    ok(tbl::stripeColorAt(t, 3, 9).isEmpty(), "ngoai cot -> rong");

    ok(tbl::tableStripeColor(0, QStringLiteral("a"), QStringLiteral("b")) == QLatin1String("a"), "stripe chan");
    ok(tbl::tableStripeColor(1, QStringLiteral("a"), QStringLiteral("b")) == QLatin1String("b"), "stripe le");

    // Không có hàng tiêu đề: banding tính từ đỉnh.
    tbl::Table t2;
    t2.top = 0; t2.left = 0; t2.bottom = 2; t2.right = 0; t2.header = false;
    t2.band1 = QStringLiteral("X"); t2.band2 = QStringLiteral("Y");
    ok(tbl::stripeColorAt(t2, 0, 0) == QLatin1String("X"), "khong header offset0");
    ok(tbl::stripeColorAt(t2, 1, 0) == QLatin1String("Y"), "khong header offset1");

    // Công thức tổng cột.
    ok(tbl::sumFormula(QStringLiteral("B"), 2, 6) == QLatin1String("=SUM(B2:B6)"), "sum formula");
    ok(tbl::sumFormula(QStringLiteral("AA"), 1, 100) == QLatin1String("=SUM(AA1:AA100)"), "sum formula cot AA");

    std::printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
