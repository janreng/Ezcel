// Test thuần cho Tô nổi bật ô theo quy tắc (Highlight Cells Rules, Spec 13).
#include "model/HighlightRule.h"
#include <cstdio>

static int g_pass = 0, g_fail = 0;
static void ok(bool c, const char *n) { if (c) ++g_pass; else { ++g_fail; std::printf("FAIL %s\n", n); } }

int main() {
    using namespace hlrule;
    const QString s5 = QStringLiteral("5");

    ok(matches(s5, Op::Greater, 3), "5 > 3");
    ok(!matches(s5, Op::Greater, 5), "5 > 5 sai");
    ok(matches(s5, Op::Less, 10), "5 < 10");
    ok(!matches(s5, Op::Less, 5), "5 < 5 sai");
    ok(matches(s5, Op::Equal, 5), "5 = 5");
    ok(!matches(s5, Op::Equal, 4), "5 = 4 sai");

    ok(matches(QStringLiteral("7"), Op::Between, 5, 10), "7 trong [5,10]");
    ok(matches(QStringLiteral("5"), Op::Between, 5, 10), "5 biên trái");
    ok(matches(QStringLiteral("10"), Op::Between, 10, 5), "10 biên (a,b đảo)");
    ok(!matches(QStringLiteral("11"), Op::Between, 5, 10), "11 ngoài [5,10]");

    // Ô không phải số -> các phép số không khớp.
    ok(!matches(QStringLiteral("abc"), Op::Greater, 0), "abc khong phai so");

    // Contains (chữ).
    ok(matches(QStringLiteral("Hà Nội"), Op::Contains, 0, 0, QStringLiteral("nội")), "chua 'nội' (khong hoa thuong)");
    ok(!matches(QStringLiteral("Hà Nội"), Op::Contains, 0, 0, QStringLiteral("sài")), "khong chua 'sài'");
    ok(!matches(QStringLiteral("x"), Op::Contains, 0, 0, QStringLiteral("  ")), "text rong -> khong khop");

    // Số âm / thập phân.
    ok(matches(QStringLiteral("-2.5"), Op::Less, 0), "-2.5 < 0");
    ok(matches(QStringLiteral("3.14"), Op::Greater, 3), "3.14 > 3");

    std::printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
