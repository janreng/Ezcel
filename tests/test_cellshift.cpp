// Test chen/xoa o co dich chuyen (cellshift). Chi in ASCII.
#include "model/CellShift.h"
#include <QString>
#include <cstdio>

static int g_pass = 0, g_fail = 0;
static void ok(bool c, const char *n) { if (c) ++g_pass; else { ++g_fail; std::printf("FAIL %s\n", n); } }

using namespace cellshift;

// Mo phong ap ke hoach len luoi 3x3 chuoi de kiem tra ket qua.
static QVector<QVector<QString>> apply(QVector<QVector<QString>> g, const QVector<Move> &mv) {
    const auto old = g;
    for (const Move &m : mv) {
        if (m.srcR < 0) g[m.dstR][m.dstC] = "";
        else            g[m.dstR][m.dstC] = old[m.srcR][m.srcC];
    }
    return g;
}

int main() {
    QVector<QVector<QString>> base = {{"a","b","c"},{"d","e","f"},{"g","h","i"}};

    // Chen sang phai o A1: a->blank, b->B1, c... old A1,B1 -> B1,C1; C1 cu (c) mat.
    {
        auto g = apply(base, plan(3,3, 0,0,0,0, Dir::Right));
        ok(g[0][0] == "" && g[0][1] == "a" && g[0][2] == "b", "insert right A1");
        ok(g[1][0] == "d" && g[2][0] == "g", "insert right khong dung hang khac");
    }
    // Xoa sang trai A1: b->A1, c->B1, C1 blank.
    {
        auto g = apply(base, plan(3,3, 0,0,0,0, Dir::Left));
        ok(g[0][0] == "b" && g[0][1] == "c" && g[0][2] == "", "delete left A1");
    }
    // Chen xuong duoi A1: A1 blank, a->A2, d->A3; A3 cu (g) mat.
    {
        auto g = apply(base, plan(3,3, 0,0,0,0, Dir::Down));
        ok(g[0][0] == "" && g[1][0] == "a" && g[2][0] == "d", "insert down A1");
        ok(g[0][1] == "b" && g[0][2] == "c", "insert down khong dung cot khac");
    }
    // Xoa len tren A1: d->A1, g->A2, A3 blank.
    {
        auto g = apply(base, plan(3,3, 0,0,0,0, Dir::Up));
        ok(g[0][0] == "d" && g[1][0] == "g" && g[2][0] == "", "delete up A1");
    }
    // Vung 2 cot A1:B1 chen sang phai (W=2): hang 0 -> a,b day het ra mep, blank A1,B1.
    {
        auto g = apply(base, plan(3,3, 0,0,0,1, Dir::Right));
        ok(g[0][0] == "" && g[0][1] == "" && g[0][2] == "a", "insert right A1:B1 W=2");
    }
    // Vung ngoai luoi -> ke hoach rong.
    ok(plan(3,3, 0,0,0,3, Dir::Right).isEmpty(), "box ngoai luoi -> rong");
    ok(plan(3,3, 2,0,1,0, Dir::Down).isEmpty(), "box dao nguoc -> rong");

    std::printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
