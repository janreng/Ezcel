// Test thuần cho "Bạn muốn làm gì?" (Tell Me, Spec 55).
#include "ui/TellMe.h"
#include <cstdio>

static int g_pass = 0, g_fail = 0;
static void ok(bool c, const char *n) { if (c) ++g_pass; else { ++g_fail; std::printf("FAIL %s\n", n); } }

int main() {
    ok(tellme::matchSubsequence(QStringLiteral("In đậm"), QStringLiteral("đậm")), "khop chuoi con");
    ok(tellme::matchSubsequence(QStringLiteral("In đậm"), QStringLiteral("Iđ")), "khop khong lien tuc");
    ok(tellme::matchSubsequence(QStringLiteral("Sắp xếp tăng"), QStringLiteral("sxt")), "khop tat ky tu");
    ok(!tellme::matchSubsequence(QStringLiteral("In đậm"), QStringLiteral("xyz")), "khong khop");
    ok(tellme::matchSubsequence(QStringLiteral("Bất kỳ"), QString()), "query rong -> khop");

    const QStringList cmds = {
        QStringLiteral("In đậm"), QStringLiteral("In nghiêng"), QStringLiteral("Lưu"),
        QStringLiteral("Lọc theo giá trị"), QStringLiteral("Sắp xếp tăng"),
    };
    const QStringList r = tellme::rank(QStringLiteral("in"), cmds);
    ok(r.size() == 2, "loc 'in' ra 2 lenh");
    ok(r.contains(QStringLiteral("In đậm")) && r.contains(QStringLiteral("In nghiêng")), "dung lenh");
    ok(tellme::rank(QString(), cmds).size() == cmds.size(), "query rong -> tat ca");
    ok(tellme::rank(QStringLiteral("l"), cmds, 1).size() == 1, "ton trong gioi han");

    std::printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
