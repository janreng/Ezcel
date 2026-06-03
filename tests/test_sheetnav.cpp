// Test dieu huong trang tinh (sheetnav). Chi in ASCII.
#include "ui/SheetNav.h"
#include <QCoreApplication>
#include <cstdio>

static int g_pass = 0, g_fail = 0;
static void ok(bool c, const char *n) { if (c) ++g_pass; else { ++g_fail; std::printf("FAIL %s\n", n); } }

int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);

    ok(sheetnav::wrapIndex(0, 3, 1) == 1, "0 -> 1 (sau)");
    ok(sheetnav::wrapIndex(1, 3, 1) == 2, "1 -> 2 (sau)");
    ok(sheetnav::wrapIndex(2, 3, 1) == 0, "cuoi -> dau (quay vong)");
    ok(sheetnav::wrapIndex(0, 3, -1) == 2, "dau -> cuoi (quay vong)");
    ok(sheetnav::wrapIndex(2, 3, -1) == 1, "2 -> 1 (truoc)");
    ok(sheetnav::wrapIndex(0, 1, 1) == 0, "mot trang: van 0");
    ok(sheetnav::wrapIndex(0, 1, -1) == 0, "mot trang lui: van 0");
    ok(sheetnav::wrapIndex(0, 0, 1) == 0, "khong trang: tra cur");

    std::printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
