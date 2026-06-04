#include "model/TextCase.h"
#include <QCoreApplication>
#include <cstdio>

static int g_pass = 0, g_fail = 0;
static void eq(const QString &a, const QString &b, const char *n) {
    if (a == b) ++g_pass;
    else { ++g_fail; std::printf("FAIL %s: '%s' != '%s'\n", n, a.toUtf8().constData(), b.toUtf8().constData()); }
}

int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);
    using namespace textcase;

    eq(convert("hello world", Mode::Upper), "HELLO WORLD", "upper");
    eq(convert("Hello World", Mode::Lower), "hello world", "lower");
    eq(convert("hello world", Mode::Proper), "Hello World", "proper 2 tu");
    eq(convert("HELLO   WORLD", Mode::Proper), "Hello   World", "proper giu khoang trang");
    eq(convert("don't stop", Mode::Proper), "Don'T Stop", "proper tach o dau nhay");
    eq(convert("a1b2 c3", Mode::Proper), "A1B2 C3", "proper: chu sau so cung viet hoa (giong Excel)");
    eq(convert("", Mode::Upper), "", "rong");

    std::printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
