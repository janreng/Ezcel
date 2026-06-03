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

    std::printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
