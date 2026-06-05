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

    // ----- Các hàm tổng hợp (bản 2) -----
    // Dùng lại g: Bắc{100,30}, Nam{50,20}, Trung{10}
    pivot::Result ac = pivot::aggregate(g, 0, 0, 5, 1, 0, 1, pivot::Agg::Count);
    ok(eq(ac.values[0], 2), "Dem Bac = 2");
    ok(eq(ac.values[2], 1), "Dem Trung = 1");
    ok(eq(ac.grandTotal, 5), "Dem tong = 5 ban ghi");

    pivot::Result av = pivot::aggregate(g, 0, 0, 5, 1, 0, 1, pivot::Agg::Average);
    ok(eq(av.values[0], 65), "TB Bac = 65");
    ok(eq(av.values[1], 35), "TB Nam = 35");
    ok(eq(av.grandTotal, 42), "TB tong = 210/5 = 42");

    pivot::Result amax = pivot::aggregate(g, 0, 0, 5, 1, 0, 1, pivot::Agg::Max);
    ok(eq(amax.values[0], 100), "Max Bac = 100");
    ok(eq(amax.grandTotal, 100), "Max tong = 100");

    pivot::Result amin = pivot::aggregate(g, 0, 0, 5, 1, 0, 1, pivot::Agg::Min);
    ok(eq(amin.values[0], 30), "Min Bac = 30");
    ok(eq(amin.values[1], 20), "Min Nam = 20");
    ok(eq(amin.grandTotal, 10), "Min tong = 10");

    // Đếm tính cả ô không phải số; TB/Max/Min bỏ qua ô không phải số.
    // g2: x{abc, 5} -> Count=2, Average=5/1=5, Max=5, Min=5
    pivot::Result c2 = pivot::aggregate(g2, 0, 0, 2, 1, 0, 1, pivot::Agg::Count);
    ok(eq(c2.values[0], 2), "Dem ke ca abc = 2");
    pivot::Result v2 = pivot::aggregate(g2, 0, 0, 2, 1, 0, 1, pivot::Agg::Average);
    ok(eq(v2.values[0], 5), "TB bo qua abc = 5");

    ok(pivot::aggName(pivot::Agg::Average) == QStringLiteral("Trung bình"), "ten ham TB");

    // ----- Bảng chéo 2 chiều (bản 3) -----
    // Vùng/Quý/Doanh thu
    QVector<QVector<QString>> gx = {
        row({"Vùng", "Quý", "DT"}),
        row({"Bắc", "Q1", "100"}),
        row({"Bắc", "Q2", "40"}),
        row({"Nam", "Q1", "50"}),
        row({"Bắc", "Q1", "10"}),   // cộng dồn vào (Bắc,Q1)
    };
    pivot::CrossResult cr = pivot::crosstab(gx, 0, 0, 4, 2, 0, 1, 2, pivot::Agg::Sum);
    ok(cr.valid, "crosstab hop le");
    ok(cr.rowLabels.size() == 2, "2 hang (Bac,Nam)");
    ok(cr.colLabels.size() == 2, "2 cot (Q1,Q2)");
    ok(cr.rowField == QStringLiteral("Vùng") && cr.colField == QStringLiteral("Quý"), "ten truong hang/cot");
    // rowLabels sắp xếp: Bắc, Nam ; colLabels: Q1, Q2
    int iBac = cr.rowLabels.indexOf(QStringLiteral("Bắc"));
    int iNam = cr.rowLabels.indexOf(QStringLiteral("Nam"));
    int jQ1 = cr.colLabels.indexOf(QStringLiteral("Q1"));
    int jQ2 = cr.colLabels.indexOf(QStringLiteral("Q2"));
    ok(eq(cr.values[iBac][jQ1], 110), "(Bac,Q1) = 100+10 = 110");
    ok(eq(cr.values[iBac][jQ2], 40), "(Bac,Q2) = 40");
    ok(eq(cr.values[iNam][jQ1], 50), "(Nam,Q1) = 50");
    ok(eq(cr.values[iNam][jQ2], 0), "(Nam,Q2) khong co = 0");
    ok(eq(cr.rowTotals[iBac], 150), "tong hang Bac = 150");
    ok(eq(cr.colTotals[jQ1], 160), "tong cot Q1 = 160");
    ok(eq(cr.grandTotal, 200), "grand = 200");

    // Crosstab với Max
    pivot::CrossResult crm = pivot::crosstab(gx, 0, 0, 4, 2, 0, 1, 2, pivot::Agg::Max);
    ok(eq(crm.values[iBac][jQ1], 100), "Max (Bac,Q1) = 100");
    ok(eq(crm.rowTotals[iBac], 100), "Max hang Bac = 100");

    // Tham số sai
    pivot::CrossResult crb = pivot::crosstab(gx, 0, 0, 4, 2, 9, 1, 2, pivot::Agg::Sum);
    ok(!crb.valid, "crosstab rowCol sai -> invalid");

    std::printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
