// Test thuần cho biểu đồ (Spec 19): rút chuỗi + trục.
#include "model/Chart.h"
#include <cstdio>
#include <cmath>

static int g_pass = 0, g_fail = 0;
static void ok(bool c, const char *n) { if (c) ++g_pass; else { ++g_fail; std::printf("FAIL %s\n", n); } }

static QVector<QString> row(std::initializer_list<const char *> xs) {
    QVector<QString> r; for (auto x : xs) r << QString::fromUtf8(x); return r;
}

int main() {
    QVector<QVector<QString>> g = { row({"Q1", "10"}), row({"Q2", "25"}), row({"Q3", "abc"}) };

    // 2 cột: nhãn trái, giá trị phải; ô lỗi -> 0.
    chart::Series s = chart::extractSeries(g, 0, 0, 2, 1);
    ok(s.labels == QStringList({"Q1", "Q2", "Q3"}), "nhan dung");
    ok(s.values == QVector<double>({10, 25, 0}), "gia tri (abc->0)");

    // 1 cột: nhãn = số thứ tự.
    chart::Series s1 = chart::extractSeries(g, 0, 1, 1, 1);
    ok(s1.labels == QStringList({"1", "2"}), "1 cot -> nhan so thu tu");
    ok(s1.values == QVector<double>({10, 25}), "1 cot gia tri");

    // niceMax.
    ok(chart::niceMax(0) == 1.0, "max 0 -> 1");
    ok(chart::niceMax(8) == 10.0, "8 -> 10");
    ok(chart::niceMax(25) == 50.0, "25 -> 50");
    ok(chart::niceMax(120) == 200.0, "120 -> 200");
    ok(chart::niceMax(1000) == 1000.0, "1000 -> 1000");

    // Góc quạt pie.
    QVector<double> ang = chart::pieAngles({1, 1, 2}); // tổng 4 -> 90,90,180
    ok(ang.size() == 3 && std::abs(ang[0] - 90) < 1e-9 && std::abs(ang[2] - 180) < 1e-9, "pie goc dung");
    double tot = 0; for (double a : ang) tot += a;
    ok(std::abs(tot - 360) < 1e-9, "pie tong 360");
    ok(chart::pieAngles({0, 0}).isEmpty(), "tong 0 -> rong");
    QVector<double> angNeg = chart::pieAngles({-5, 5}); // âm coi như 0 -> 0,360
    ok(angNeg.size() == 2 && std::abs(angNeg[0]) < 1e-9 && std::abs(angNeg[1] - 360) < 1e-9, "am coi nhu 0");

    std::printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
