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

    // ---- topN ----
    const QStringList vals = {QStringLiteral("10"), QStringLiteral("5"), QStringLiteral("8"),
                              QStringLiteral("1"), QStringLiteral("x")};
    auto top2 = topN(vals, 2, true);
    ok(top2.size() == 2 && top2.contains(0) && top2.contains(2), "top2 = {10,8} -> idx 0,2");
    auto bot2 = topN(vals, 2, false);
    ok(bot2.size() == 2 && bot2.contains(3) && bot2.contains(1), "bottom2 = {1,5} -> idx 3,1");
    ok(!top2.contains(4), "o chu khong vao top");
    ok(topN(vals, 0, true).isEmpty(), "n=0 -> rong");
    ok(topN(vals, 10, true).size() == 4, "n>so o so -> tat ca 4 o so");

    // Đồng hạng ở ngưỡng -> giữ tất cả.
    const QStringList ties = {QStringLiteral("9"), QStringLiteral("9"), QStringLiteral("1")};
    ok(topN(ties, 1, true).size() == 2, "top1 nhung 2 o =9 dong hang -> 2");

    // ---- duplicates ----
    const QStringList dv = {QStringLiteral("a"), QStringLiteral("B"), QStringLiteral("a"),
                            QStringLiteral("b"), QStringLiteral("c"), QStringLiteral("")};
    auto dup = duplicates(dv);
    ok(dup.contains(0) && dup.contains(2), "a lap -> idx 0,2");
    ok(dup.contains(1) && dup.contains(3), "B/b lap (khong phan biet hoa thuong) -> 1,3");
    ok(!dup.contains(4), "c khong lap");
    ok(!dup.contains(5), "o rong bo qua");

    std::printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
