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
        // Dong tien F1:F6 (cot 5) cho test tai chinh NPV/IRR
        {"0,5", "-1000"}, {"1,5", "500"}, {"2,5", "500"}, {"3,5", "500"},
        {"4,5", "-100"}, {"5,5", "110"},
        // G1:G3 (cot 6) = 2*A + 1 cho test hoi quy: x=A1:A3{10,20,30}, y=G1:G3{21,41,61}
        {"0,6", "21"}, {"1,6", "41"}, {"2,6", "61"},
        // H1:H3 (cot 7) = {1,3,2} cho tuong quan khong hoan hao (CORREL voi A = 0.5)
        {"0,7", "1"}, {"1,7", "3"}, {"2,7", "2"},
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
    // Luong giac nghich dao + hyperbolic nguoc (Spec 12)
    checkNum("=SEC(0)", 1);
    checkNum("=COT(PI()/4)", 1);
    checkNum("=CSC(PI()/6)", 2);
    checkNum("=SECH(0)", 1);
    checkNum("=ACOSH(1)", 0);
    checkNum("=ATANH(0)", 0);
    checkNum("=ASINH(0)", 0);
    checkNum("=ACOSH(COSH(1))", 1);
    checkNum("=ATANH(TANH(0.5))", 0.5);
    checkNum("=ASINH(SINH(2))", 2);
    // Bitwise (Spec 12)
    checkNum("=BITAND(12,10)", 8);   // 1100 & 1010 = 1000
    checkNum("=BITOR(12,10)", 14);   // 1100 | 1010 = 1110
    checkNum("=BITXOR(12,10)", 6);   // 1100 ^ 1010 = 0110
    checkNum("=BITLSHIFT(1,4)", 16);
    checkNum("=BITRSHIFT(16,4)", 1);
    checkNum("=BITLSHIFT(16,-4)", 1); // shift am -> dich phai
    checkNum("=BITAND(255,15)", 15);
    checkNum("=LEN(B1)", 5);
    checkNum("=COUNTIF(A1:A3,\">15\")", 2);
    checkNum("=SUMIF(A1:A3,\">15\")", 50);
    checkNum("=IF(A1>50,1,2)", 2);

    checkStr("=IF(A1>5,\"big\",\"small\")", "big");
    checkStr("=CONCAT(B1,\" world\")", "hello world");
    checkStr("=UPPER(B1)", "HELLO");
    checkStr("=LEFT(B1,3)", "hel");
    checkStr("=MID(B1,2,3)", "ell");
    // TEXTBEFORE / TEXTAFTER (Spec 12)
    checkStr("=TEXTBEFORE(\"a-b-c\",\"-\")", "a");
    checkStr("=TEXTAFTER(\"a-b-c\",\"-\")", "b-c");
    checkStr("=TEXTBEFORE(\"a-b-c\",\"-\",2)", "a-b");
    checkStr("=TEXTAFTER(\"a-b-c\",\"-\",2)", "c");
    checkStr("=TEXTBEFORE(\"a-b-c\",\"-\",-1)", "a-b"); // tu cuoi
    checkStr("=TEXTAFTER(\"a-b-c\",\"-\",-1)", "c");
    checkStr("=TEXTBEFORE(\"hello world\",\" \")", "hello");
    checkStr("=TEXTAFTER(\"name@mail.com\",\"@\")", "mail.com");
    checkStr("=TEXTBEFORE(\"AxBxC\",\"x\",1,1)", "A"); // khong phan biet hoa thuong
    checkStr("=TEXTBEFORE(\"abc\",\"-\")", "#N/A"); // khong tim thay
    // ROMAN / ARABIC (Spec 12)
    checkStr("=ROMAN(4)", "IV");
    checkStr("=ROMAN(1994)", "MCMXCIV");
    checkStr("=ROMAN(2024)", "MMXXIV");
    checkStr("=ROMAN(0)", "");
    checkNum("=ARABIC(\"IV\")", 4);
    checkNum("=ARABIC(\"MCMXCIV\")", 1994);
    checkNum("=ARABIC(\"MMXXIV\")", 2024);
    checkNum("=ARABIC(ROMAN(3888))", 3888); // khu hoi
    // DELTA / GESTEP (Spec 12, ky thuat)
    checkNum("=DELTA(5,5)", 1);
    checkNum("=DELTA(5,4)", 0);
    checkNum("=DELTA(0)", 1);    // b mac dinh 0
    checkNum("=GESTEP(5,4)", 1);
    checkNum("=GESTEP(4,5)", 0);
    checkNum("=GESTEP(5,5)", 1); // bang nhau -> 1
    checkNum("=GESTEP(-1)", 0);  // step mac dinh 0
    // YEARFRAC (Spec 12): 1/1/2024 -> 1/7/2024 (182 ngay thuc; nam 2024 nhuan)
    checkNum("=YEARFRAC(DATE(2024,1,1),DATE(2024,7,1),0)", 0.5);            // US 30/360
    checkNum("=YEARFRAC(DATE(2024,1,1),DATE(2024,7,1),4)", 0.5);            // EU 30/360
    checkNum("=YEARFRAC(DATE(2024,1,1),DATE(2024,7,1),3)", 182.0/365.0);    // thuc/365
    checkNum("=YEARFRAC(DATE(2024,1,1),DATE(2024,7,1),2)", 182.0/360.0);    // thuc/360
    checkNum("=YEARFRAC(DATE(2024,1,1),DATE(2024,7,1),1)", 182.0/366.0);    // thuc/thuc nam nhuan
    checkNum("=YEARFRAC(DATE(2024,1,31),DATE(2024,2,28),0)", 28.0/360.0);   // 30/360 don ngay 31
    // AVERAGEA / MAXA / MINA (Spec 12): text="hello" -> 0 (A1:B1 = {10, "hello"})
    checkNum("=AVERAGEA(A1:B1)", 5);   // (10 + 0)/2
    checkNum("=MAXA(A1:B1)", 10);
    checkNum("=MINA(A1:B1)", 0);       // text ke 0 -> min 0
    checkNum("=AVERAGEA(2,TRUE,FALSE)", 1); // (2+1+0)/3
    checkNum("=MAXA(2,TRUE)", 2);
    checkNum("=MINA(-1,FALSE)", -1);
    checkNum("=AVERAGE(A1:B1)", 10);   // AVERAGE bo qua text -> 10/1
    // PERCENTILE / QUARTILE (Spec 12): A1:A3 = {10,20,30}
    checkNum("=PERCENTILE(A1:A3,0)", 10);
    checkNum("=PERCENTILE(A1:A3,1)", 30);
    checkNum("=PERCENTILE(A1:A3,0.5)", 20);   // trung vi
    checkNum("=PERCENTILE(A1:A3,0.25)", 15);  // noi suy
    checkNum("=QUARTILE(A1:A3,0)", 10);
    checkNum("=QUARTILE(A1:A3,1)", 15);
    checkNum("=QUARTILE(A1:A3,2)", 20);
    checkNum("=QUARTILE(A1:A3,3)", 25);
    checkNum("=QUARTILE(A1:A3,4)", 30);
    // DEVSQ / AVEDEV (Spec 12): A1:A3 = {10,20,30}, trung binh 20
    checkNum("=DEVSQ(A1:A3)", 200);        // 100 + 0 + 100
    checkNum("=AVEDEV(A1:A3)", 20.0/3.0);  // (10+0+10)/3
    checkNum("=DEVSQ(2,4,6)", 8);          // mean 4 -> 4+0+4
    checkNum("=AVEDEV(2,4,6)", 4.0/3.0);
    // SKEW / KURT (Spec 12) — dang mau Excel
    checkNum("=SKEW(1,2,3,4,5)", 0);        // doi xung -> 0
    checkNum("=SKEW(1,1,2)", 1.7320508075688772); // = can(3)
    checkNum("=KURT(1,2,3,4,5)", -1.2);     // gia tri Excel
    // FISHER / FISHERINV (Spec 12) — cap nghich dao
    checkNum("=FISHER(0)", 0);
    checkNum("=FISHER(0.5)", 0.5493061443340548); // atanh(0.5)
    checkNum("=FISHERINV(0)", 0);
    checkNum("=FISHERINV(FISHER(0.5))", 0.5);      // khu hoi
    checkNum("=FISHER(FISHERINV(1.2))", 1.2);
    // PHI / GAUSS (Spec 12) — phan phoi chuan tac
    checkNum("=PHI(0)", 0.3989422804014327);   // 1/can(2pi)
    checkNum("=GAUSS(0)", 0);
    checkNum("=GAUSS(1)", 0.3413447460685429);
    checkNum("=GAUSS(-1)", -0.3413447460685429); // ham le
    // COMBINA / PERMUTATIONA (Spec 12) — co lap
    checkNum("=COMBINA(4,3)", 20);   // C(6,3)
    checkNum("=COMBINA(3,2)", 6);    // C(4,2)
    checkNum("=COMBINA(5,0)", 1);
    checkNum("=PERMUTATIONA(3,2)", 9);  // 3^2
    checkNum("=PERMUTATIONA(2,4)", 16); // 2^4
    checkNum("=PERMUTATIONA(5,0)", 1);
    // STANDARDIZE / MULTINOMIAL (Spec 12)
    checkNum("=STANDARDIZE(42,40,1.5)", 4.0/3.0);  // (42-40)/1.5
    checkNum("=STANDARDIZE(40,40,2)", 0);
    checkNum("=MULTINOMIAL(2,3,4)", 1260);  // 9!/(2!3!4!)
    checkNum("=MULTINOMIAL(1,2,3)", 60);    // 6!/(1!2!3!)
    // PERCENTRANK / SERIESSUM (Spec 12): A1:A3 = {10,20,30}
    checkNum("=PERCENTRANK(A1:A3,10)", 0);
    checkNum("=PERCENTRANK(A1:A3,30)", 1);
    checkNum("=PERCENTRANK(A1:A3,20)", 0.5);
    checkNum("=PERCENTRANK(A1:A3,15)", 0.25); // noi suy
    checkNum("=SERIESSUM(2,0,1,A1:A3)", 170); // 10*1+20*2+30*4
    checkNum("=SERIESSUM(1,0,1,A1:A3)", 60);  // 10+20+30
    // CONVERT (Spec 12) — doi don vi
    checkNum("=CONVERT(1,\"kg\",\"g\")", 1000);
    checkNum("=CONVERT(1,\"lbm\",\"ozm\")", 16);
    checkNum("=CONVERT(1,\"ft\",\"in\")", 12);
    checkNum("=CONVERT(1,\"hr\",\"sec\")", 3600);
    checkNum("=CONVERT(1,\"mi\",\"km\")", 1.609344);
    checkNum("=CONVERT(0,\"C\",\"F\")", 32);
    checkNum("=CONVERT(100,\"C\",\"F\")", 212);
    checkNum("=CONVERT(300,\"K\",\"C\")", 26.85);
    checkStr("=CONVERT(1,\"kg\",\"m\")", "#N/A"); // khac nhom
    // CONVERT mo rong dien tich + the tich (Spec 12)
    checkNum("=CONVERT(1,\"km2\",\"m2\")", 1000000);
    checkNum("=CONVERT(1,\"ha\",\"m2\")", 10000);
    checkNum("=CONVERT(1,\"ft2\",\"in2\")", 144);
    checkNum("=CONVERT(1,\"m3\",\"l\")", 1000);
    checkNum("=CONVERT(1,\"gal\",\"qt\")", 4);
    checkNum("=CONVERT(1,\"l\",\"ml\")", 1000);
    checkStr("=CONVERT(1,\"m2\",\"l\")", "#N/A"); // dien tich vs the tich
    // NUMBERVALUE (Spec 12)
    checkNum("=NUMBERVALUE(\"1,234.56\")", 1234.56);
    checkNum("=NUMBERVALUE(\"2.5%\")", 0.025);
    checkNum("=NUMBERVALUE(\"1.234,56\",\",\",\".\")", 1234.56); // kieu chau Au
    checkNum("=NUMBERVALUE(\"100\")", 100);
    checkNum("=NUMBERVALUE(\"50%%\")", 0.005);
    checkNum("=NUMBERVALUE(\"\")", 0);
    // FV / PV (Spec 12) — gia tri tuong lai / hien tai
    checkNum("=FV(0,10,-100,0,0)", 1000);   // lai 0% -> -pmt*n
    checkNum("=FV(0.1,1,0,-100,0)", 110);   // pv -100 sinh lai 10%
    checkNum("=FV(0.05,10,-100,-1000,0)", 2886.683880332324);
    checkNum("=PV(0,5,-100,0,0)", 500);     // lai 0%
    checkNum("=PV(0.1,1,0,-110,0)", 100);   // chiet khau fv -110
    checkNum("=PV(0.05,10,-100,0,0)", 772.1734929184818); // tra gop
    // PMT / NPER (Spec 12) — tra gop / so ky
    checkNum("=PMT(0,10,-1000,0,0)", 100);   // lai 0%
    checkNum("=PMT(0.01,12,-1000,0,0)", 88.8487886783417);
    checkNum("=NPER(0,-100,1000,0,0)", 10);  // lai 0%
    checkNum("=NPER(0.01,88.8487886783417,-1000,0,0)", 12); // khu hoi PMT
    checkNum("=NPER(0.1,-110,100,0,0)", 1);
    // NPV / IRR (Spec 12): F1:F4 = {-1000,500,500,500}, F5:F6 = {-100,110}
    checkNum("=NPV(0.1,100,100,100)", 248.685199098422);
    checkNum("=NPV(0.1,F2:F4)", 1243.42599549211);
    checkNum("=IRR(F1:F4)", 0.233751928528259);
    checkNum("=IRR(F5:F6)", 0.1);
    // MIRR (Spec 12): F1:F4 = {-1000,500,500,500}, lai vay 10%, tai dau tu 12%
    checkNum("=MIRR(F1:F4,0.1,0.12)", 0.190480233637062);
    // CUMIPMT / CUMPRINC (Spec 12): vay 8000, lai 10%/nam, 36 ky
    checkNum("=CUMIPMT(0.1/12,36,8000,1,12,0)", -691.71020732028);
    checkNum("=CUMPRINC(0.1/12,36,8000,1,12,0)", -2405.93976328813);
    checkNum("=CUMIPMT(0.1/12,36,8000,1,36,0)+CUMPRINC(0.1/12,36,8000,1,36,0)", -9292.94991182524); // = 36*PMT
    // SLOPE / INTERCEPT (Spec 12): y=G1:G3{21,41,61} = 2*x+1, x=A1:A3{10,20,30}
    checkNum("=SLOPE(G1:G3,A1:A3)", 2);
    checkNum("=INTERCEPT(G1:G3,A1:A3)", 1);
    // CORREL / PEARSON / RSQ (Spec 12)
    checkNum("=CORREL(A1:A3,G1:G3)", 1);   // tuyen tinh hoan hao
    checkNum("=CORREL(A1:A3,H1:H3)", 0.5); // tuong quan 0.5
    checkNum("=PEARSON(A1:A3,H1:H3)", 0.5);
    checkNum("=RSQ(A1:A3,H1:H3)", 0.25);   // 0.5^2
    // COVAR / FORECAST (Spec 12)
    checkNum("=COVAR(A1:A3,H1:H3)", 10.0/3.0); // hiep phuong sai tong the
    checkNum("=FORECAST(50,G1:G3,A1:A3)", 101); // y=2*50+1
    checkNum("=FORECAST(0,G1:G3,A1:A3)", 1);    // = intercept
    // STEYX (Spec 12) — sai so chuan hoi quy
    checkNum("=STEYX(H1:H3,A1:A3)", 1.22474487139159); // = can(1.5)
    checkNum("=STEYX(G1:G3,A1:A3)", 0);                 // tuyen tinh hoan hao -> 0
    // IPMT / PPMT (Spec 12): vay 8000, lai 10%/nam, 36 ky
    checkNum("=IPMT(0.1/12,1,36,8000)", -66.6666666666667); // ky 1: -pv*rate
    checkNum("=IPMT(0.1/12,2,36,8000)", -65.0710764092997);
    checkNum("=PPMT(0.1/12,1,36,8000)", -191.470830884034);
    checkNum("=IPMT(0.1/12,1,36,8000)+PPMT(0.1/12,1,36,8000)", -258.137497550701); // = PMT
    // RATE (Spec 12) — lai suat moi ky (khu hoi PMT/PV)
    checkNum("=RATE(1,0,-100,110)", 0.1);  // -100*(1+r)+110=0
    checkNum("=RATE(36,-258.137497550701,8000)", 0.1/12); // khu hoi PMT
    // SLN / SYD (Spec 12) — khau hao
    checkNum("=SLN(10000,1000,5)", 1800);     // deu: (cost-salvage)/life
    checkNum("=SYD(10000,1000,5,1)", 3000);   // nam dau lon nhat
    checkNum("=SYD(10000,1000,5,5)", 600);    // nam cuoi nho nhat
    checkNum("=SYD(10000,1000,5,3)", 1800);   // nam giua
    // DDB (Spec 12) — so du giam dan kep, factor mac dinh 2
    checkNum("=DDB(10000,1000,5,1)", 4000);   // 10000*0.4
    checkNum("=DDB(10000,1000,5,2)", 2400);
    checkNum("=DDB(10000,1000,5,5)", 296);    // bi chan o muc salvage
    checkNum("=DDB(10000,1000,5,3,1.5)", 1470); // factor 1.5
    // DOLLARDE / DOLLARFR (Spec 12) — gia kieu phan so
    checkNum("=DOLLARDE(1.02,16)", 1.125); // 1 + 2/16
    checkNum("=DOLLARDE(1.1,8)", 1.125);   // 1 + 1/8
    checkNum("=DOLLARFR(1.125,16)", 1.02);
    checkNum("=DOLLARFR(1.125,8)", 1.1);
    checkNum("=DOLLARDE(DOLLARFR(1.5,32),32)", 1.5); // khu hoi
    // EFFECT / NOMINAL (Spec 12) — lai hieu dung / danh nghia
    checkNum("=EFFECT(0.1,4)", 0.10381289062499954);
    checkNum("=EFFECT(0.12,12)", 0.12682503013196977);
    checkNum("=NOMINAL(EFFECT(0.1,4),4)", 0.1); // khu hoi
    checkNum("=NOMINAL(EFFECT(0.08,2),2)", 0.08);
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
    // EOMONTH/EDATE (Spec 12 A2): don ngay cuoi thang, nam nhuan
    checkNum("=DAY(EOMONTH(DATE(2024,2,15),0))", 29);   // thang 2 nam nhuan -> 29
    checkNum("=DAY(EOMONTH(DATE(2024,1,15),1))", 29);   // +1 thang -> cuoi thang 2 = 29
    checkNum("=MONTH(EOMONTH(DATE(2024,1,15),1))", 2);
    checkNum("=DAY(EDATE(DATE(2024,1,31),1))", 29);     // 31/1 +1 thang -> 29/2 (don ngay)
    checkNum("=MONTH(EDATE(DATE(2024,1,31),1))", 2);
    checkNum("=YEAR(EDATE(DATE(2023,12,15),1))", 2024); // qua nam
    checkNum("=MONTH(EDATE(DATE(2023,12,15),1))", 1);
    // WORKDAY/NETWORKDAYS (Spec 12): bo T7/CN. 2024-01-05 la Thu Sau.
    checkNum("=DAY(WORKDAY(DATE(2024,1,5),1))", 8);     // T6 +1 ngay lam viec -> T2 8/1
    checkNum("=DAY(WORKDAY(DATE(2024,1,5),5))", 12);    // +5 ngay lam viec -> T6 12/1
    checkNum("=DAY(WORKDAY(DATE(2024,1,8),-1))", 5);    // T2 lui 1 -> T6 5/1
    checkNum("=NETWORKDAYS(DATE(2024,1,1),DATE(2024,1,7))", 5);  // T2->CN: 5 ngay lam viec
    checkNum("=NETWORKDAYS(DATE(2024,1,1),DATE(2024,1,5))", 5);  // T2->T6: 5
    checkNum("=NETWORKDAYS(DATE(2024,1,7),DATE(2024,1,1))", -5); // dao chieu -> am
    checkNum("=NETWORKDAYS(DATE(2024,1,1),DATE(2024,1,7),DATE(2024,1,3))", 4); // tru 1 ngay nghi
    checkNum("=DAY(WORKDAY(DATE(2024,1,5),1,DATE(2024,1,8)))", 9); // 8/1 nghi -> nhay 9/1
    // WEEKNUM / ISOWEEKNUM (Spec 12)
    checkNum("=WEEKNUM(DATE(2024,1,1))", 1);     // 1/1 -> tuan 1
    checkNum("=WEEKNUM(DATE(2024,1,7))", 2);     // CN 7/1 -> bat dau tuan 2 (type1 CN-start)
    checkNum("=WEEKNUM(DATE(2024,1,6))", 1);     // T7 6/1 -> con tuan 1
    checkNum("=WEEKNUM(DATE(2024,1,8),2)", 2);   // type2 T2-start: 8/1 la T2 -> tuan 2
    checkNum("=WEEKNUM(DATE(2024,1,7),2)", 1);   // type2: CN 7/1 con tuan 1
    checkNum("=ISOWEEKNUM(DATE(2024,1,1))", 1);  // T2 1/1/2024 -> ISO tuan 1
    checkNum("=ISOWEEKNUM(DATE(2023,1,1))", 52); // CN 1/1/2023 -> ISO tuan 52 (cua 2022)

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

    // Ham moi: TIME / T / N
    checkNum("=TIME(6,0,0)", 0.25);
    checkNum("=TIME(12,0,0)", 0.5);
    checkNum("=N(5)", 5);
    checkNum("=N(TRUE)", 1);
    checkNum("=N(\"abc\")", 0);
    checkStr("=T(\"hello\")", "hello");
    checkStr("=T(123)", "");
    checkStr("=FIXED(1234.567,2)", "1,234.57");
    checkStr("=FIXED(1234.5,0)", "1,235");
    checkStr("=DOLLAR(1234.5,2)", "$1,234.50");
    checkStr("=BASE(255,16)", "FF");
    checkStr("=BASE(5,2,8)", "00000101");
    checkNum("=DECIMAL(\"FF\",16)", 255);
    checkNum("=DECIMAL(\"101\",2)", 5);
    checkStr("=DEC2BIN(5)", "101");
    checkStr("=DEC2HEX(255)", "FF");
    checkNum("=BIN2DEC(\"1010\")", 10);
    checkNum("=HEX2DEC(\"1F\")", 31);
    // OCT2DEC / DEC2OCT (Spec 12) — bat phan
    checkStr("=DEC2OCT(8)", "10");
    checkStr("=DEC2OCT(64)", "100");
    checkNum("=OCT2DEC(\"17\")", 15);
    checkNum("=OCT2DEC(\"100\")", 64);
    checkNum("=OCT2DEC(DEC2OCT(255))", 255); // khu hoi
    // Doi cheo he (Spec 12)
    checkStr("=BIN2HEX(\"1111\")", "F");
    checkStr("=HEX2BIN(\"F\")", "1111");
    checkStr("=BIN2OCT(\"1000\")", "10");
    checkStr("=OCT2BIN(\"10\")", "1000");
    checkStr("=HEX2OCT(\"FF\")", "377");
    checkStr("=OCT2HEX(\"377\")", "FF");
    // So phuc (Spec 12)
    checkStr("=COMPLEX(3,4)", "3+4i");
    checkStr("=COMPLEX(0,1)", "i");
    checkStr("=COMPLEX(0,-1)", "-i");
    checkStr("=COMPLEX(3,0)", "3");
    checkStr("=COMPLEX(3,-4)", "3-4i");
    checkStr("=COMPLEX(3,1)", "3+i");
    checkNum("=IMREAL(\"3+4i\")", 3);
    checkNum("=IMAGINARY(\"3+4i\")", 4);
    checkNum("=IMAGINARY(\"-i\")", -1);
    checkNum("=IMAGINARY(\"5\")", 0);
    checkNum("=IMABS(\"3+4i\")", 5);
    // Phep toan so phuc (Spec 12)
    checkStr("=IMSUM(\"3+4i\",\"1+2i\")", "4+6i");
    checkStr("=IMSUB(\"3+4i\",\"1+2i\")", "2+2i");
    checkStr("=IMCONJUGATE(\"3+4i\")", "3-4i");
    checkNum("=IMARGUMENT(\"1+i\")", 0.7853981633974483);  // pi/4
    checkNum("=IMARGUMENT(\"3+4i\")", 0.9272952180016122);
    // IMPRODUCT / IMDIV (Spec 12)
    checkStr("=IMPRODUCT(\"3+4i\",\"1+2i\")", "-5+10i"); // (3-8)+(6+4)i
    checkStr("=IMDIV(\"3+4i\",\"1+2i\")", "2.2-0.4i");   // (11-2i)/5
    checkStr("=IMDIV(\"1\",\"i\")", "-i");               // 1/i = -i
    // IMEXP / IMLN (Spec 12) — dung khu hoi de tranh sai so dinh dang
    checkNum("=IMREAL(IMEXP(\"0\"))", 1);       // e^0 = 1
    checkNum("=IMREAL(IMLN(\"1\"))", 0);        // ln(1) = 0
    checkNum("=IMAGINARY(IMLN(\"i\"))", 1.5707963267948966); // ln(i) = i*pi/2
    checkNum("=IMREAL(IMEXP(IMLN(\"3+4i\")))", 3);   // exp(ln(z)) = z
    checkNum("=IMAGINARY(IMEXP(IMLN(\"3+4i\")))", 4);

    std::printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
