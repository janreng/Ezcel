// Test thuần cho trích tham chiếu công thức (point mode, Spec 12).
#include "model/FormulaRefs.h"
#include <cstdio>

static int g_pass = 0, g_fail = 0;
static void ok(bool c, const char *n) { if (c) ++g_pass; else { ++g_fail; std::printf("FAIL %s\n", n); } }

int main() {
    ok(formularefs::colToIndex(QStringLiteral("A")) == 0, "A=0");
    ok(formularefs::colToIndex(QStringLiteral("Z")) == 25, "Z=25");
    ok(formularefs::colToIndex(QStringLiteral("AA")) == 26, "AA=26");

    int r, c;
    ok(formularefs::parseCell(QStringLiteral("B3"), r, c) && r == 2 && c == 1, "B3 -> (2,1)");
    ok(formularefs::parseCell(QStringLiteral("$D$10"), r, c) && r == 9 && c == 3, "$D$10 -> (9,3)");
    ok(!formularefs::parseCell(QStringLiteral("3"), r, c), "3 khong phai ref");
    ok(!formularefs::parseCell(QStringLiteral("SUM"), r, c), "SUM khong phai ref");

    // Ô đơn trong công thức
    auto v1 = formularefs::extract(QStringLiteral("=A1+B2"), 100, 100);
    ok(v1.size() == 2, "=A1+B2 -> 2 ref");
    ok(v1[0].top == 0 && v1[0].left == 0 && v1[0].bottom == 0 && v1[0].right == 0, "A1 = o (0,0)");
    ok(v1[1].top == 1 && v1[1].left == 1, "B2 = (1,1)");

    // Vùng A1:B3
    auto v2 = formularefs::extract(QStringLiteral("=SUM(A1:B3)"), 100, 100);
    ok(v2.size() == 1, "=SUM(A1:B3) -> 1 vung");
    ok(v2[0].top == 0 && v2[0].left == 0 && v2[0].bottom == 2 && v2[0].right == 1, "A1:B3 = (0,0)-(2,1)");

    // Hàm SUM không bị nhận nhầm là ref; nhiều ref + $.
    auto v3 = formularefs::extract(QStringLiteral("=SUM(A1:A3)*$C$1"), 100, 100);
    ok(v3.size() == 2, "SUM(A1:A3)*$C$1 -> 2 ref");

    // Kẹp trong phạm vi lưới
    auto v4 = formularefs::extract(QStringLiteral("=Z99"), 5, 5);
    ok(v4.size() == 1 && v4[0].top == 4 && v4[0].left == 4, "Z99 kep ve (4,4)");

    // Không có ref
    ok(formularefs::extract(QStringLiteral("=1+2*3"), 10, 10).isEmpty(), "=1+2*3 khong ref");

    std::printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
