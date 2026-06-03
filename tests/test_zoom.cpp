// Test tien ich thu phong (zoom). Chi in ASCII.
#include "ui/Zoom.h"
#include <QCoreApplication>
#include <cstdio>

static int g_pass = 0, g_fail = 0;
static void ok(bool c, const char *n) { if (c) ++g_pass; else { ++g_fail; std::printf("FAIL %s\n", n); } }

int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);

    ok(zoom::clamp(5) == 10, "duoi min -> 10");
    ok(zoom::clamp(500) == 400, "tren max -> 400");
    ok(zoom::clamp(123) == 123, "trong khoang giu nguyen");
    ok(zoom::stepped(100, 10) == 110, "tang 1 nac");
    ok(zoom::stepped(100, -10) == 90, "giam 1 nac");
    ok(zoom::stepped(400, 10) == 400, "tang kich max van 400");
    ok(zoom::stepped(10, -10) == 10, "giam kich min van 10");
    ok(zoom::presets().contains(100), "presets co 100");
    ok(zoom::presets().first() >= 10 && zoom::presets().last() <= 400, "presets nam trong [10,400]");

    std::printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
