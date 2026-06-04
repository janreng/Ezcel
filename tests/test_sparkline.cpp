// Test sparkline (toa do diem duong + chieu cao cot). Chi in ASCII.
#include "model/Sparkline.h"
#include <cmath>
#include <cstdio>

static int g_pass = 0, g_fail = 0;
static void ok(bool c, const char *n) { if (c) ++g_pass; else { ++g_fail; std::printf("FAIL %s\n", n); } }
static bool near(double a, double b) { return std::fabs(a - b) < 1e-6; }

int main() {
    // Line: values 0,5,10 trong khung 100x20, margin 2 -> innerW=96, innerH=16.
    QVector<double> v = {0, 5, 10};
    auto pts = sparkline::linePoints(v, 100, 20, 2);
    ok(pts.size() == 3, "line 3 diem");
    // x: dau = margin=2; cuoi = margin+innerW = 98; giua = 2+48 = 50
    ok(near(pts[0].x(), 2) && near(pts[2].x(), 98) && near(pts[1].x(), 50), "x trai deu");
    // y: gia tri nho nhat (0) -> day = h-margin = 18; lon nhat (10) -> tren = margin = 2
    ok(near(pts[0].y(), 18), "min -> day (y=18)");
    ok(near(pts[2].y(), 2), "max -> tren (y=2)");
    ok(near(pts[1].y(), 10), "giua -> y=10");

    // 1 diem -> giua khung
    auto one = sparkline::linePoints({7}, 100, 20, 2);
    ok(one.size() == 1 && near(one[0].x(), 50), "1 diem -> giua ngang");

    // Column heights: values 0,5,10 -> innerH=16; frac 0,0.5,1 -> 0,8,16
    auto hs = sparkline::columnHeights(v, 20, 2);
    ok(hs.size() == 3 && near(hs[0], 0) && near(hs[1], 8) && near(hs[2], 16), "cot cao theo min..max");

    // values bang nhau -> range 0 -> cot day du (frac=1)
    auto flat = sparkline::columnHeights({4, 4, 4}, 20, 2);
    ok(flat.size() == 3 && near(flat[0], 16), "values bang nhau -> cot day");

    // rong
    ok(sparkline::linePoints({}, 100, 20, 2).isEmpty(), "rong -> rong");

    std::printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
