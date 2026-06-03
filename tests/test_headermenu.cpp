#include "view/HeaderMenu.h"
#include <QtGlobal>
#include <cstdio>

static int g_pass = 0, g_fail = 0;
static void check(bool ok, const char *what)
{
    if (ok) { ++g_pass; }
    else { ++g_fail; std::printf("FAIL: %s\n", what); }
}

int main()
{
    using namespace headermenu;

    // Không có section ẩn -> không bật Unhide.
    QVector<bool> none{false, false, false, false};
    check(!canUnhide(none, 0, 3), "no hidden -> cannot unhide");
    check(hiddenSections(none, 0, 3).isEmpty(), "no hidden -> empty list");

    // Section ẩn ngay trong vùng chọn.
    QVector<bool> mid{false, true, false, false};
    check(canUnhide(mid, 0, 3), "hidden inside range -> can unhide");
    check(hiddenSections(mid, 0, 3) == QVector<int>{1}, "hidden index 1 listed");

    // Chọn hai biên ôm lấy hàng ẩn ở giữa (hàng 2 ẩn, chọn 1..3).
    QVector<bool> between{false, true, false, false, false};
    check(canUnhide(between, 1, 3), "selection straddles hidden middle");

    // Section ẩn nằm NGOÀI vùng chọn -> không bật.
    QVector<bool> outside{true, false, false, false};
    check(!canUnhide(outside, 1, 3), "hidden outside range -> cannot unhide");

    // Nhiều section ẩn.
    QVector<bool> many{false, true, false, true, true};
    check(hiddenSections(many, 0, 4) == (QVector<int>{1, 3, 4}), "multiple hidden listed in order");

    // Biên vượt mảng được kẹp an toàn.
    check(canUnhide(mid, -5, 99), "out-of-range bounds clamped");

    std::printf("%d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
