// Test thuần cho danh sách tệp gần đây (Spec 51 Backstage).
#include "ui/RecentFiles.h"
#include <cstdio>

static int g_pass = 0, g_fail = 0;
static void ok(bool c, const char *n) { if (c) ++g_pass; else { ++g_fail; std::printf("FAIL %s\n", n); } }

int main() {
    QStringList l;
    l = recentfiles::add(l, QStringLiteral("a.csv"));
    l = recentfiles::add(l, QStringLiteral("b.csv"));
    ok(l == QStringList({QStringLiteral("b.csv"), QStringLiteral("a.csv")}), "moi nhat len dau");

    l = recentfiles::add(l, QStringLiteral("a.csv")); // mở lại a -> lên đầu, không nhân đôi
    ok(l == QStringList({QStringLiteral("a.csv"), QStringLiteral("b.csv")}), "bo trung + len dau");

    // Giới hạn.
    QStringList big;
    for (int i = 0; i < 12; ++i) big = recentfiles::add(big, QStringLiteral("f%1").arg(i), 8);
    ok(big.size() == 8, "gioi han 8");
    ok(big.first() == QStringLiteral("f11"), "moi nhat dau");
    ok(big.last() == QStringLiteral("f4"), "cu nhat bi cat");

    ok(recentfiles::add(l, QString()) == l, "path rong -> giu nguyen");

    std::printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
