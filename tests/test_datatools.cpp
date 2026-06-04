// Test cong cu du lieu (datatools). Chi in ASCII.
#include "model/DataTools.h"
#include <QCoreApplication>
#include <cstdio>

static int g_pass = 0, g_fail = 0;
static void ok(bool c, const char *n) { if (c) ++g_pass; else { ++g_fail; std::printf("FAIL %s\n", n); } }
static std::vector<QString> R(std::initializer_list<const char*> xs) { std::vector<QString> v; for (auto x : xs) v.push_back(QString::fromUtf8(x)); return v; }

int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);

    // splitDelimited
    auto p = datatools::splitDelimited("a,b,c", ",");
    ok(p.size() == 3 && p[0] == "a" && p[2] == "c", "split a,b,c");
    auto p2 = datatools::splitDelimited("a,,c", ",");
    ok(p2.size() == 3 && p2[1] == "", "giu token rong");
    auto p3 = datatools::splitDelimited("a,,c", ",", true);
    ok(p3.size() == 2, "gop dau phan cach lien nhau");
    ok(datatools::splitDelimited("xyz", ",").size() == 1, "khong co delim -> 1 phan");

    // duplicateRowIndices: cot 0 lam khoa, co header
    std::vector<std::vector<QString>> rows = {
        R({"Ten","Diem"}),     // 0 header
        R({"An","9"}),         // 1
        R({"Binh","8"}),       // 2
        R({"An","7"}),         // 3 trung An -> xoa
        R({"Binh","8"}),       // 4 trung Binh -> xoa
    };
    auto d = datatools::duplicateRowIndices(rows, {0}, true);
    ok(d.size() == 2 && d.contains(3) && d.contains(4), "dup theo cot 0 -> xoa 3,4");
    ok(!d.contains(0), "header khong tinh");

    // khoa nhieu cot: ca dong phai trung
    auto d2 = datatools::duplicateRowIndices(rows, {0, 1}, true);
    ok(d2.size() == 1 && d2.contains(4), "khoa (0,1): chi Binh/8 trung");

    // --- subtotal (Spec 27.6) ---
    // Da sort theo cot 0 (vung). Cot 1 = doanh so.
    QVector<QVector<QString>> sdata = {
        {"Bac", "10"}, {"Bac", "20"},
        {"Nam", "5"},  {"Nam", "5"}, {"Nam", "30"},
    };
    auto st = datatools::subtotal(sdata, 0, {1}, datatools::Agg::Sum,
                                  QString::fromUtf8("Tong"), QString::fromUtf8("Tong cong"));
    // 5 dong goc + 2 dong tong phu + 1 dong tong cong = 8
    ok(st.size() == 8, "subtotal: 5 goc + 2 phu + 1 cong = 8 dong");
    ok(st[2][0] == QString::fromUtf8("Bac Tong") && st[2][1] == "30", "tong phu Bac = 30");
    ok(st[6][0] == QString::fromUtf8("Nam Tong") && st[6][1] == "40", "tong phu Nam = 40");
    ok(st[7][0] == QString::fromUtf8("Tong cong") && st[7][1] == "70", "tong cong = 70");

    // Count + Average + Max + Min tren mot nhom
    QVector<QVector<QString>> one = {{"X", "2"}, {"X", "4"}, {"X", "9"}};
    auto sc = datatools::subtotal(one, 0, {1}, datatools::Agg::Count, "T", "G");
    ok(sc[3][1] == "3", "Count = 3");
    auto sa = datatools::subtotal(one, 0, {1}, datatools::Agg::Average, "T", "G");
    ok(sa[3][1] == "5", "Average = 5");
    auto smx = datatools::subtotal(one, 0, {1}, datatools::Agg::Max, "T", "G");
    ok(smx[3][1] == "9", "Max = 9");
    auto smn = datatools::subtotal(one, 0, {1}, datatools::Agg::Min, "T", "G");
    ok(smn[3][1] == "2", "Min = 2");

    ok(datatools::subtotal({}, 0, {1}, datatools::Agg::Sum, "T", "G").isEmpty(), "rong -> rong");

    // --- reverseRows ---
    {
        QVector<QVector<QString>> blk = {{"a", "1"}, {"b", "2"}, {"c", "3"}};
        auto rev = datatools::reverseRows(blk);
        ok(rev.size() == 3 && rev[0][0] == "c" && rev[2][0] == "a", "reverseRows dao hang");
        ok(rev[0][1] == "3" && rev[1][0] == "b", "reverseRows giu nguyen cot");
        ok(datatools::reverseRows({}).isEmpty(), "reverse rong -> rong");
        // reverseCols
        auto rc = datatools::reverseCols(blk);
        ok(rc[0][0] == "1" && rc[0][1] == "a", "reverseCols dao cot hang 0");
        ok(rc[2][0] == "3" && rc[2][1] == "c", "reverseCols hang 2");
        ok(datatools::reverseCols({}).isEmpty(), "reverseCols rong -> rong");
    }

    // --- fillBlanksDown ---
    {
        QVector<QVector<QString>> blk = {
            {"Bac", "10"},
            {"",    "20"},
            {"",    "30"},
            {"Nam", "5"},
            {"",    "15"},
        };
        auto f = datatools::fillBlanksDown(blk);
        ok(f[1][0] == "Bac" && f[2][0] == "Bac", "fill blank lay nhom Bac");
        ok(f[4][0] == "Nam", "doi nhom -> Nam");
        ok(f[0][0] == "Bac" && f[3][0] == "Nam", "o khong rong giu nguyen");
        ok(f[2][1] == "30", "cot khac khong bi anh huong");
        // o trong dau cot (chua co gia tri tren) -> giu trong
        QVector<QVector<QString>> blk2 = {{""}, {"x"}};
        auto f2 = datatools::fillBlanksDown(blk2);
        ok(f2[0][0].isEmpty() && f2[1][0] == "x", "o trong dau cot giu trong");
    }

    // --- joinColumns ---
    {
        QVector<QVector<QString>> blk = {{"Nguyen", "Van", "A"}, {"Tran", "", "B"}};
        auto j = datatools::joinColumns(blk, " ", false);
        ok(j.size() == 2 && j[0] == "Nguyen Van A", "join 3 cot");
        ok(j[1] == "Tran  B", "giu o trong -> 2 dau cach");
        auto js = datatools::joinColumns(blk, " ", true);
        ok(js[1] == "Tran B", "skipEmpty bo o trong");
        auto jc = datatools::joinColumns(blk, ", ", false);
        ok(jc[0] == "Nguyen, Van, A", "separator nhieu ky tu");
    }

    // --- duplicateValueIndices ---
    {
        QVector<QString> vals = {"a", "b", "A", "c", "b", ""};
        auto d = datatools::duplicateValueIndices(vals);
        // 'a'(0) & 'A'(2) trung (ci); 'b'(1) & 'b'(4) trung -> indices 0,1,2,4
        ok(d.size() == 4 && d.contains(0) && d.contains(2) && d.contains(1) && d.contains(4), "tim 4 o trung (ci)");
        ok(!d.contains(3) && !d.contains(5), "c don nhat + o rong khong tinh");
        ok(datatools::duplicateValueIndices({"x", "y", "z"}).isEmpty(), "khong trung -> rong");
    }

    // --- pivotSummary (Spec 18) ---
    {
        QVector<QVector<QString>> data = {
            {"Bac", "10"}, {"Nam", "5"}, {"Bac", "30"}, {"Nam", "20"}, {"Trung", "7"},
        };
        auto sum = datatools::pivotSummary(data, 0, 1, datatools::Agg::Sum);
        // nhom sap xep: Bac(40), Nam(25), Trung(7)
        ok(sum.size() == 3, "3 nhom");
        ok(sum[0].first == "Bac" && sum[0].second == 40, "Bac tong 40");
        ok(sum[1].first == "Nam" && sum[1].second == 25, "Nam tong 25");
        ok(sum[2].first == "Trung" && sum[2].second == 7, "Trung tong 7");
        auto cnt = datatools::pivotSummary(data, 0, 1, datatools::Agg::Count);
        ok(cnt[0].second == 2, "Count Bac = 2");
        auto avg = datatools::pivotSummary(data, 0, 1, datatools::Agg::Average);
        ok(avg[0].second == 20, "Average Bac = 20");
        ok(datatools::pivotSummary({}, 0, 1, datatools::Agg::Sum).isEmpty(), "rong -> rong");
    }

    // --- pivotCrosstab (Spec 18, bang cheo 2 chieu) ---
    {
        // cot 0 = vung, cot 1 = quy, cot 2 = doanh thu
        QVector<QVector<QString>> data = {
            {"Bac", "Q1", "10"}, {"Nam", "Q1", "5"}, {"Bac", "Q2", "30"},
            {"Nam", "Q2", "20"}, {"Bac", "Q1", "40"},
        };
        auto t = datatools::pivotCrosstab(data, 0, 1, 2, datatools::Agg::Sum,
                                          QString("Vung"), QString("Tong"));
        // header + 2 hang (Bac, Nam) + hang tong = 4 hang; 1 + 2 cot (Q1,Q2) + tong = 4 cot
        ok(t.size() == 4, "cheo: 4 hang");
        ok(t[0].size() == 4, "cheo: 4 cot");
        ok(t[0][0] == "Vung" && t[0][1] == "Q1" && t[0][2] == "Q2" && t[0][3] == "Tong", "header dung");
        // Bac: Q1 = 10+40 = 50, Q2 = 30, tong hang = 80
        ok(t[1][0] == "Bac", "hang 1 = Bac");
        ok(t[1][1] == "50", "Bac/Q1 = 50");
        ok(t[1][2] == "30", "Bac/Q2 = 30");
        ok(t[1][3] == "80", "Bac tong hang = 80");
        // Nam: Q1 = 5, Q2 = 20, tong = 25
        ok(t[2][1] == "5" && t[2][2] == "20" && t[2][3] == "25", "Nam dung");
        // hang tong cot: Q1 = 55, Q2 = 50, tong toan bo = 105
        ok(t[3][0] == "Tong" && t[3][1] == "55" && t[3][2] == "50" && t[3][3] == "105", "tong cot + toan bo dung");
    }

    std::printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
