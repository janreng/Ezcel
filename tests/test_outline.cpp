#include "view/Outline.h"
#include <cstdio>

static int g_pass = 0, g_fail = 0;
static void ok(bool c, const char *n) { if (c) ++g_pass; else { ++g_fail; std::printf("FAIL %s\n", n); } }

int main()
{
    using namespace outline;

    // Nhóm hàng 5..10, thu gọn -> ẩn 5..10.
    {
        Outline o;
        o.add(5, 10);
        ok(o.hiddenRows().isEmpty(), "moi them: chua an");
        bool col = o.toggle(7); // bam trong nhom
        ok(col, "toggle -> thu gon");
        auto h = o.hiddenRows();
        ok(h.size() == 6 && h.contains(5) && h.contains(10) && !h.contains(4), "an dung 5..10");
        o.toggle(7); // mo lai
        ok(o.hiddenRows().isEmpty(), "toggle lan 2 -> mo, het an");
    }

    // Chuan hoa first>last + bo qua trung khit.
    {
        Outline o;
        o.add(10, 5);
        o.add(5, 10); // trung khit -> bo qua
        ok(o.groups().size() == 1, "trung khit -> 1 nhom");
        ok(o.groups()[0].first == 5 && o.groups()[0].last == 10, "chuan hoa first<last");
    }

    // Lồng nhóm: 2..20 chua 5..10 -> level.
    {
        Outline o;
        o.add(2, 20);
        o.add(5, 10);
        // group 0 = 2..20 (ngoai, level 1); group 1 = 5..10 (trong, level 2)
        ok(o.levelOf(0) == 1, "nhom ngoai level 1");
        ok(o.levelOf(1) == 2, "nhom trong level 2");
        ok(o.maxLevel() == 2, "maxLevel = 2");

        // innermost: bam row 7 -> nhom trong (5..10); thu gon chi an 5..10.
        o.toggle(7);
        auto h = o.hiddenRows();
        ok(h.contains(5) && h.contains(10) && !h.contains(2) && !h.contains(20), "toggle innermost an 5..10");
    }

    // remove innermost.
    {
        Outline o;
        o.add(2, 20);
        o.add(5, 10);
        ok(o.remove(7), "remove tai row 7");
        ok(o.groups().size() == 1 && o.groups()[0].first == 2, "con lai nhom ngoai");
        ok(!o.remove(100), "remove ngoai vung -> false");
    }

    // toggle/remove khi khong co nhom.
    {
        Outline o;
        ok(!o.toggle(3), "toggle rong -> false");
        ok(o.maxLevel() == 0, "rong -> maxLevel 0");
    }

    std::printf("%d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
