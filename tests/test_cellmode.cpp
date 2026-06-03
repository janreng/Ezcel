// Test chi bao che do o (cellmode::label). Chi in ASCII.
#include "ui/CellMode.h"
#include <QCoreApplication>
#include <cstdio>

static int g_pass = 0, g_fail = 0;
static void eq(const QString &got, const char *want, const char *name) {
    if (got == QString::fromUtf8(want)) ++g_pass;
    else { ++g_fail; std::printf("FAIL %s: got '%s'\n", name, got.toUtf8().constData()); }
}

int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);
    using cellmode::Mode;
    eq(cellmode::label(Mode::Ready), "Sẵn sàng", "ready");
    eq(cellmode::label(Mode::Enter), "Nhập", "enter");
    eq(cellmode::label(Mode::Edit), "Sửa", "edit");
    eq(cellmode::label(Mode::Point), "Chọn", "point");
    std::printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
