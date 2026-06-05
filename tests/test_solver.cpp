// Test thuần cho Solver — Bộ giải tối ưu (Spec 50).
#include "model/Solver.h"
#include <cstdio>
#include <cmath>

static int g_pass = 0, g_fail = 0;
static void ok(bool c, const char *n) { if (c) ++g_pass; else { ++g_fail; std::printf("FAIL %s\n", n); } }
static bool near(double a, double b, double eps = 1e-2) { return std::fabs(a - b) < eps; }

int main() {
    using namespace solver;

    // Nhỏ nhất của (x-3)^2 trên [0,10] -> x = 3.
    double xmin = optimize1D([](double x){ return (x-3)*(x-3); }, 0, 10, Goal::Min);
    ok(near(xmin, 3.0), "min (x-3)^2 -> 3");

    // Lớn nhất của -(x-2)^2 trên [0,10] -> x = 2.
    double xmax = optimize1D([](double x){ return -(x-2)*(x-2); }, 0, 10, Goal::Max);
    ok(near(xmax, 2.0), "max -(x-2)^2 -> 2");

    // Bằng giá trị: x^2 = 9 trên [0,10] -> x = 3.
    double xt = optimize1D([](double x){ return x*x; }, 0, 10, Goal::Target, 9.0);
    ok(near(xt, 3.0), "target x^2=9 -> 3");

    // Hàm tuyến tính tăng: Max trên [0,5] -> biên 5; Min -> biên 0.
    ok(near(optimize1D([](double x){ return 2*x+1; }, 0, 5, Goal::Max), 5.0), "max tuyen tinh -> 5");
    ok(near(optimize1D([](double x){ return 2*x+1; }, 0, 5, Goal::Min), 0.0), "min tuyen tinh -> 0");

    // score đúng dấu.
    ok(score(10, Goal::Max, 0) == 10, "score Max");
    ok(score(10, Goal::Min, 0) == -10, "score Min");
    ok(near(score(7, Goal::Target, 10), -3), "score Target = -|7-10|");

    // Khoảng suy biến.
    ok(near(optimize1D([](double x){ return x; }, 4, 4, Goal::Max), 4.0), "khoang suy bien");

    std::printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
