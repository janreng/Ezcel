// Test gop du lieu nhieu vung theo nhan (consolidate). Chi in ASCII.
#include "model/Consolidate.h"
#include <QString>
#include <cstdio>

static int g_pass = 0, g_fail = 0;
static void ok(bool c, const char *n) { if (c) ++g_pass; else { ++g_fail; std::printf("FAIL %s\n", n); } }

static QVector<QString> R(std::initializer_list<const char *> xs) {
    QVector<QString> v; for (auto x : xs) v.push_back(QString::fromUtf8(x)); return v;
}

int main() {
    using datatools::Agg;
    // Bang 1: hang=Bac/Nam, cot=Q1/Q2
    QVector<QVector<QString>> t1 = { R({"", "Q1", "Q2"}), R({"Bac", "10", "20"}), R({"Nam", "5", "8"}) };
    // Bang 2: hang=Bac/Trung, cot=Q1/Q3 (cot khac nhau + hang moi)
    QVector<QVector<QString>> t2 = { R({"", "Q1", "Q3"}), R({"Bac", "100", "7"}), R({"Trung", "3", "4"}) };

    auto res = consolidate::byLabels({t1, t2}, Agg::Sum, QString("Vung"));
    // Nhan hang union: Bac,Nam,Trung (sap xep) -> 3 hang + header = 4
    // Nhan cot union: Q1,Q2,Q3 -> 3 cot + goc = 4
    ok(res.size() == 4, "4 hang (header + 3 nhan)");
    ok(res[0].size() == 4, "4 cot (goc + Q1,Q2,Q3)");
    ok(res[0][0] == "Vung" && res[0][1] == "Q1" && res[0][2] == "Q2" && res[0][3] == "Q3", "header dung");
    // Bac: Q1=10+100=110, Q2=20, Q3=7
    ok(res[1][0] == "Bac", "hang 1 = Bac");
    ok(res[1][1] == "110", "Bac/Q1 = 110 (gop 2 bang)");
    ok(res[1][2] == "20", "Bac/Q2 = 20");
    ok(res[1][3] == "7", "Bac/Q3 = 7");
    // Nam: Q1=5, Q2=8, Q3 trong
    ok(res[2][0] == "Nam" && res[2][1] == "5" && res[2][2] == "8" && res[2][3] == "", "Nam dung + Q3 trong");
    // Trung: Q1=3, Q3=4
    ok(res[3][0] == "Trung" && res[3][1] == "3" && res[3][3] == "4", "Trung dung");

    // Average: Bac/Q1 = (10+100)/2 = 55
    auto avg = consolidate::byLabels({t1, t2}, Agg::Average, QString("V"));
    ok(avg[1][1] == "55", "Average Bac/Q1 = 55");

    // Bang rong/khong hop le -> bo qua.
    auto only1 = consolidate::byLabels({t1, {}}, Agg::Sum, QString("V"));
    ok(only1.size() == 3, "bang rong bi bo qua (chi t1 -> 2 nhan hang)");

    std::printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
