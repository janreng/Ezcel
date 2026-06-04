#include "model/RangeParse.h"
#include <QCoreApplication>
#include <cstdio>

static int g_pass = 0, g_fail = 0;
static void ok(bool c, const char *n) { if (c) ++g_pass; else { ++g_fail; std::printf("FAIL %s\n", n); } }

static bool eqR(const MergeRange &r, int t, int l, int b, int rr) {
    return r.top == t && r.left == l && r.bottom == b && r.right == rr;
}

int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);
    using namespace rangeparse;
    const int ROWS = 100, COLS = 26;

    // Ô đơn
    auto a1 = parseOne("A1", ROWS, COLS);
    ok(a1 && eqR(*a1, 0, 0, 0, 0), "A1 -> (0,0)");
    auto c3 = parseOne("C3", ROWS, COLS);
    ok(c3 && eqR(*c3, 2, 2, 2, 2), "C3 -> (2,2)");

    // Vùng
    auto r = parseOne("A1:B3", ROWS, COLS);
    ok(r && eqR(*r, 0, 0, 2, 1), "A1:B3 -> 0,0,2,1");
    // Vùng đảo ngược -> chuẩn hóa
    auto rinv = parseOne("B3:A1", ROWS, COLS);
    ok(rinv && eqR(*rinv, 0, 0, 2, 1), "B3:A1 chuan hoa");

    // Cả cột A:A và A:C
    auto colA = parseOne("A:A", ROWS, COLS);
    ok(colA && eqR(*colA, 0, 0, ROWS - 1, 0), "A:A ca cot");
    auto colAC = parseOne("A:C", ROWS, COLS);
    ok(colAC && eqR(*colAC, 0, 0, ROWS - 1, 2), "A:C ca 3 cot");

    // Cả hàng 1:1 và 2:4
    auto row1 = parseOne("1:1", ROWS, COLS);
    ok(row1 && eqR(*row1, 0, 0, 0, COLS - 1), "1:1 ca hang");
    auto row24 = parseOne("2:4", ROWS, COLS);
    ok(row24 && eqR(*row24, 1, 0, 3, COLS - 1), "2:4 ca 3 hang");

    // Không hợp lệ
    ok(!parseOne("", ROWS, COLS), "rong -> nullopt");
    ok(!parseOne("xyz", ROWS, COLS), "khong phai ref -> nullopt");
    ok(!parseOne("A", ROWS, COLS), "thieu hang (o don) -> nullopt");

    // Kẹp ngoài lưới
    auto big = parseOne("A200", ROWS, COLS);
    ok(big && big->top == ROWS - 1, "A200 kep ve hang cuoi");

    // Đa vùng
    auto multi = parseMulti("A1:B3,D5,F1:F10", ROWS, COLS);
    ok(multi.size() == 3, "3 vung");
    ok(eqR(multi[1], 4, 3, 4, 3), "vung 2 = D5");
    ok(eqR(multi[2], 0, 5, 9, 5), "vung 3 = F1:F10");
    // Token rac bi bo qua
    auto mix = parseMulti("A1,,xyz,C3", ROWS, COLS);
    ok(mix.size() == 2, "bo token rac -> 2 vung");

    std::printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
