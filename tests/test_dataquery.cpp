// Test thuần cho Lấy & Biến đổi dữ liệu (Get & Transform, Spec 20).
#include "model/DataQuery.h"
#include <cstdio>

static int g_pass = 0, g_fail = 0;
static void ok(bool c, const char *n) { if (c) ++g_pass; else { ++g_fail; std::printf("FAIL %s\n", n); } }

static QVector<QString> row(std::initializer_list<const char *> xs) {
    QVector<QString> r; for (auto x : xs) r << QString::fromUtf8(x); return r;
}

int main() {
    dquery::Grid g = {
        row({"A", "B", "C"}),
        row({"1", "2", "3"}),
        row({"", "", ""}),
        row({"4", "5", "6"}),
    };

    // selectColumns: giữ cột 0 và 2
    dquery::Grid s = dquery::selectColumns(g, {0, 2});
    ok(s.size() == 4 && s[0].size() == 2, "select 2 cot");
    ok(s[0][0] == QStringLiteral("A") && s[0][1] == QStringLiteral("C"), "header A,C");
    ok(s[1][1] == QStringLiteral("3"), "gia tri cot C giu dung");

    // selectColumns đổi thứ tự + cột ngoài phạm vi -> rỗng
    dquery::Grid s2 = dquery::selectColumns(g, {2, 0, 9});
    ok(s2[0][0] == QStringLiteral("C") && s2[0][1] == QStringLiteral("A"), "doi thu tu cot");
    ok(s2[0][2].isEmpty(), "cot ngoai pham vi -> rong");

    // keep rỗng -> lưới rỗng
    ok(dquery::selectColumns(g, {}).isEmpty(), "keep rong -> rong");

    // isEmptyRow
    ok(dquery::isEmptyRow(row({"", "  ", ""})), "dong toan trang -> rong");
    ok(!dquery::isEmptyRow(row({"", "x"})), "dong co chu -> khong rong");

    // removeEmptyRows
    dquery::Grid r = dquery::removeEmptyRows(g);
    ok(r.size() == 3, "bo 1 dong trong -> con 3");

    // apply: giữ cột 0,1 + bỏ dòng trống
    dquery::Grid a = dquery::apply(g, {0, 1}, true);
    ok(a.size() == 3 && a[0].size() == 2, "apply: 3 dong x 2 cot");
    ok(a[2][0] == QStringLiteral("4"), "apply giu dung du lieu");

    // apply không giữ cột (keep rỗng) nhưng bỏ dòng trống -> giữ nguyên cột
    dquery::Grid a2 = dquery::apply(g, {}, true);
    ok(a2.size() == 3 && a2[0].size() == 3, "apply keep rong = giu cot, bo dong trong");

    std::printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
