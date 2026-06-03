// Test so sanh phien ban (updater::parseVersion / isNewer). Chi in ASCII.
#include "update/VersionCompare.h"
#include <QCoreApplication>
#include <cstdio>

static int g_pass = 0, g_fail = 0;
static void ok(bool cond, const char *name) {
    if (cond) ++g_pass; else { ++g_fail; std::printf("FAIL %s\n", name); }
}

int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);

    ok(updater::parseVersion("v0.20.0") == (QVector<int>{0, 20, 0}), "parse v0.20.0");
    ok(updater::parseVersion("1.2") == (QVector<int>{1, 2}), "parse 1.2");
    ok(updater::parseVersion("khong-so") == (QVector<int>{0}), "parse non-number -> [0]");

    ok(updater::isNewer("0.21.0", "0.20.0"), "0.21.0 > 0.20.0");
    ok(updater::isNewer("v0.20.1", "v0.20.0"), "0.20.1 > 0.20.0 (co prefix v)");
    ok(updater::isNewer("1.0.0", "0.99.99"), "1.0.0 > 0.99.99");
    ok(!updater::isNewer("0.20.0", "0.20.0"), "bang nhau -> khong moi hon");
    ok(!updater::isNewer("0.19.9", "0.20.0"), "0.19.9 < 0.20.0");
    ok(updater::isNewer("0.20", "0.20.0") == false, "0.20 == 0.20.0 (thieu so coi nhu 0)");
    ok(updater::isNewer("0.20.1", "0.20"), "0.20.1 > 0.20");

    std::printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
