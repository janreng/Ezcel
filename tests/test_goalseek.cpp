// Test do muc tieu (goal seek - secant solver). Chi in ASCII.
#include "model/GoalSeek.h"
#include <cmath>
#include <cstdio>

static int g_pass = 0, g_fail = 0;
static void ok(bool c, const char *n) { if (c) ++g_pass; else { ++g_fail; std::printf("FAIL %s\n", n); } }
static bool near(double a, double b) { return std::fabs(a - b) < 1e-4; }

int main() {
    // Tuyen tinh: 2x + 1 = 7 -> x = 3
    auto r1 = goalseek::solve([](double x){ return 2 * x + 1; }, 7, 0);
    ok(r1.ok && near(r1.x, 3), "2x+1=7 -> x=3");

    // Bac hai: x^2 = 9, guess gan nghiem duong -> ~3
    auto r2 = goalseek::solve([](double x){ return x * x; }, 9, 2);
    ok(r2.ok && near(std::fabs(r2.x), 3), "x^2=9 -> |x|=3");

    // Phi tuyen: x^3 = 27 -> 3
    auto r3 = goalseek::solve([](double x){ return x * x * x; }, 27, 1);
    ok(r3.ok && near(r3.x, 3), "x^3=27 -> x=3");

    // Guess dung ngay nghiem
    auto r4 = goalseek::solve([](double x){ return 5 * x; }, 50, 10);
    ok(r4.ok && near(r4.x, 10), "5x=50 -> x=10 (guess dung)");

    // Hang so != target -> khong giai duoc (dao ham 0)
    auto r5 = goalseek::solve([](double){ return 4.0; }, 9, 1);
    ok(!r5.ok, "hang so 4 != 9 -> that bai");

    std::printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
