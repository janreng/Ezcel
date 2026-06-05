// Test thuần cho PivotTable / Bảng tổng hợp (Spec 41).
#include "model/Pivot.h"
#include <cstdio>
#include <cmath>

static int g_pass = 0, g_fail = 0;
static void ok(bool c, const char *n) { if (c) ++g_pass; else { ++g_fail; std::printf("FAIL %s\n", n); } }
static bool eq(double a, double b) { return std::fabs(a - b) < 1e-9; }

static QVector<QString> row(std::initializer_list<const char *> xs) {
    QVector<QString> r; for (auto x : xs) r << QString::fromUtf8(x); return r;
}

int main() {
    // Vùng: tiêu đề "Vùng/Doanh thu", gom theo Vùng + tổng Doanh thu.
    QVector<QVector<QString>> g = {
        row({"Vùng", "Doanh thu"}),
        row({"Bắc", "100"}),
        row({"Nam", "50"}),
        row({"Bắc", "30"}),
        row({"Nam", "20"}),
        row({"Trung", "10"}),
    };

    pivot::Result r = pivot::sum(g, 0, 0, 5, 1, 0, 1);
    ok(r.valid, "hop le");
    ok(r.rowField == QStringLiteral("Vùng"), "ten truong hang");
    ok(r.valueField == QStringLiteral("Doanh thu"), "ten truong gia tri");
    ok(r.rowLabels.size() == 3, "3 nhom");
    // Sắp xếp chuỗi: Bắc, Nam, Trung
    ok(r.rowLabels[0] == QStringLiteral("Bắc"), "nhom 0 = Bac");
    ok(eq(r.values[0], 130), "Bac = 130");
    ok(eq(r.values[1], 70), "Nam = 70");
    ok(eq(r.values[2], 10), "Trung = 10");
    ok(eq(r.grandTotal, 210), "tong cong 210");

    // Giá trị không phải số -> 0
    QVector<QVector<QString>> g2 = {
        row({"K", "V"}), row({"x", "abc"}), row({"x", "5"}),
    };
    pivot::Result r2 = pivot::sum(g2, 0, 0, 2, 1, 0, 1);
    ok(r2.valid && r2.rowLabels.size() == 1, "g2 1 nhom");
    ok(eq(r2.values[0], 5), "abc coi nhu 0");

    // Nhãn rỗng -> "(trống)"
    QVector<QVector<QString>> g3 = {
        row({"K", "V"}), row({"", "7"}), row({"a", "3"}),
    };
    pivot::Result r3 = pivot::sum(g3, 0, 0, 2, 1, 0, 1);
    ok(r3.rowLabels.contains(QStringLiteral("(trống)")), "nhan rong -> (trong)");

    // Nhãn toàn số -> sắp theo số (2 < 10)
    QVector<QVector<QString>> g4 = {
        row({"K", "V"}), row({"10", "1"}), row({"2", "1"}),
    };
    pivot::Result r4 = pivot::sum(g4, 0, 0, 2, 1, 0, 1);
    ok(r4.rowLabels[0] == QStringLiteral("2"), "sap theo so: 2 truoc 10");

    // Tham số sai -> không hợp lệ
    pivot::Result rb = pivot::sum(g, 0, 0, 5, 1, 5, 1);
    ok(!rb.valid, "rowCol ngoai vung -> invalid");

    std::printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
