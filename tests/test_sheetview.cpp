// Test thuần cho Khung xem (Sheet View, Spec 56).
#include "model/SheetView.h"
#include <cstdio>

static int g_pass = 0, g_fail = 0;
static void ok(bool c, const char *n) { if (c) ++g_pass; else { ++g_fail; std::printf("FAIL %s\n", n); } }

int main() {
    sheetview::Store s;
    s.save(QStringLiteral("Quý 1"), {2, 3, 5});
    s.save(QStringLiteral("Quý 2"), {0, 1});
    ok(s.count() == 2, "co 2 khung");
    ok(s.names() == QStringList({QStringLiteral("Quý 1"), QStringLiteral("Quý 2")}), "ten dung thu tu");
    ok(s.hiddenOf(QStringLiteral("Quý 1")) == QVector<int>({2, 3, 5}), "hidden Q1");
    ok(s.hiddenOf(QStringLiteral("Quý 2")) == QVector<int>({0, 1}), "hidden Q2");
    ok(s.contains(QStringLiteral("Quý 1")) && !s.contains(QStringLiteral("Lạ")), "contains");

    // Ghi đè cùng tên.
    s.save(QStringLiteral("Quý 1"), {9});
    ok(s.count() == 2 && s.hiddenOf(QStringLiteral("Quý 1")) == QVector<int>({9}), "ghi de cung ten");

    // Tên lạ -> rỗng.
    ok(s.hiddenOf(QStringLiteral("Khong co")).isEmpty(), "ten la -> rong");

    // Xóa.
    ok(s.remove(QStringLiteral("Quý 2")) && s.count() == 1, "xoa khung");
    ok(!s.remove(QStringLiteral("Quý 2")), "xoa lai -> false");

    std::printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
