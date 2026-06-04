#include "model/RefCycle.h"
#include <QCoreApplication>
#include <cstdio>

static int g_pass = 0, g_fail = 0;
static void eq(const QString &a, const QString &b, const char *n) {
    if (a == b) ++g_pass;
    else { ++g_fail; std::printf("FAIL %s: '%s' != '%s'\n", n, a.toUtf8().constData(), b.toUtf8().constData()); }
}

int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);
    using namespace refcycle;

    // Vòng A1 -> $A$1 -> A$1 -> $A1 -> A1
    eq(cycle("A1"), "$A$1", "A1 -> $A$1");
    eq(cycle("$A$1"), "A$1", "$A$1 -> A$1");
    eq(cycle("A$1"), "$A1", "A$1 -> $A1");
    eq(cycle("$A1"), "A1", "$A1 -> A1 (het vong)");

    // Giu cot/hang nhieu ky tu.
    eq(cycle("AB12"), "$AB$12", "AB12 -> $AB$12");

    // Khong phai ref -> nguyen van.
    eq(cycle("hello"), "hello", "text giu nguyen");
    eq(cycle("=A1+1"), "=A1+1", "ca cong thuc giu nguyen (khong khop ^...$)");

    // cycleAt: con tro trong token giua cong thuc.
    eq(cycleAt("=A1+B2", 2), "=$A$1+B2", "cycleAt tai A1 trong cong thuc");
    eq(cycleAt("=A1+B2", 5), "=A1+$B$2", "cycleAt tai B2");
    // con tro o cuoi token (sau '1') van bat token A1
    eq(cycleAt("=A1+B2", 3), "=$A$1+B2", "cycleAt o ranh A1/+");
    // khong co ref tai vi tri -> nguyen
    eq(cycleAt("hello world", 3), "hello world", "khong co ref -> nguyen");

    std::printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
