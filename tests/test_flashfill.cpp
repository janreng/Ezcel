// Test Flash Fill (flashfill::infer). Chi in ASCII.
#include "model/FlashFill.h"
#include <QCoreApplication>
#include <cstdio>

static int g_pass = 0, g_fail = 0;
static void eqv(const std::optional<std::vector<QString>> &got, const std::vector<const char*> &want, const char *name) {
    bool ok = got.has_value() && got->size() == want.size();
    if (ok) for (size_t i = 0; i < want.size(); ++i) if ((*got)[i] != QString::fromUtf8(want[i])) { ok = false; break; }
    if (ok) ++g_pass; else { ++g_fail; std::printf("FAIL %s\n", name); }
}
static void none(const std::optional<std::vector<QString>> &got, const char *name) {
    if (!got.has_value()) ++g_pass; else { ++g_fail; std::printf("FAIL %s: expected none\n", name); }
}
static std::vector<QString> S(std::initializer_list<const char*> xs) { std::vector<QString> v; for (auto x : xs) v.push_back(QString::fromUtf8(x)); return v; }

int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);

    // Lay token dau (ho) theo dau cach
    eqv(flashfill::infer(S({"Nguyen Van A","Tran Van B","Le Thi C"}), S({"Nguyen"})),
        {"Nguyen","Tran","Le"}, "token dau theo space");
    // Lay token cuoi
    eqv(flashfill::infer(S({"Nguyen Van A","Tran Van B"}), S({"A"})),
        {"A","B"}, "token cuoi");
    // Tach domain theo @
    eqv(flashfill::infer(S({"user@gmail.com","admin@yahoo.com"}), S({"gmail.com"})),
        {"gmail.com","yahoo.com"}, "domain theo @");
    // Cat chuoi con theo vi tri (prefix 3 ky tu)
    eqv(flashfill::infer(S({"ABC123","DEF456","GHI789"}), S({"ABC"})),
        {"ABC","DEF","GHI"}, "prefix 3 ky tu");
    // Hai vi du nhat quan
    eqv(flashfill::infer(S({"Nguyen Van A","Tran Van B","Le Thi C"}), S({"Nguyen","Tran"})),
        {"Nguyen","Tran","Le"}, "2 vi du token dau");
    // Khong nhan ra mau
    none(flashfill::infer(S({"abc","def"}), S({"xyz"})), "khong nhan ra");

    std::printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
