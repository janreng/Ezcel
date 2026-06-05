// Test thuần cho hộp kiểm trong ô (Form control, Spec 37).
#include "model/FormControl.h"
#include <cstdio>

static int g_pass = 0, g_fail = 0;
static void ok(bool c, const char *n) { if (c) ++g_pass; else { ++g_fail; std::printf("FAIL %s\n", n); } }

int main() {
    ok(formctl::isBool(QStringLiteral("TRUE")), "TRUE la bool");
    ok(formctl::isBool(QStringLiteral(" false ")), "false (co space) la bool");
    ok(!formctl::isBool(QStringLiteral("Có")), "text thuong khong phai bool");
    ok(!formctl::isBool(QStringLiteral("1")), "so khong phai bool");

    ok(formctl::isTrue(QStringLiteral("true")), "true -> dung");
    ok(!formctl::isTrue(QStringLiteral("FALSE")), "FALSE -> sai");

    ok(formctl::toggle(QStringLiteral("TRUE")) == QStringLiteral("FALSE"), "dao TRUE->FALSE");
    ok(formctl::toggle(QStringLiteral("FALSE")) == QStringLiteral("TRUE"), "dao FALSE->TRUE");
    ok(formctl::toggle(QStringLiteral("xyz")) == QStringLiteral("FALSE"), "khong bool -> FALSE");

    std::printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
