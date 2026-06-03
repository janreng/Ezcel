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

    // --- ngay/gio ---
    checkNum("=YEAR(DATE(2020,6,15))", 2020);
    checkNum("=MONTH(DATE(2020,6,15))", 6);
    checkNum("=DAY(DATE(2020,6,15))", 15);
    checkNum("=DATEDIF(DATE(2020,1,1),DATE(2020,12,31),\"M\")", 11);
    checkNum("=DAYS(DATE(2020,1,11),DATE(2020,1,1))", 10);
    checkNum("=WEEKDAY(DATE(2020,6,14),2)", 7); // 2020-06-14 la Chu Nhat -> type2 CN=7

    // --- thong ke ---
    checkNum("=MEDIAN(A1:A3)", 20);
    checkNum("=STDEV(A1:A3)", 10);
    checkNum("=VAR(A1:A3)", 100);
    checkNum("=LARGE(A1:A3,1)", 30);
    checkNum("=SMALL(A1:A3,2)", 20);
    checkNum("=RANK(20,A1:A3)", 2);

    // --- da dieu kien + math/text mo rong ---
    checkNum("=COUNTIFS(A1:A3,\">15\")", 2);
    checkNum("=SUMIFS(A1:A3,A1:A3,\">15\")", 50);
    checkNum("=MAXIFS(A1:A3,A1:A3,\">15\")", 30);
    checkNum("=MINIFS(A1:A3,A1:A3,\">15\")", 20);
    checkNum("=SUMPRODUCT(A1:A3,D1:D3)", 140);
    checkNum("=GCD(12,18)", 6);
    checkNum("=LCM(4,6)", 12);
    checkNum("=FACT(5)", 120);
    checkNum("=COMBIN(5,2)", 10);
    checkNum("=MROUND(10,3)", 9);
    checkNum("=QUOTIENT(17,5)", 3);
    checkNum("=EVEN(3)", 4);
    checkNum("=ODD(2)", 3);
    checkNum("=DEGREES(PI())", 180);
    checkNum("=CODE(\"A\")", 65);
    checkNum("=ISEVEN(4)", 1);
    checkStr("=CHAR(65)", "A");
    checkStr("=REPLACE(\"abcdef\",2,3,\"XY\")", "aXYef");
    checkStr("=ISODD(4)", "false");

    std::printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
