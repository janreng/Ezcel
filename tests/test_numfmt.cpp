// Test dinh dang so (numfmt::apply). Chi in ASCII.
#include "model/NumberFormat.h"
#include <QCoreApplication>
#include <cstdio>

static int g_pass = 0, g_fail = 0;
static void eq(const QString &got, const char *want, const char *name) {
    if (got == QString::fromUtf8(want)) ++g_pass;
    else { ++g_fail; std::printf("FAIL %s: got '%s' want '%s'\n", name, got.toUtf8().constData(), want); }
}

int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);

    eq(numfmt::apply(1234.5, "#,##0.00"), "1,234.50", "thousands");
    eq(numfmt::apply(0.25, "0.00%"), "25.00%", "percent");
    eq(numfmt::apply(1234.5, "$#,##0.00"), "$1,234.50", "currency");
    eq(numfmt::apply(12300.0, "0.00E+00"), "1.23E+04", "scientific");
    eq(numfmt::apply(0.5, "# ?/?"), "1/2", "fraction half");
    eq(numfmt::apply(1.5, "# ?/?"), "1 1/2", "fraction mixed");
    eq(numfmt::apply(0.75, "# ?/?"), "3/4", "fraction 3/4");
    eq(numfmt::apply(QVariant(QString("abc")), "#,##0.00"), "", "non-number -> rong");

    // Tăng/giảm chữ số thập phân.
    eq(numfmt::adjustDecimals("", 1), "0.0", "tang tu rong");
    eq(numfmt::adjustDecimals("0", 1), "0.0", "tang tu 0");
    eq(numfmt::adjustDecimals("#,##0.00", 1), "#,##0.000", "tang giu phan cach");
    eq(numfmt::adjustDecimals("#,##0.00", -1), "#,##0.0", "giam");
    eq(numfmt::adjustDecimals("#,##0.0", -1), "#,##0", "giam het -> bo cham");
    eq(numfmt::adjustDecimals("$#,##0.00", 1), "$#,##0.000", "tien te tang");
    eq(numfmt::adjustDecimals("0.00%", 1), "0.000%", "phan tram tang");
    eq(numfmt::adjustDecimals("0.0%", -1), "0%", "phan tram giam het");

    std::printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
