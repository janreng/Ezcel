#include "model/SeriesGen.h"
#include <cstdio>

static int g_pass = 0, g_fail = 0;
static void ok(bool c, const char *n) { if (c) ++g_pass; else { ++g_fail; std::printf("FAIL %s\n", n); } }

int main() {
    using namespace seriesgen;

    auto lin = generate(1, 2, 5, Type::Linear); // 1,3,5,7,9
    ok(lin.size() == 5 && lin[0] == 1 && lin[1] == 3 && lin[4] == 9, "linear 1 buoc 2");

    auto dec = generate(10, -3, 4, Type::Linear); // 10,7,4,1
    ok(dec.size() == 4 && dec[3] == 1, "linear buoc am");

    auto grow = generate(2, 3, 4, Type::Growth); // 2,6,18,54
    ok(grow.size() == 4 && grow[0] == 2 && grow[1] == 6 && grow[3] == 54, "growth x3");

    auto one = generate(5, 99, 1, Type::Linear); // chi 1 phan tu = start
    ok(one.size() == 1 && one[0] == 5, "count 1 -> chi start");

    ok(generate(1, 1, 0, Type::Linear).isEmpty(), "count 0 -> rong");
    ok(generate(1, 1, -3, Type::Growth).isEmpty(), "count am -> rong");

    std::printf("%d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
