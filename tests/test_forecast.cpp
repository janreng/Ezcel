// Test du bao tuyen tinh + trung binh truot (forecast). Chi in ASCII.
#include "model/Forecast.h"
#include <cmath>
#include <cstdio>

static int g_pass = 0, g_fail = 0;
static void ok(bool c, const char *n) { if (c) ++g_pass; else { ++g_fail; std::printf("FAIL %s\n", n); } }
static bool near(double a, double b) { return std::fabs(a - b) < 1e-9; }

int main() {
    // y = 2x + 1: x=1..4 -> y=3,5,7,9. Fit phai ra slope=2, intercept=1.
    QVector<double> x = {1, 2, 3, 4};
    QVector<double> y = {3, 5, 7, 9};
    auto fit = forecast::linearFit(x, y);
    ok(fit.ok && near(fit.slope, 2.0) && near(fit.intercept, 1.0), "linearFit y=2x+1");

    // Du bao 2 ky: x buoc deu = 1 -> x=5,6 -> y=11,13.
    auto fc = forecast::linearForecast(x, y, 2);
    ok(fc.size() == 2, "du bao 2 ky");
    ok(near(fc[0].first, 5) && near(fc[0].second, 11), "ky 1: x=5,y=11");
    ok(near(fc[1].first, 6) && near(fc[1].second, 13), "ky 2: x=6,y=13");

    // Buoc deu khac 1: x=0,10,20 -> step=10 -> du bao x=30.
    QVector<double> x2 = {0, 10, 20};
    QVector<double> y2 = {0, 10, 20}; // y=x
    auto fc2 = forecast::linearForecast(x2, y2, 1);
    ok(fc2.size() == 1 && near(fc2[0].first, 30) && near(fc2[0].second, 30), "buoc deu 10 -> x=30");

    // Thieu du lieu / x trung -> khong fit.
    ok(!forecast::linearFit({1}, {2}).ok, "1 diem -> khong fit");
    ok(!forecast::linearFit({5, 5, 5}, {1, 2, 3}).ok, "x trung nhau -> khong fit");
    ok(forecast::linearForecast(x, y, 0).isEmpty(), "0 ky -> rong");

    // Trung binh truot cua so 3: y=1,2,3,4,5 -> [nan,nan,2,3,4].
    auto ma = forecast::movingAverage({1, 2, 3, 4, 5}, 3);
    ok(ma.size() == 5 && std::isnan(ma[0]) && std::isnan(ma[1]), "MA: 2 dau chua du cua so");
    ok(near(ma[2], 2) && near(ma[3], 3) && near(ma[4], 4), "MA cua so 3 dung");
    ok(forecast::movingAverage({1, 2, 3}, 0).isEmpty(), "window 0 -> rong");

    std::printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
