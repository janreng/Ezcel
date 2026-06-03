// Test headless engine cong thuc. CHI in ASCII (console cp1252).
#include "formula/Formula.h"
#include <QHash>
#include <QString>
#include <QVariant>
#include <cstdio>
#include <cmath>

using formula::evaluate;

// Luoi mau: A1=10 A2=20 A3=30 (cot 0), B1="hello" (cot 1).
static QVariant resolver(int row, int col) {
    static QHash<QString, QString> cells = {
        {"0,0", "10"}, {"1,0", "20"}, {"2,0", "30"}, {"0,1", "hello"},
        // Bang tra cuu D1:E3 (cot 3=keys, cot 4=values)
        {"0,3", "1"}, {"1,3", "2"}, {"2,3", "3"},
        {"0,4", "one"}, {"1,4", "two"}, {"2,4", "three"},
    };
    auto it = cells.constFind(QString("%1,%2").arg(row).arg(col));
    return it == cells.constEnd() ? QVariant(QString()) : QVariant(it.value());
}

static int g_pass = 0, g_fail = 0;

static QVariant ev(const char *f) {
    try { return evaluate(QString::fromLatin1(f), resolver); }
    catch (const formula::FormulaError &e) { return e.etype(); }
}

static void checkNum(const char *f, double expected) {
    QVariant r = ev(f);
    bool ok = r.canConvert<double>() && std::abs(r.toDouble() - expected) < 1e-9;
    if (ok) { ++g_pass; }
    else { ++g_fail; std::printf("FAIL %s -> %s (want %g)\n", f, r.toString().toLatin1().constData(), expected); }
}

static void checkStr(const char *f, const char *expected) {
    QVariant r = ev(f);
    bool ok = r.toString() == QString::fromLatin1(expected);
    if (ok) { ++g_pass; }
    else { ++g_fail; std::printf("FAIL %s -> '%s' (want '%s')\n", f, r.toString().toLatin1().constData(), expected); }
}

int main() {
    checkNum("=1+2*3", 7);
    checkNum("=(1+2)*3", 9);
    checkNum("=2^10", 1024);
    checkNum("=-A1", -10);
    checkNum("=SUM(A1:A3)", 60);
    checkNum("=AVERAGE(A1:A3)", 20);
    checkNum("=MAX(A1:A3)", 30);
    checkNum("=MIN(A1:A3)", 10);
    checkNum("=COUNT(A1:A3)", 3);
    checkNum("=PRODUCT(A1:A3)", 6000);
    checkNum("=ROUND(3.14159,2)", 3.14);
    checkNum("=MOD(10,3)", 1);
    checkNum("=ABS(-7)", 7);
    checkNum("=INT(3.9)", 3);
    checkNum("=POWER(2,8)", 256);
    checkNum("=LEN(B1)", 5);
    checkNum("=COUNTIF(A1:A3,\">15\")", 2);
    checkNum("=SUMIF(A1:A3,\">15\")", 50);
    checkNum("=IF(A1>50,1,2)", 2);

    checkStr("=IF(A1>5,\"big\",\"small\")", "big");
    checkStr("=CONCAT(B1,\" world\")", "hello world");
    checkStr("=UPPER(B1)", "HELLO");
    checkStr("=LEFT(B1,3)", "hel");
    checkStr("=MID(B1,2,3)", "ell");
    checkStr("=A1/0", "#DIV/0!");
    checkStr("=IFERROR(A1/0,\"err\")", "err");
    checkStr("=AND(A1>5,A2>5)", "true");
    checkStr("=OR(A1>50,A2>50)", "false");
    checkStr("=NOSUCHFN(1)", "#NAME?");

    // --- nhom tra cuu ---
    checkStr("=VLOOKUP(2,D1:E3,2,FALSE)", "two");
    checkStr("=INDEX(D1:E3,3,2)", "three");
    checkNum("=MATCH(2,D1:D3,0)", 2);
    checkNum("=HLOOKUP(1,D1:E1,1,FALSE)", 1);
    checkStr("=CHOOSE(2,\"a\",\"b\",\"c\")", "b");
    checkNum("=ROWS(D1:E3)", 3);
    checkNum("=COLUMNS(D1:E3)", 2);
    checkStr("=XLOOKUP(3,D1:D3,E1:E3)", "three");
    checkStr("=XLOOKUP(9,D1:D3,E1:E3,\"none\")", "none");

    std::printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
