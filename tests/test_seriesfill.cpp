// Test tu dien chuoi lich/quy (seriesfill::next). Chi in ASCII.
#include "model/SeriesFill.h"
#include <QCoreApplication>
#include <cstdio>

static int g_pass = 0, g_fail = 0;
static void eq(const std::optional<QString> &got, const char *want, const char *name) {
    QString w = QString::fromUtf8(want);
    bool ok = got.has_value() && *got == w;
    if (ok) ++g_pass; else { ++g_fail; std::printf("FAIL %s: got '%s'\n", name, got ? got->toUtf8().constData() : "(none)"); }
}
static void none(const std::optional<QString> &got, const char *name) {
    if (!got.has_value()) ++g_pass; else { ++g_fail; std::printf("FAIL %s: expected none\n", name); }
}

int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);

    eq(seriesfill::next("Mon", 1), "Tue", "Mon+1");
    eq(seriesfill::next("Sun", 1), "Mon", "Sun xoay vong -> Mon");
    eq(seriesfill::next("Monday", 2), "Wednesday", "Monday+2");
    eq(seriesfill::next("Jan", 1), "Feb", "Jan+1");
    eq(seriesfill::next("Dec", 1), "Jan", "Dec xoay vong -> Jan");
    eq(seriesfill::next("January", 1), "February", "January+1");
    eq(seriesfill::next("Q1", 1), "Q2", "Q1+1");
    eq(seriesfill::next("Q4", 1), "Q1", "Q4 xoay vong -> Q1");
    eq(seriesfill::next("Q4", 2), "Q2", "Q4+2");
    // Giu kieu hoa/thuong
    eq(seriesfill::next("MON", 1), "TUE", "hoa het");
    eq(seriesfill::next("mon", 1), "tue", "thuong het");
    // Khong khop
    none(seriesfill::next("xyz", 1), "khong khop");
    none(seriesfill::next("123", 1), "so khong khop");

    std::printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
