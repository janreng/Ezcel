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
        // I1:I3 (cot 8) = {2,4,8} = 2^x voi x = D1:D3 {1,2,3} cho test GROWTH (mu)
        {"0,8", "2"}, {"1,8", "4"}, {"2,8", "8"},
        // CSDL K1:L5 cho D-functions (cot 10=Loai, cot 11=Tien)
        {"0,10", "Loai"}, {"1,10", "A"}, {"2,10", "B"}, {"3,10", "A"}, {"4,10", "B"},
        {"0,11", "Tien"}, {"1,11", "10"}, {"2,11", "20"}, {"3,11", "30"}, {"4,11", "40"},
        // Tieu chi N1:N2 (Loai=A) va O1:O2 (Tien>15)
        {"0,13", "Loai"}, {"1,13", "A"},
        {"0,14", "Tien"}, {"1,14", ">15"},
        // Tieu chi P1:P2 (Tien=10) -> khop dung 1 ban ghi
        {"0,15", "Tien"}, {"1,15", "10"},
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
    // ERF / ERFC (Spec 12)
    checkNum("=ERF(0)", 0);
    checkNum("=ERF(1)", 0.8427007929497149);
    checkNum("=ERFC(0)", 1);
    checkNum("=ERFC(1)", 0.1572992070502851);
    checkNum("=ERF(0,1)", 0.8427007929497149); // erf(1)-erf(0)
    checkNum("=ERF(1)+ERFC(1)", 1);            // bu nhau
    // GAMMA / GAMMALN (Spec 12)
    checkNum("=GAMMA(5)", 24);                  // = 4!
    checkNum("=GAMMA(0.5)", 1.772453850905516); // = can(pi)
    checkNum("=GAMMALN(1)", 0);                 // ln(0!)=0
    checkNum("=GAMMALN(10)", 12.8018274800815); // ln(9!)
    // NORMSDIST / NORMSINV (Spec 12) — phan phoi chuan tac
    checkNum("=NORMSDIST(0)", 0.5);
    checkNum("=NORMSDIST(1.96)", 0.9750021048517795);
    checkNum("=NORMSINV(0.5)", 0);
    checkNum("=NORMSDIST(NORMSINV(0.975))", 0.975); // khu hoi (xap xi)
    // === Batch ultracode (Spec 12): phan phoi xac suat ===
    checkNum("=BINOMDIST(2,10,0.3,0)", 0.23347444049999988);
    checkNum("=BINOMDIST(2,10,0.3,1)", 0.3827827863999998);
    checkNum("=POISSON(3,2,0)", 0.1804470443154836);
    checkNum("=POISSON(3,2,1)", 0.8571234604985472);
    checkNum("=EXPONDIST(1,2,0)", 0.2706705664732254);
    checkNum("=EXPONDIST(1,2,1)", 0.8646647167633873);
    checkNum("=WEIBULL(1,2,1,0)", 0.7357588823428847);
    checkNum("=WEIBULL(1,2,1,1)", 0.6321205588285577);
    checkNum("=NEGBINOMDIST(3,2,0.4)", 0.13824);
    checkNum("=NEGBINOMDIST(4,2,0.5)", 0.078125);
    checkNum("=HYPGEOMDIST(1,4,8,20)", 0.3632610939112487);
    checkNum("=HYPGEOMDIST(2,3,5,10)", 0.4166666666666667);
    // === Batch ultracode (Spec 12): doi don vi mo rong ===
    checkNum("=CONVERT(1,\"bar\",\"Pa\")", 100000.0);
    checkNum("=CONVERT(1,\"atm\",\"Pa\")", 101325.0);
    checkNum("=CONVERT(760,\"mmHg\",\"Pa\")", 101324.72);
    checkNum("=CONVERT(1,\"atm\",\"kPa\")", 101.325);
    checkNum("=CONVERT(1,\"kcal\",\"J\")", 4184.0);
    checkNum("=CONVERT(1,\"kWh\",\"Wh\")", 1000.0);
    checkNum("=CONVERT(1,\"HP\",\"W\")", 745.69987158227);
    checkNum("=CONVERT(1,\"psi\",\"Pa\")", 6894.757293168);
    checkNum("=CONVERT(1,\"eV\",\"J\")", 1.602176634e-19);
    // === Batch ultracode (Spec 12): toan/thong ke ===
    checkNum("=CONFIDENCE(0.05,2.5,50)", 0.69295191217483854);
    checkNum("=CONFIDENCE(0.1,1,16)", 0.41121340673786788);
    checkNum("=TRIMMEAN(A1:A3,0.2)", 20.0);
    checkNum("=MODE(1,2,2,3)", 2.0);
    checkNum("=MODE(H1:H3,1,1)", 1.0);
    checkNum("=SUMX2MY2(H1:H3,I1:I3)", -70.0);
    checkNum("=SUMX2PY2(H1:H3,I1:I3)", 98.0);
    checkNum("=SUMXMY2(H1:H3,I1:I3)", 38.0);
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
    // TREND / GROWTH (Spec 12)
    checkNum("=TREND(G1:G3,A1:A3,50)", 101); // y=2x+1 -> 2*50+1
    checkNum("=TREND(G1:G3,A1:A3,0)", 1);    // = intercept
    checkNum("=GROWTH(I1:I3,D1:D3,4)", 16);  // y=2^x -> 2^4=16
    // VARA / STDEVA (Spec 12) — tinh ca text=0
    checkNum("=VARA(A1:A3)", 100);   // {10,20,30} mau
    checkNum("=STDEVA(A1:A3)", 10);
    checkNum("=VARA(A1:B1)", 50);    // {10, "hello"->0}
    checkNum("=STDEVA(A1:B1)", 7.0710678118654755);
    // VARPA / STDEVPA (Spec 12) — tong the (chia n)
    checkNum("=VARPA(A1:A3)", 200.0/3.0);    // {10,20,30}
    checkNum("=STDEVPA(A1:A3)", 8.16496580927726);
    // ENCODEURL (Spec 12) — ma hoa URL
    checkStr("=ENCODEURL(\"a b\")", "a%20b");
    checkStr("=ENCODEURL(\"a&b\")", "a%26b");
    checkStr("=ENCODEURL(\"hello\")", "hello");
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
    // IMSQRT / IMPOWER (Spec 12) — dung khu hoi
    checkNum("=IMREAL(IMPRODUCT(IMSQRT(\"3+4i\"),IMSQRT(\"3+4i\")))", 3);     // sqrt(z)^2 = z
    checkNum("=IMAGINARY(IMPRODUCT(IMSQRT(\"3+4i\"),IMSQRT(\"3+4i\")))", 4);
    checkNum("=IMREAL(IMPOWER(\"2+i\",2))", 3);    // (2+i)^2 = 3+4i
    checkNum("=IMAGINARY(IMPOWER(\"2+i\",2))", 4);
    checkNum("=IMAGINARY(IMPOWER(\"1+i\",2))", 2); // (1+i)^2 = 2i
    // IMSIN / IMCOS (Spec 12)
    checkNum("=IMREAL(IMSIN(\"0\"))", 0);   // sin(0)=0
    checkNum("=IMREAL(IMCOS(\"0\"))", 1);   // cos(0)=1
    checkNum("=IMAGINARY(IMSIN(\"i\"))", 1.1752011936438014); // sin(i)=i*sinh(1)
    checkNum("=IMREAL(IMCOS(\"i\"))", 1.5430806348152437);    // cos(i)=cosh(1)
    checkNum("=IMAGINARY(IMCOS(\"i\"))", 0);
    // IMTAN / IMLOG10 / IMLOG2 (Spec 12)
    checkNum("=IMREAL(IMTAN(\"0\"))", 0);   // tan(0)=0
    checkNum("=IMAGINARY(IMTAN(\"i\"))", 0.7615941559557649); // tan(i)=i*tanh(1)
    checkNum("=IMREAL(IMLOG10(\"100\"))", 2); // log10(100)=2
    checkNum("=IMREAL(IMLOG2(\"8\"))", 3);    // log2(8)=3
    checkNum("=IMREAL(IMLOG10(\"1\"))", 0);
    // IMSINH / IMCOSH (Spec 12)
    checkNum("=IMREAL(IMSINH(\"0\"))", 0);   // sinh(0)=0
    checkNum("=IMREAL(IMCOSH(\"0\"))", 1);   // cosh(0)=1
    checkNum("=IMAGINARY(IMSINH(\"i\"))", 0.8414709848078965); // sinh(i)=i*sin(1)
    checkNum("=IMREAL(IMCOSH(\"i\"))", 0.5403023058681398);    // cosh(i)=cos(1)
    // IMCSC / IMSEC / IMCOT (Spec 12)
    checkNum("=IMREAL(IMSEC(\"0\"))", 1);   // 1/cos(0)=1
    checkNum("=IMAGINARY(IMCSC(\"i\"))", -0.8509181282393216); // 1/sin(i) = -i/sinh(1)
    checkNum("=IMREAL(IMSEC(\"i\"))", 0.6480542736638853);     // 1/cosh(1)
    checkNum("=IMAGINARY(IMCOT(\"i\"))", -1.3130352854993312); // -1/tanh(1)

    // --- DATEDIF unit MD/YM/YD (Spec 12) ---  s=2024-01-15, e=2026-03-20
    checkNum("=DATEDIF(DATE(2024,1,15),DATE(2026,3,20),\"Y\")", 2);
    checkNum("=DATEDIF(DATE(2024,1,15),DATE(2026,3,20),\"M\")", 26);
    checkNum("=DATEDIF(DATE(2024,1,15),DATE(2026,3,20),\"YM\")", 2);
    checkNum("=DATEDIF(DATE(2024,1,15),DATE(2026,3,20),\"MD\")", 5);
    checkNum("=DATEDIF(DATE(2024,1,15),DATE(2026,3,20),\"YD\")", 64);

    // --- NETWORKDAYSINTL / WORKDAYINTL (Spec 12) --- 7 ngay lien tiep phu moi thu trong tuan 1 lan
    checkNum("=NETWORKDAYSINTL(DATE(2026,6,1),DATE(2026,6,7))", 5);        // mac dinh T7+CN nghi
    checkNum("=NETWORKDAYSINTL(DATE(2026,6,1),DATE(2026,6,7),11)", 6);     // chi CN nghi
    checkNum("=NETWORKDAYSINTL(DATE(2026,6,1),DATE(2026,6,7),3)", 5);      // T2+T3 nghi
    checkNum("=NETWORKDAYSINTL(DATE(2026,6,1),DATE(2026,6,7),\"0000000\")", 7); // khong nghi
    checkNum("=NETWORKDAYSINTL(DATE(2026,6,1),DATE(2026,6,7),\"1111111\")", 0); // nghi het
    checkNum("=WORKDAYINTL(DATE(2026,6,1),5,\"0000000\")-DATE(2026,6,1)", 5);   // khong nghi -> +5 ngay

    // --- Regex functions (Spec 22.2) ---
    checkStr("=REGEXTEST(\"abc123\",\"[0-9]+\")", "true");
    checkStr("=REGEXTEST(\"abcdef\",\"[0-9]+\")", "false");
    checkStr("=REGEXTEST(\"ABC\",\"abc\")", "false");        // phan biet hoa/thuong
    checkStr("=REGEXTEST(\"ABC\",\"abc\",1)", "true");       // khong phan biet
    checkStr("=REGEXEXTRACT(\"Gia: 1234 dong\",\"[0-9]+\")", "1234");
    checkStr("=REGEXEXTRACT(\"a1b2c3\",\"[a-z]([0-9])\",2)", "1"); // nhom bat 1
    checkStr("=REGEXREPLACE(\"a1b2c3\",\"[0-9]\",\"*\")", "a*b*c*"); // thay tat ca
    checkStr("=REGEXREPLACE(\"a1b2c3\",\"[0-9]\",\"*\",2)", "a1b*c3"); // chi lan thu 2
    checkStr("=IFERROR(REGEXEXTRACT(\"abc\",\"[0-9]+\"),\"none\")", "none"); // khong khop -> #N/A

    // --- DATEVALUE / TIMEVALUE / DAYS360 (Spec 12) ---
    checkNum("=YEAR(DATEVALUE(\"2026-06-03\"))", 2026);
    checkNum("=MONTH(DATEVALUE(\"2026-06-03\"))", 6);
    checkNum("=DAY(DATEVALUE(\"2026-06-03\"))", 3);
    checkStr("=DATEVALUE(\"2026-06-03\")=DATE(2026,6,3)", "true"); // bang nhau
    checkNum("=TIMEVALUE(\"12:00:00\")", 0.5);
    checkNum("=TIMEVALUE(\"06:00\")", 0.25);
    checkNum("=DAYS360(DATE(2026,1,1),DATE(2026,2,1))", 30);
    checkNum("=DAYS360(DATE(2026,1,1),DATE(2026,12,31))", 360);
    checkNum("=DAYS360(DATE(2026,1,31),DATE(2026,3,31),1)", 60); // European

    // --- D-functions (Spec 27): CSDL K1:L5, tieu chi N1:N2 (Loai=A), O1:O2 (Tien>15) ---
    checkNum("=DSUM(K1:L5,\"Tien\",N1:N2)", 40);      // Loai=A -> 10+30
    checkNum("=DCOUNT(K1:L5,\"Tien\",N1:N2)", 2);     // 2 hang Loai=A
    checkNum("=DAVERAGE(K1:L5,\"Tien\",N1:N2)", 20);  // (10+30)/2
    checkNum("=DMAX(K1:L5,\"Tien\",N1:N2)", 30);
    checkNum("=DMIN(K1:L5,\"Tien\",N1:N2)", 10);
    checkNum("=DSUM(K1:L5,\"Tien\",O1:O2)", 90);      // Tien>15 -> 20+30+40
    checkNum("=DSUM(K1:L5,2,N1:N2)", 40);             // field theo so cot (2)
    // DGET (P1:P2 = Tien=10 -> dung 1 ban ghi), DVAR/DSTDEV (Loai=A -> 10,30)
    checkStr("=DGET(K1:L5,\"Loai\",P1:P2)", "A");
    checkNum("=DGET(K1:L5,\"Tien\",P1:P2)", 10);
    checkStr("=IFERROR(DGET(K1:L5,\"Tien\",N1:N2),\"nhieu\")", "nhieu"); // 2 khop -> #NUM
    checkNum("=DVAR(K1:L5,\"Tien\",N1:N2)", 200);     // ((10-20)^2+(30-20)^2)/1
    checkNum("=DVARP(K1:L5,\"Tien\",N1:N2)", 100);    // /2
    checkNum("=DSTDEVP(K1:L5,\"Tien\",N1:N2)", 10);   // sqrt(100)

    std::printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
