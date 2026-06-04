#include "model/NameValidate.h"
#include <QCoreApplication>
#include <cstdio>

static int g_pass = 0, g_fail = 0;
static void ok(bool c, const char *n) { if (c) ++g_pass; else { ++g_fail; std::printf("FAIL %s\n", n); } }

int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);
    using namespace namevalidate;

    // Hợp lệ
    ok(isValid("DoanhThu"), "ten chu thuong");
    ok(isValid("_tong"), "bat dau bang _");
    ok(isValid("Q1.2026"), "co dau cham va so");
    ok(isValid("Thue_GTGT"), "co gach duoi");

    // Không hợp lệ
    ok(!isValid(""), "rong");
    ok(!isValid("1abc"), "bat dau bang so");
    ok(!isValid("a b"), "co khoang trang");
    ok(!isValid("A1"), "trung dia chi o A1");
    ok(!isValid("$B$2"), "trung dia chi o $B$2");
    ok(!isValid("R1C1"), "trung kieu R1C1");
    ok(!isValid("C"), "C reserved");
    ok(!isValid("r"), "r reserved (ci)");
    ok(!isValid("ten@xyz"), "ky tu cam @");

    // AB12 dạng địa chỉ -> cấm; nhưng ABC (khong so) hợp lệ
    ok(!isValid("AB12"), "AB12 dia chi -> cam");
    ok(isValid("ABC"), "ABC khong phai dia chi -> ok");

    std::printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
