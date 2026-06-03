// Test dich Viet/Anh (i18n). Chi in ASCII.
#include "ui/I18n.h"
#include <QCoreApplication>
#include <cstdio>

static int g_pass = 0, g_fail = 0;
static void ok(bool cond, const char *name) {
    if (cond) ++g_pass; else { ++g_fail; std::printf("FAIL %s\n", name); }
}

int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);

    i18n::setLang(i18n::Lang::Vi);
    ok(i18n::tr("menu_file") == QString::fromUtf8("&T\xE1\xBB\x87p"), "vi: menu_file = &Tep");
    ok(i18n::tr("file_save") == QString::fromUtf8("&L\xC6\xB0u"), "vi: file_save = &Luu");

    i18n::setLang(i18n::Lang::En);
    ok(i18n::tr("menu_file") == "&File", "en: menu_file = &File");
    ok(i18n::tr("file_save") == "&Save", "en: file_save = &Save");
    ok(i18n::tr("view_zoom_in") == "Zoom In", "en: zoom in");

    // khoa khong co -> tra chinh khoa
    ok(i18n::tr("khong_ton_tai") == "khong_ton_tai", "khoa thieu -> tra key");
    ok(i18n::lang() == i18n::Lang::En, "lang() = En sau setLang");

    std::printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
