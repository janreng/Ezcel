// Test bảng phím tắt (shortcuts::all). Chi in ASCII.
#include "ui/Shortcuts.h"
#include <QCoreApplication>
#include <cstdio>

static int g_pass = 0, g_fail = 0;
static void ok(bool cond, const char *name) {
    if (cond) ++g_pass;
    else { ++g_fail; std::printf("FAIL %s\n", name); }
}

static bool hasKey(const QList<shortcuts::Entry> &v, const char *keys) {
    for (const auto &e : v) if (e.keys == QString::fromUtf8(keys)) return true;
    return false;
}

int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);
    const auto v = shortcuts::all();

    ok(v.size() >= 35, "co it nhat 35 phim tat");

    // Moi dong phai du 3 truong.
    bool allFilled = true;
    for (const auto &e : v)
        if (e.category.isEmpty() || e.keys.isEmpty() || e.desc.isEmpty()) allFilled = false;
    ok(allFilled, "moi dong day du category/keys/desc");

    // Mot so phim tieu bieu phai co.
    ok(hasKey(v, "Ctrl + S"), "co Ctrl+S");
    ok(hasKey(v, "F2"), "co F2");
    ok(hasKey(v, "Alt + ="), "co Alt+= AutoSum");
    ok(hasKey(v, "Ctrl + Z / Y"), "co Undo/Redo");
    ok(hasKey(v, "F1"), "co F1 tro giup");

    // 6 nhom, dung thu tu xuat hien.
    const auto cats = shortcuts::categories();
    ok(cats.size() == 6, "co dung 6 nhom");
    ok(cats.value(0) == QString::fromUtf8("Di chuyển"), "nhom dau la Di chuyen");

    std::printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
