// Test headless cho CSV I/O (csvio). Chi in ASCII.
#include "io/Csv.h"
#include "io/Xlsx.h"
#include <QCoreApplication>
#include <QTemporaryDir>
#include <QFile>
#include <cstdio>

using csvio::Grid;

static int g_pass = 0, g_fail = 0;
static void ok(bool cond, const char *name) {
    if (cond) ++g_pass; else { ++g_fail; std::printf("FAIL %s\n", name); }
}

static Grid g(std::initializer_list<std::initializer_list<const char *>> rows) {
    Grid out;
    for (auto r : rows) {
        QVector<QString> row;
        for (auto c : r) row.push_back(QString::fromUtf8(c));
        out.push_back(row);
    }
    return out;
}

int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);

    // --- parse co ban (dau phay) ---
    {
        Grid r = csvio::parse("a,b,c\n1,2,3\n", QLatin1Char(','));
        ok(r.size() == 2 && r[0].size() == 3, "parse 2 hang 3 cot");
        ok(r[1][2] == "3", "parse o cuoi = 3");
    }

    // --- truong co nhay: dau phay & xuong dong & nhay long nhau ---
    {
        Grid r = csvio::parse("\"a,b\",\"line1\nline2\",\"say \"\"hi\"\"\"\n", QLatin1Char(','));
        ok(r.size() == 1 && r[0].size() == 3, "quoted: 1 hang 3 cot");
        ok(r[0][0] == "a,b", "quoted dau phay trong o");
        ok(r[0][1] == "line1\nline2", "quoted xuong dong trong o");
        ok(r[0][2] == "say \"hi\"", "quoted nhay thoat \"\"");
    }

    // --- CRLF ---
    {
        Grid r = csvio::parse("a,b\r\nc,d\r\n", QLatin1Char(','));
        ok(r.size() == 2 && r[1][0] == "c", "CRLF tach hang");
    }

    // --- sniff delimiter ---
    {
        ok(csvio::sniffDelimiter("a;b;c\n1;2;3") == QLatin1Char(';'), "sniff ;");
        ok(csvio::sniffDelimiter("a\tb\tc") == QLatin1Char('\t'), "sniff tab");
        ok(csvio::sniffDelimiter("a|b|c") == QLatin1Char('|'), "sniff |");
        ok(csvio::sniffDelimiter("only one field") == QLatin1Char(','), "sniff mac dinh ,");
        // dau phay trong nhay khong duoc dem
        ok(csvio::sniffDelimiter("\"x,y,z\";a") == QLatin1Char(';'), "sniff bo qua trong nhay");
    }

    // --- normalize: chu nhat hoa ---
    {
        Grid r = csvio::normalize(g({{"a"}, {"b", "c", "d"}}));
        ok(r[0].size() == 3 && r[1].size() == 3, "normalize cung so cot");
        ok(r[0][1].isEmpty() && r[0][2].isEmpty(), "normalize chen o rong");
    }
    {
        Grid r = csvio::normalize(Grid{});
        ok(r.size() == 1 && r[0].size() == 1, "normalize rong -> 1x1");
    }

    // --- toCsv: trich nhay khi can ---
    {
        QString s = csvio::toCsv(g({{"plain", "a,b", "q\"x", "n\nl"}}));
        ok(s == "plain,\"a,b\",\"q\"\"x\",\"n\nl\"\r\n", "toCsv trich nhay dung");
    }

    // --- round-trip qua file (co BOM, tieng Viet) ---
    {
        QTemporaryDir dir;
        QString path = dir.path() + "/rt.csv";
        Grid src = g({{"Ten", "Gia"}, {"Ca phe", "25,000"}, {"=1+2", "x\"y"}});
        ok(csvio::saveCsv(path, src), "saveCsv tra true");

        // File phai bat dau bang BOM UTF-8.
        QFile f(path); f.open(QIODevice::ReadOnly);
        QByteArray head = f.read(3); f.close();
        ok(head == QByteArray("\xEF\xBB\xBF"), "file co BOM UTF-8");

        bool okLoad = false;
        Grid back = csvio::loadCsv(path, &okLoad);
        ok(okLoad, "loadCsv tra ok");
        ok(back == src, "round-trip giu nguyen du lieu");
    }

    // --- load file khong ton tai ---
    {
        bool okLoad = true;
        Grid r = csvio::loadCsv("Z:/khong/ton/tai.csv", &okLoad);
        ok(!okLoad, "load file thieu -> ok=false");
        ok(r.size() == 1 && r[0].size() == 1, "load loi -> luoi 1x1");
    }

    // --- XLSX round-trip qua QXlsx (gia tri + cong thuc + so + o gop) ---
    {
        QTemporaryDir dir;
        QString path = dir.path() + "/rt.xlsx";
        Grid src = g({{"Ten", "SL", "Tong"},
                      {"Ca phe", "2", "=B2*10"},
                      {"Tra", "3", "=B3*10"}});
        QVector<xlsxio::Merge> merges = {{0, 0, 0, 2}}; // gop A1:C1

        ok(xlsxio::saveXlsx(path, "BangGia", src, merges), "saveXlsx tra true");
        ok(QFile::exists(path), "file xlsx ton tai");

        xlsxio::Sheet sh;
        ok(xlsxio::loadXlsx(path, sh), "loadXlsx tra true");
        ok(sh.name == "BangGia", "giu ten sheet");

        // Gia tri: chuoi + so giu nguyen text; cong thuc giu nguyen "=...".
        ok(sh.rows.size() >= 3 && sh.rows[0][0] == "Ten", "xlsx A1=Ten");
        ok(sh.rows[1][0] == "Ca phe", "xlsx A2=Ca phe");
        ok(sh.rows[1][1] == "2", "xlsx B2=2 (so)");
        ok(sh.rows[1][2] == "=B2*10", "xlsx C2 giu cong thuc");

        // O gop round-trip.
        ok(sh.merges.size() == 1, "xlsx 1 vung gop");
        ok(sh.merges[0] == (xlsxio::Merge{0, 0, 0, 2}), "xlsx vung gop A1:C1");
    }

    // --- XLSX load file khong ton tai ---
    {
        xlsxio::Sheet sh;
        ok(!xlsxio::loadXlsx("Z:/khong/co.xlsx", sh), "load xlsx thieu -> false");
    }

    std::printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
