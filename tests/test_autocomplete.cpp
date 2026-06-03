// Test AutoComplete (autocomplete::suggest/uniqueTexts). Chi in ASCII.
#include "model/AutoComplete.h"
#include <QCoreApplication>
#include <cstdio>

static int g_pass = 0, g_fail = 0;
static void eqs(const QString &got, const char *want, const char *name) {
    if (got == QString::fromUtf8(want)) ++g_pass; else { ++g_fail; std::printf("FAIL %s: got '%s'\n", name, got.toUtf8().constData()); }
}
static std::vector<QString> S(std::initializer_list<const char*> xs) { std::vector<QString> v; for (auto x : xs) v.push_back(QString::fromUtf8(x)); return v; }

int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);

    // Mot match duy nhat
    eqs(autocomplete::suggest(S({"Apple","Banana","Cherry"}), "Ap"), "Apple", "1 match Ap->Apple");
    eqs(autocomplete::suggest(S({"Apple","Banana"}), "ba"), "Banana", "khong phan biet hoa thuong");
    // Nhieu match -> rong
    eqs(autocomplete::suggest(S({"Apple","Apricot"}), "Ap"), "", "2 match -> rong");
    // Trung prefix -> rong
    eqs(autocomplete::suggest(S({"Apple"}), "Apple"), "", "trung prefix -> rong");
    // Khong match -> rong
    eqs(autocomplete::suggest(S({"Apple"}), "Xy"), "", "khong match");
    // Bo qua so/cong thuc/rong
    eqs(autocomplete::suggest(S({"10","=A1","","Mango"}), "Ma"), "Mango", "bo qua so/ct/rong");
    // Trung lap khong tinh la nhieu match
    eqs(autocomplete::suggest(S({"Apple","Apple","Apple"}), "Ap"), "Apple", "trung lap van 1 match");

    // uniqueTexts
    auto u = autocomplete::uniqueTexts(S({"Banana","Apple","Apple","10","=B1","","Cherry"}));
    bool ok = (u.size() == 3 && u[0] == "Apple" && u[1] == "Banana" && u[2] == "Cherry");
    if (ok) ++g_pass; else { ++g_fail; std::printf("FAIL uniqueTexts\n"); }

    std::printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
