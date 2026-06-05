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

    // ---- optimizeND (nhiều biến) ----
    // Tách biến: min (x-1)^2 + (y-2)^2 -> (1,2). Coordinate descent giải đúng.
    {
        auto f = [](const std::vector<double> &v){ return (v[0]-1)*(v[0]-1) + (v[1]-2)*(v[1]-2); };
        auto r = optimizeND(f, {0,0}, {10,10}, Goal::Min);
        ok(near(r[0], 1.0) && near(r[1], 2.0), "ND min tach bien -> (1,2)");
    }
    // 3 biến tách: max -( (x-3)^2+(y+1)^2+(z-5)^2 ) -> (3,-1,5).
    {
        auto f = [](const std::vector<double> &v){ return -((v[0]-3)*(v[0]-3)+(v[1]+1)*(v[1]+1)+(v[2]-5)*(v[2]-5)); };
        auto r = optimizeND(f, {-10,-10,-10}, {10,10,10}, Goal::Max);
        ok(near(r[0],3.0)&&near(r[1],-1.0)&&near(r[2],5.0), "ND max 3 bien");
    }
    // Ràng buộc bằng phạt: min (x-1)^2+(y-2)^2 với x+y<=2. Nghiệm phải KHẢ THI + tốt.
    {
        auto f = [](const std::vector<double> &v){
            double base = (v[0]-1)*(v[0]-1) + (v[1]-2)*(v[1]-2);
            double viol = v[0]+v[1]-2.0;
            if (viol > 0) base += 1e6 * viol; // phạt khi vi phạm (đang Min)
            return base;
        };
        auto r = optimizeND(f, {0,0}, {10,10}, Goal::Min);
        ok(r[0]+r[1] <= 2.0 + 1e-2, "ND rang buoc x+y<=2 KHA THI");
        ok((r[0]-1)*(r[0]-1)+(r[1]-2)*(r[1]-2) <= 1.0 + 1e-2, "ND rang buoc: muc tieu hop ly");
    }
    // Box constraint qua lo/hi: min (x-5)^2 nhưng x bị chặn <=3 -> x=3.
    {
        auto f = [](const std::vector<double> &v){ return (v[0]-5)*(v[0]-5); };
        auto r = optimizeND(f, {0}, {3}, Goal::Min);
        ok(near(r[0], 3.0), "ND box chan tren x<=3 -> 3");
    }

    std::printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
