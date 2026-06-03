// Test thong ke bang tinh (wbstats::analyze). Chi in ASCII.
#include "ui/WorkbookStats.h"
#include <QCoreApplication>
#include <cstdio>

static int g_pass = 0, g_fail = 0;
static void ok(bool c, const char *n) { if (c) ++g_pass; else { ++g_fail; std::printf("FAIL %s\n", n); } }

int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);

    std::vector<QString> cells = {
        "10", "20", "=A1+A2", "hello world", "", "  ", "foo", "=SUM(A1:A2)", "3.14",
    };
    auto r = wbstats::analyze(cells);
    ok(r.cellsWithData == 7, "7 o khac rong");   // 10,20,=A1+A2,hello world,foo,=SUM,3.14
    ok(r.formulas == 2, "2 cong thuc");
    ok(r.numbers == 3, "3 so (10,20,3.14)");
    ok(r.words == 3, "3 tu (hello,world,foo)");

    // Rong / trang trong
    auto e = wbstats::analyze({"", "   ", "\t"});
    ok(e.cellsWithData == 0 && e.formulas == 0 && e.words == 0, "trang rong = 0");

    // Cong don
    wbstats::Result total;
    wbstats::add(total, r);
    wbstats::add(total, r);
    ok(total.cellsWithData == 14 && total.formulas == 4 && total.words == 6, "cong don 2 trang");

    std::printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
