// Test thuần cho Kẻ viền ô (Cell borders, Spec 06).
#include "model/BorderOps.h"
#include <cstdio>

static int g_pass = 0, g_fail = 0;
static void ok(bool c, const char *n) { if (c) ++g_pass; else { ++g_fail; std::printf("FAIL %s\n", n); } }

int main() {
    using namespace borderops;

    // fromString / toString
    ok(fromString(QStringLiteral("TLBR")) == All, "TLBR = All(15)");
    ok(fromString(QStringLiteral("")) == 0, "rong = 0");
    ok(fromString(QStringLiteral("tb")) == (Top | Bottom), "tb (thuong) = Top|Bottom");
    ok((fromString(QStringLiteral("R")) & Right) != 0, "R co canh phai");
    ok(toString(All) == QStringLiteral("TLBR"), "All -> TLBR");
    ok(toString(Top | Bottom) == QStringLiteral("TB"), "Top|Bottom -> TB");
    ok(toString(0).isEmpty(), "0 -> rong");

    // round-trip
    ok(fromString(toString(Left | Right)) == (Left | Right), "round-trip LR");

    // outlineEdges trong vùng 3x3 [1..3]x[1..3]
    ok(outlineEdges(1, 1, 1, 1, 3, 3) == (Top | Left), "goc tren-trai = Top|Left");
    ok(outlineEdges(3, 3, 1, 1, 3, 3) == (Bottom | Right), "goc duoi-phai = Bottom|Right");
    ok(outlineEdges(2, 2, 1, 1, 3, 3) == 0, "o giua = khong canh");
    ok(outlineEdges(1, 2, 1, 1, 3, 3) == Top, "mep tren giua = Top");
    ok(outlineEdges(2, 1, 1, 1, 3, 3) == Left, "mep trai giua = Left");

    // ô đơn (vùng 1x1) -> đủ 4 cạnh
    ok(outlineEdges(5, 5, 5, 5, 5, 5) == All, "o don = All");

    std::printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
