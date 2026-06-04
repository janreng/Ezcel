// Test headless cho SpreadsheetModel (offscreen). CHI in ASCII.
#include "model/SpreadsheetModel.h"
#include "model/TextSearch.h"
#include "model/CondFormat.h"
#include "model/Filter.h"
#include "model/PasteOps.h"
#include "model/Stats.h"
#include "model/AutoSum.h"
#include "model/Validation.h"
#include "model/CellStyles.h"
#include "model/GotoSpecial.h"
#include <QGuiApplication>
#include <QFont>
#include <QColor>
#include <cstdio>

static int g_pass = 0, g_fail = 0;
static void ok(bool cond, const char *name) {
    if (cond) ++g_pass; else { ++g_fail; std::printf("FAIL %s\n", name); }
}

static QString disp(SpreadsheetModel &m, int r, int c) {
    return m.data(m.index(r, c), Qt::DisplayRole).toString();
}
static void put(SpreadsheetModel &m, int r, int c, const QString &v) {
    m.setData(m.index(r, c), v, Qt::EditRole);
}

int main(int argc, char **argv) {
    qputenv("QT_QPA_PLATFORM", "offscreen");
    QGuiApplication app(argc, argv);

    SpreadsheetModel m;
    m.resizeGrid(10, 5);

    put(m, 0, 0, "10");           // A1
    put(m, 1, 0, "20");           // A2
    put(m, 2, 0, "=A1+A2");       // A3
    ok(disp(m, 2, 0) == "30", "formula A3=30");

    // Recalc chon loc khi A1 doi.
    put(m, 0, 0, "100");
    ok(disp(m, 2, 0) == "120", "recalc A3=120 sau khi A1=100");

    // Undo: A1 ve 10 -> A3=30.
    ok(m.undo(), "undo tra ve true");
    ok(disp(m, 0, 0) == "10", "undo A1=10");
    ok(disp(m, 2, 0) == "30", "undo A3=30");

    // Redo: A1=100 -> A3=120.
    ok(m.redo(), "redo tra ve true");
    ok(disp(m, 2, 0) == "120", "redo A3=120");

    // Hien cong thuc (Ctrl+`).
    m.setShowFormulas(true);
    ok(disp(m, 2, 0) == "=A1+A2", "show formulas hien raw");
    m.setShowFormulas(false);
    ok(disp(m, 2, 0) == "120", "tat show formulas hien ket qua");

    // Can le: so canh phai mac dinh.
    int al = m.data(m.index(0, 0), Qt::TextAlignmentRole).toInt();
    ok((al & Qt::AlignRight) != 0, "so canh phai mac dinh");

    // Dinh dang: bold.
    SpreadsheetModel::Format f; f.insert("bold", true);
    m.setFormat(0, 0, 0, 0, f);
    QVariant fv = m.data(m.index(0, 0), Qt::FontRole);
    ok(fv.canConvert<QFont>() && fv.value<QFont>().bold(), "set bold -> font.bold()");

    // Dinh dang: mau nen.
    SpreadsheetModel::Format f2; f2.insert("bg", "#ff0000");
    m.setFormat(1, 0, 1, 0, f2);
    QVariant bg = m.data(m.index(1, 0), Qt::BackgroundRole);
    ok(bg.canConvert<QColor>() && bg.value<QColor>().red() == 255, "set bg do");

    // Undo dinh dang bg.
    m.undo();
    ok(!m.data(m.index(1, 0), Qt::BackgroundRole).isValid(), "undo bo bg");

    // --- autofill chuoi so (doc) ---
    put(m, 0, 1, "1");  // B1
    put(m, 1, 1, "2");  // B2
    m.autofillVertical(1, 0, 1, 4);
    ok(disp(m, 2, 1) == "3" && disp(m, 4, 1) == "5", "autofill so doc B3=3 B5=5");

    // --- autofill tang so duoi (text) ---
    put(m, 0, 2, "Item1"); // C1
    m.autofillVertical(2, 0, 0, 2);
    ok(disp(m, 1, 2) == "Item2" && disp(m, 2, 2) == "Item3", "autofill Item1->Item2/Item3");

    // --- autofill ngang ---
    put(m, 7, 0, "5");
    put(m, 7, 1, "10");
    m.autofillHorizontal(7, 0, 1, 3);
    ok(disp(m, 7, 2) == "15" && disp(m, 7, 3) == "20", "autofill ngang 15,20");

    // --- autofill chuoi lich (Spec 05): Jan -> Feb/Mar, Mon -> Tue ---
    put(m, 0, 4, "Jan"); // E1
    m.autofillVertical(4, 0, 0, 2);
    ok(disp(m, 1, 4) == "Feb" && disp(m, 2, 4) == "Mar", "autofill Jan->Feb/Mar");
    put(m, 5, 4, "Mon"); // E6
    m.autofillVertical(4, 5, 5, 6);
    ok(disp(m, 6, 4) == "Tue", "autofill Mon->Tue");

    // --- paste block voi offset cong thuc ---
    QVector<QVector<QString>> block = {{"=A1+A2"}};
    m.pasteBlock(5, 1, block, 2, 0); // anchor (2,0) -> (5,1): drow=3 dcol=1
    ok(m.data(m.index(5, 1), Qt::EditRole).toString() == "=B4+B5", "paste offset =B4+B5");

    // --- clear range + undo ---
    put(m, 0, 3, "zzz"); // D1
    m.clearRange(0, 3, 0, 3);
    ok(disp(m, 0, 3) == "", "clear D1 rong");
    m.undo();
    ok(disp(m, 0, 3) == "zzz", "undo clear -> zzz");

    // --- clear formats / clear all + undo (Spec 09) ---
    {
        SpreadsheetModel mm;
        mm.resizeGrid(5, 3);
        put(mm, 0, 0, "hello");
        SpreadsheetModel::Format bf; bf.insert("bold", true);
        mm.setFormat(0, 0, 0, 0, bf);
        ok(mm.formatAt(0, 0).value("bold").toBool(), "co dinh dang bold");

        // Clear Formats: giu noi dung, mat dinh dang.
        mm.clearFormatsRange(0, 0, 0, 0);
        ok(mm.formatAt(0, 0).isEmpty(), "clearFormats -> het dinh dang");
        ok(mm.data(mm.index(0, 0), Qt::EditRole).toString() == "hello", "clearFormats giu noi dung");
        mm.undo();
        ok(mm.formatAt(0, 0).value("bold").toBool(), "undo clearFormats -> bold lai");

        // Clear All: mat ca noi dung lan dinh dang, 1 buoc undo khoi phuc het.
        mm.clearAllRange(0, 0, 0, 0);
        ok(mm.formatAt(0, 0).isEmpty(), "clearAll -> het dinh dang");
        ok(mm.data(mm.index(0, 0), Qt::EditRole).toString() == "", "clearAll -> het noi dung");
        mm.undo();
        ok(mm.data(mm.index(0, 0), Qt::EditRole).toString() == "hello", "undo clearAll -> noi dung lai");
        ok(mm.formatAt(0, 0).value("bold").toBool(), "undo clearAll -> bold lai");
    }

    // --- gop o (merge) ---
    {
        SpreadsheetModel mm;
        mm.resizeGrid(8, 5);
        put(mm, 0, 0, "Tieu de"); // A1
        put(mm, 0, 1, "xoa di");  // B1 (se bi xoa khi gop)
        put(mm, 1, 0, "xoa di");  // A2
        mm.mergeCells(0, 0, 1, 1); // gop A1:B1:A2:B2
        ok(mm.merges().size() == 1, "merge: 1 vung");
        ok(disp(mm, 0, 0) == "Tieu de", "merge: giu noi dung goc trai");
        ok(disp(mm, 0, 1) == "" && disp(mm, 1, 0) == "", "merge: xoa o khong phai goc");
        auto at = mm.mergeAt(1, 1);
        ok(at.has_value() && at->top == 0 && at->right == 1, "mergeAt tra dung vung");
        ok(!mm.mergeAt(3, 3).has_value(), "mergeAt ngoai vung -> rong");

        // undo: bo gop + khoi phuc noi dung
        mm.undo();
        ok(mm.merges().isEmpty(), "undo merge: het vung gop");
        ok(disp(mm, 0, 1) == "xoa di" && disp(mm, 1, 0) == "xoa di", "undo merge: khoi phuc o");
        mm.redo();
        ok(mm.merges().size() == 1 && disp(mm, 0, 1) == "", "redo merge lai");

        // toggle: dang gop -> bo gop
        mm.toggleMerge(0, 0, 1, 1);
        ok(mm.merges().isEmpty(), "toggle bo gop khi dang gop");
        // toggle: chua gop -> gop
        mm.toggleMerge(0, 0, 1, 1);
        ok(mm.merges().size() == 1, "toggle gop khi chua gop");

        // gop o don le -> khong tao vung
        mm.unmergeCells(0, 0, 1, 1);
        mm.mergeCells(3, 3, 3, 3);
        ok(mm.merges().isEmpty(), "gop 1 o khong tao vung");

        // gop vung moi giao vung cu -> thay the
        mm.mergeCells(0, 0, 0, 2); // A1:C1
        mm.mergeCells(0, 1, 0, 3); // B1:D1 (giao) -> bo A1:C1
        ok(mm.merges().size() == 1 && mm.mergeAt(0, 3).has_value(), "vung moi thay vung giao");
    }

    // --- formatAt + number_format presets (thanh dinh dang dung) ---
    {
        SpreadsheetModel mm;
        mm.resizeGrid(5, 3);
        put(mm, 0, 0, "1234.5");
        SpreadsheetModel::Format nf; nf.insert("number_format", "#,##0.00");
        mm.setFormat(0, 0, 0, 0, nf);
        ok(mm.formatAt(0, 0).value("number_format").toString() == "#,##0.00", "formatAt tra number_format");
        ok(disp(mm, 0, 0) == "1,234.50", "number_format #,##0.00 -> 1,234.50");

        put(mm, 1, 0, "0.25");
        SpreadsheetModel::Format pf; pf.insert("number_format", "0.00%");
        mm.setFormat(1, 0, 1, 0, pf);
        ok(disp(mm, 1, 0) == "25.00%", "number_format 0.00% -> 25.00%");

        // bo number_format (value null) -> hien lai thuong
        SpreadsheetModel::Format clr; clr.insert("number_format", QVariant());
        mm.setFormat(0, 0, 0, 0, clr);
        ok(!mm.formatAt(0, 0).contains("number_format"), "xoa number_format khi value null");
    }

    // --- chen/xoa hang-cot ---
    {
        SpreadsheetModel mm;
        mm.resizeGrid(5, 4);
        put(mm, 0, 0, "A1"); put(mm, 1, 0, "A2"); put(mm, 2, 0, "A3");
        SpreadsheetModel::Format bold; bold.insert("bold", true);
        mm.setFormat(2, 0, 2, 0, bold);          // dinh dang o A3

        mm.insertRows(1, 1);                      // chen 1 hang truoc hang index 1
        ok(mm.rowCount() == 6, "insertRows tang so hang");
        ok(disp(mm, 0, 0) == "A1" && disp(mm, 1, 0) == "" && disp(mm, 2, 0) == "A2",
           "insertRows doi du lieu xuong");
        QVariant fv2 = mm.data(mm.index(3, 0), Qt::FontRole); // A3 doi xuong hang 3
        ok(fv2.canConvert<QFont>() && fv2.value<QFont>().bold(), "insertRows doi dinh dang theo");

        mm.undo();
        ok(mm.rowCount() == 5 && disp(mm, 1, 0) == "A2", "undo insertRows");
        mm.redo();
        ok(mm.rowCount() == 6 && disp(mm, 2, 0) == "A2", "redo insertRows");

        mm.removeRows(1, 1);                       // xoa lai hang vua chen
        ok(mm.rowCount() == 5 && disp(mm, 1, 0) == "A2", "removeRows xoa hang");

        // cot
        put(mm, 0, 1, "B1");
        mm.insertColumns(1, 2);
        ok(mm.columnCount() == 6 && disp(mm, 0, 3) == "B1", "insertColumns doi cot phai");
        mm.removeColumns(1, 2);
        ok(mm.columnCount() == 4 && disp(mm, 0, 1) == "B1", "removeColumns xoa cot");

        // merge doi theo khi chen hang
        SpreadsheetModel mm2;
        mm2.resizeGrid(6, 6);
        mm2.mergeCells(2, 1, 3, 2);                // C3:B4 vung [2,1]-[3,2]
        mm2.insertRows(0, 1);                       // chen 1 hang dau -> vung tut xuong
        auto at = mm2.mergeAt(3, 1);
        ok(at.has_value() && at->top == 3 && at->bottom == 4, "insertRows doi vung gop xuong");
        mm2.removeRows(0, 1);
        auto at2 = mm2.mergeAt(2, 1);
        ok(at2.has_value() && at2->top == 2, "removeRows doi vung gop len lai");
    }

    // --- tim & thay the ---
    {
        // replaceSubstr
        ok(textsearch::replaceSubstr("aXaXa", "X", "-", true) == "a-a-a", "replaceSubstr case");
        ok(textsearch::replaceSubstr("aXxa", "x", "-", false) == "a--a", "replaceSubstr ci");
        ok(textsearch::replaceSubstr("aXxa", "x", "-", true) == "aX-a", "replaceSubstr case-sensitive");

        // findNext wrap-around
        QVector<QVector<QString>> grid = {{"foo", "bar"}, {"baz", "Foo"}};
        auto cell = [&](int r, int c) { return grid[r][c]; };
        auto h1 = textsearch::findNext(2, 2, 0, 0, "foo", false, cell); // sau (0,0): tim Foo o (1,1)
        ok(h1.has_value() && h1->first == 1 && h1->second == 1, "findNext ci -> (1,1)");
        auto h2 = textsearch::findNext(2, 2, 0, 0, "foo", true, cell);  // case: bo qua Foo, vong lai (0,0)
        ok(h2.has_value() && h2->first == 0 && h2->second == 0, "findNext case wrap -> (0,0)");
        auto h3 = textsearch::findNext(2, 2, 0, -1, "zzz", false, cell);
        ok(!h3.has_value(), "findNext khong thay -> rong");

        // findAll (Spec 32): quet ca luoi. grid: foo(0,0), Foo(1,1)
        auto all = textsearch::findAll(2, 2, "foo", false, cell); // ci -> ca hai
        ok(all.size() == 2 && all.contains(qMakePair(0, 0)) && all.contains(qMakePair(1, 1)), "findAll ci -> 2 o");
        auto allCase = textsearch::findAll(2, 2, "foo", true, cell); // case -> chi foo(0,0)
        ok(allCase.size() == 1 && allCase.contains(qMakePair(0, 0)), "findAll case -> 1 o");
        ok(textsearch::findAll(2, 2, "", false, cell).isEmpty(), "findAll rong -> rong");
        ok(textsearch::findAll(2, 2, "zzz", false, cell).isEmpty(), "findAll khong thay -> rong");

        // model.replaceAll undoable
        SpreadsheetModel mm;
        mm.resizeGrid(3, 3);
        put(mm, 0, 0, "cat"); put(mm, 0, 1, "CAT"); put(mm, 1, 0, "dog");
        int n = mm.replaceAll("cat", "cow", false);   // ci: doi 2 o
        ok(n == 2, "replaceAll ci doi 2 o");
        ok(disp(mm, 0, 0) == "cow" && disp(mm, 0, 1) == "cow", "replaceAll thay dung");
        mm.undo();
        ok(disp(mm, 0, 0) == "cat" && disp(mm, 0, 1) == "CAT", "undo replaceAll");
        int n2 = mm.replaceAll("cat", "cow", true);    // case: chi 1 o
        ok(n2 == 1 && disp(mm, 0, 1) == "CAT", "replaceAll case-sensitive 1 o");
    }

    // --- sortRange (tich hop engine sort vao model, undoable) ---
    {
        SpreadsheetModel mm;
        mm.resizeGrid(5, 3);
        // vung A1:B3 — cot A khoa
        put(mm, 0, 0, "3"); put(mm, 0, 1, "ba");
        put(mm, 1, 0, "1"); put(mm, 1, 1, "mot");
        put(mm, 2, 0, "2"); put(mm, 2, 1, "hai");
        mm.sortRange(0, 0, 2, 1, 0, true);   // sap tang theo cot A
        ok(disp(mm, 0, 0) == "1" && disp(mm, 0, 1) == "mot", "sortRange tang: hang 1->mot");
        ok(disp(mm, 2, 0) == "3" && disp(mm, 2, 1) == "ba", "sortRange tang: hang cuoi->ba");
        // hang phai di theo (khoa gan voi du lieu)
        mm.undo();
        ok(disp(mm, 0, 0) == "3" && disp(mm, 0, 1) == "ba", "undo sortRange ve nguyen");
        mm.sortRange(0, 0, 2, 1, 0, false);  // giam dan
        ok(disp(mm, 0, 0) == "3" && disp(mm, 2, 0) == "1", "sortRange giam: 3..1");
    }

    // --- sortRangeMulti (sap nhieu cap, Spec 15) ---
    {
        SpreadsheetModel mm;
        mm.resizeGrid(6, 3);
        // cot A = nhom, cot B = diem. Sap A tang, roi B giam.
        const char *A[] = {"Bac", "Nam", "Bac", "Nam", "Bac"};
        const char *B[] = {"10",  "5",   "30",  "20",  "20"};
        for (int i = 0; i < 5; ++i) { put(mm, i, 0, A[i]); put(mm, i, 1, B[i]); }
        mm.sortRangeMulti(0, 0, 4, 1, {{0, true}, {1, false}});
        // Bac truoc (tang chu): Bac/30, Bac/20, Bac/10, roi Nam/20, Nam/5
        ok(disp(mm, 0, 0) == "Bac" && disp(mm, 0, 1) == "30", "cap1 Bac, cap2 diem giam -> 30 dau");
        ok(disp(mm, 2, 0) == "Bac" && disp(mm, 2, 1) == "10", "Bac/10 cuoi nhom Bac");
        ok(disp(mm, 3, 0) == "Nam" && disp(mm, 3, 1) == "20", "Nam/20 truoc Nam/5");
        ok(disp(mm, 4, 0) == "Nam" && disp(mm, 4, 1) == "5",  "Nam/5 cuoi");
        mm.undo();
        ok(disp(mm, 0, 1) == "10", "undo sortRangeMulti ve nguyen");
    }

    // --- dinh dang co dieu kien ---
    {
        using cond::Op;
        ok(cond::match(QVariant(10.0), Op::GreaterThan, 5, 0, ""), "10 > 5");
        ok(!cond::match(QVariant(3.0), Op::GreaterThan, 5, 0, ""), "3 !> 5");
        ok(cond::match(QVariant(5.0), Op::Equal, 5, 0, ""), "5 == 5");
        ok(cond::match(QVariant(7.0), Op::Between, 5, 10, ""), "7 in [5,10]");
        ok(!cond::match(QVariant(12.0), Op::Between, 5, 10, ""), "12 not in [5,10]");
        ok(cond::match(QVariant(QString("Hello")), Op::Contains, 0, 0, "ell"), "contains ell");
        ok(!cond::match(QVariant(QString("abc")), Op::GreaterThan, 5, 0, ""), "chuoi !> 5");

        // tich hop model: BackgroundRole doi mau khi khop
        SpreadsheetModel mm;
        mm.resizeGrid(4, 3);
        put(mm, 0, 0, "100"); put(mm, 1, 0, "20");
        cond::Rule rule; rule.top = 0; rule.left = 0; rule.bottom = 3; rule.right = 0;
        rule.op = Op::GreaterThan; rule.v1 = 50; rule.bg = "#ffcccc";
        mm.addCondRule(rule);
        QVariant bg0 = mm.data(mm.index(0, 0), Qt::BackgroundRole); // 100 > 50 -> to mau
        QVariant bg1 = mm.data(mm.index(1, 0), Qt::BackgroundRole); // 20 -> khong
        ok(bg0.canConvert<QColor>() && bg0.value<QColor>().red() == 255, "cond to mau o > 50");
        ok(!bg1.isValid(), "cond khong to o <= 50");
        mm.clearCondRules();
        ok(!mm.data(mm.index(0, 0), Qt::BackgroundRole).isValid(), "clear cond -> het mau");

        // --- Data Bar (Spec 08) ---
        // dataBarFraction thuan
        ok(cond::dataBarFraction(5, 0, 10) == 0.5, "fraction 5 trong [0,10] = 0.5");
        ok(cond::dataBarFraction(0, 0, 10) == 0.0, "fraction min = 0");
        ok(cond::dataBarFraction(10, 0, 10) == 1.0, "fraction max = 1");
        ok(cond::dataBarFraction(-5, 0, 10) == 0.0, "duoi min -> kep 0");
        ok(cond::dataBarFraction(7, 5, 5) == 1.0, "max<=min -> 1");
        // tich hop model: DataBarRole tra t.le + mau
        SpreadsheetModel db;
        db.resizeGrid(5, 2);
        put(db, 0, 0, "0"); put(db, 1, 0, "5"); put(db, 2, 0, "10");
        db.addDataBar(cond::DataBar{0, 0, 2, 0, "#638EC6"});
        QVariant r0 = db.data(db.index(0, 0), SpreadsheetModel::DataBarRole);
        QVariant r1 = db.data(db.index(1, 0), SpreadsheetModel::DataBarRole);
        ok(r0.typeId() == QMetaType::QVariantList && r0.toList()[0].toDouble() == 0.0, "databar o min = 0");
        ok(r1.toList()[0].toDouble() == 0.5 && r1.toList()[1].toString() == "#638EC6", "databar giua = 0.5 + mau");
        ok(!db.data(db.index(3, 0), SpreadsheetModel::DataBarRole).isValid(), "ngoai vung -> khong co thanh");
        // o khong phai so -> khong co thanh
        put(db, 0, 0, "abc");
        ok(!db.data(db.index(0, 0), SpreadsheetModel::DataBarRole).isValid(), "o chu -> khong thanh");
        db.clearDataBars();
        ok(!db.data(db.index(1, 0), SpreadsheetModel::DataBarRole).isValid(), "clear databar -> het");

        // --- Color Scale (Spec 08) ---
        // lerpHex + colorScale thuan
        ok(cond::lerpHex("#000000", "#ffffff", 0.5) == "#808080", "lerp giua den-trang = #808080");
        ok(cond::lerpHex("#000000", "#ffffff", 0.0) == "#000000", "lerp t=0 = dau");
        ok(cond::colorScale(0.0, "#ff0000", "", "#00ff00") == "#ff0000", "scale 2 mau t=0 = low");
        ok(cond::colorScale(1.0, "#ff0000", "", "#00ff00") == "#00ff00", "scale 2 mau t=1 = high");
        ok(cond::colorScale(0.5, "#000000", "#808080", "#ffffff") == "#808080", "scale 3 mau t=0.5 = mid");
        // tich hop model: BackgroundRole tra mau noi suy
        SpreadsheetModel cs;
        cs.resizeGrid(5, 2);
        put(cs, 0, 0, "0"); put(cs, 1, 0, "10");
        cs.addColorScale(cond::ColorScale{0, 0, 1, 0, "#000000", QString(), "#ffffff"});
        QVariant bgLo = cs.data(cs.index(0, 0), Qt::BackgroundRole); // min -> low #000000
        QVariant bgHi = cs.data(cs.index(1, 0), Qt::BackgroundRole); // max -> high #ffffff
        ok(bgLo.value<QColor>() == QColor("#000000"), "color scale min = low");
        ok(bgHi.value<QColor>() == QColor("#ffffff"), "color scale max = high");
        ok(!cs.data(cs.index(3, 0), Qt::BackgroundRole).isValid(), "ngoai vung -> khong to");
        cs.clearColorScales();
        ok(!cs.data(cs.index(0, 0), Qt::BackgroundRole).isValid(), "clear color scale -> het");

        // --- Icon Set (Spec 08) ---
        ok(cond::iconIndex(0.0, 3) == 0, "icon frac 0 -> 0");
        ok(cond::iconIndex(0.5, 3) == 1, "icon frac 0.5 -> 1");
        ok(cond::iconIndex(1.0, 3) == 2, "icon frac 1 -> 2 (cao nhat)");
        ok(cond::iconIndex(0.9, 3) == 2, "icon frac 0.9 -> 2");
        ok(cond::iconIndex(0.2, 3) == 0, "icon frac 0.2 -> 0");
        // tich hop model: IconSetRole tra mau (do/vang/xanh)
        SpreadsheetModel is;
        is.resizeGrid(5, 2);
        put(is, 0, 0, "0"); put(is, 1, 0, "5"); put(is, 2, 0, "10");
        is.addIconSet(cond::IconSet{0, 0, 2, 0, 3});
        ok(is.data(is.index(0, 0), SpreadsheetModel::IconSetRole).toString() == "#e74c3c", "icon min = do");
        ok(is.data(is.index(2, 0), SpreadsheetModel::IconSetRole).toString() == "#2ecc71", "icon max = xanh");
        ok(!is.data(is.index(3, 0), SpreadsheetModel::IconSetRole).isValid(), "ngoai vung -> khong icon");
        put(is, 0, 0, "abc");
        ok(!is.data(is.index(0, 0), SpreadsheetModel::IconSetRole).isValid(), "o chu -> khong icon");
        is.clearIconSets();
        ok(!is.data(is.index(1, 0), SpreadsheetModel::IconSetRole).isValid(), "clear icon set -> het");

        // --- Bảo vệ trang tính (Spec 29) ---
        SpreadsheetModel pr;
        pr.resizeGrid(4, 3);
        // Chưa bảo vệ -> ô sửa được
        ok(pr.flags(pr.index(0, 0)) & Qt::ItemIsEditable, "chua bao ve -> sua duoc");
        pr.setCellsLocked(1, 0, 1, 0, false); // mở khóa B... (hàng1 cột0)
        pr.setSheetProtected(true);
        ok(!(pr.flags(pr.index(0, 0)) & Qt::ItemIsEditable), "bao ve: o khoa -> khong sua");
        ok(pr.flags(pr.index(1, 0)) & Qt::ItemIsEditable, "bao ve: o da mo khoa -> sua duoc");
        ok(!pr.setData(pr.index(0, 0), QStringLiteral("x"), Qt::EditRole), "setData o khoa bi chan");
        ok(pr.setData(pr.index(1, 0), QStringLiteral("ok"), Qt::EditRole), "setData o mo khoa duoc");
        pr.setSheetProtected(false);
        ok(pr.flags(pr.index(0, 0)) & Qt::ItemIsEditable, "tat bao ve -> sua lai duoc");
    }

    // --- loc du lieu ---
    {
        QVector<QString> col{"Ten", "Cafe", "Tra sua", "Banh", "cafe sua"};
        auto h = filterutil::rowsToHide(col, "cafe"); // ci: giu Cafe(1) & cafe sua(4); an 2,3
        ok(h.size() == 2 && h.contains(2) && h.contains(3), "loc 'cafe' an 2 hang");
        ok(!h.contains(0), "hang tieu de luon giu");
        ok(filterutil::rowsToHide(col, "").isEmpty(), "loc rong -> khong an");
        ok(filterutil::rowsToHide(col, "xyz").size() == 4, "loc khong khop -> an het (tru header)");
        // loc theo gia tri (Spec 15)
        auto uv = filterutil::uniqueValues(col); // bo header -> 4 gia tri unique sap xep
        ok(uv.size() == 4 && uv.first() == "Banh", "uniqueValues sap xep, bo header");
        QSet<QString> keep{"Cafe", "Banh"};
        auto hv = filterutil::rowsToHideByValues(col, keep); // an Tra sua(2), cafe sua(4)
        ok(hv.size() == 2 && hv.contains(2) && hv.contains(4), "rowsToHideByValues giu Cafe/Banh");
        ok(!hv.contains(0), "header luon giu");
    }

    // --- loc theo dieu kien so (Number Filters, Spec 15) ---
    {
        using filterutil::NumOp;
        // header + 10,20,30,40,50
        QVector<QString> col{"Diem", "10", "20", "30", "40", "50"};
        auto gt = filterutil::rowsToHideByNumber(col, NumOp::Gt, 25);
        ok(gt.size() == 2 && gt.contains(1) && gt.contains(2), ">25 an 10,20");
        auto le = filterutil::rowsToHideByNumber(col, NumOp::Le, 20);
        ok(le.size() == 3 && le.contains(3) && le.contains(5), "<=20 an 30,40,50");
        auto bw = filterutil::rowsToHideByNumber(col, NumOp::Between, 20, 40);
        ok(bw.size() == 2 && bw.contains(1) && bw.contains(5), "between 20..40 giu 20,30,40");
        auto nb = filterutil::rowsToHideByNumber(col, NumOp::NotBetween, 20, 40);
        ok(nb.size() == 3 && nb.contains(2) && nb.contains(3) && nb.contains(4), "not between an 20,30,40");
        auto eq = filterutil::rowsToHideByNumber(col, NumOp::Eq, 30);
        ok(eq.size() == 4 && !eq.contains(3), "=30 giu moi hang 30");
        // trung binh = 30 -> above giu 40,50; an 10,20,30 (+header giu)
        auto ab = filterutil::rowsToHideByNumber(col, NumOp::AboveAvg, 0);
        ok(ab.size() == 3 && ab.contains(1) && ab.contains(2) && ab.contains(3), "aboveAvg an 10,20,30");
        auto bl = filterutil::rowsToHideByNumber(col, NumOp::BelowAvg, 0);
        ok(bl.size() == 3 && bl.contains(3) && bl.contains(4) && bl.contains(5), "belowAvg an 30,40,50");
        ok(!ab.contains(0), "header luon giu (number filter)");
        // o khong phai so -> luon an
        QVector<QString> mix{"H", "5", "abc", "9"};
        auto mh = filterutil::rowsToHideByNumber(mix, NumOp::Gt, 0);
        ok(mh.contains(2) && !mh.contains(1) && !mh.contains(3), "o text bi an khi loc so");
    }

    // --- loc tuy chinh 2 dieu kien AND/OR (Custom AutoFilter, Spec 15) ---
    {
        using filterutil::FiltOp;
        // matchCond co ban
        ok(filterutil::matchCond("Hanoi", FiltOp::Contains, "ano"), "Contains ano");
        ok(filterutil::matchCond("Hanoi", FiltOp::BeginsWith, "Ha"), "BeginsWith Ha");
        ok(filterutil::matchCond("Hanoi", FiltOp::EndsWith, "oi"), "EndsWith oi");
        ok(filterutil::matchCond("30", FiltOp::Gt, "20"), "so 30>20");
        ok(filterutil::matchCond("apple", FiltOp::Lt, "banana"), "chuoi apple<banana");

        // header + so
        QVector<QString> col{"Diem", "10", "20", "30", "40", "50"};
        // >15 AND <45 -> giu 20,30,40 ; an 10,50
        auto a = filterutil::rowsToHideCustom(col, FiltOp::Gt, "15", true, true, FiltOp::Lt, "45");
        ok(a.size() == 2 && a.contains(1) && a.contains(5), "AND >15 & <45 an 10,50");
        // <15 OR >45 -> giu 10,50 ; an 20,30,40
        auto o = filterutil::rowsToHideCustom(col, FiltOp::Lt, "15", false, true, FiltOp::Gt, "45");
        ok(o.size() == 3 && o.contains(2) && o.contains(3) && o.contains(4), "OR <15 | >45 an 20,30,40");
        // chi 1 dieu kien: >=30 -> an 10,20
        auto s = filterutil::rowsToHideCustom(col, FiltOp::Ge, "30", true, false, FiltOp::Eq, "");
        ok(s.size() == 2 && s.contains(1) && s.contains(2), "1 dieu kien >=30 an 10,20");
        ok(!a.contains(0), "header luon giu (custom)");
    }

    // --- dan dac biet: chuyen vi ---
    {
        QVector<QVector<QString>> blk{{"a", "b", "c"}, {"1", "2", "3"}};
        auto t = pasteops::transpose(blk);
        ok(t.size() == 3 && t[0].size() == 2, "transpose 2x3 -> 3x2");
        ok(t[0][0] == "a" && t[0][1] == "1" && t[2][1] == "3", "transpose dung vi tri");
        ok(pasteops::transpose({}).isEmpty(), "transpose rong -> rong");
    }

    // --- dan dac biet: phep tinh (Operation) + bo qua o trong ---
    {
        using pasteops::Op;
        QVector<QVector<QString>> dest{{"5", "10"}};
        QVector<QVector<QString>> src{{"11", "3"}};
        auto add = pasteops::applyOperation(dest, src, Op::Add, false);
        ok(add[0][0] == "16" && add[0][1] == "13", "Operation Add: dest + src");
        auto sub = pasteops::applyOperation(dest, src, Op::Subtract, false);
        ok(sub[0][0] == "-6" && sub[0][1] == "7", "Operation Subtract");
        auto mul = pasteops::applyOperation(dest, src, Op::Multiply, false);
        ok(mul[0][0] == "55" && mul[0][1] == "30", "Operation Multiply");
        auto div = pasteops::applyOperation({{"10"}}, {{"4"}}, Op::Divide, false);
        ok(div[0][0] == "2.5", "Operation Divide");

        // Chia cho 0 -> giu gia tri dich.
        auto dz = pasteops::applyOperation({{"10"}}, {{"0"}}, Op::Divide, false);
        ok(dz[0][0] == "10", "Divide by zero giu dich");

        // None + skip blanks: o src trong giu dest, o khac lay src.
        QVector<QVector<QString>> src2{{"", "99"}};
        auto sk = pasteops::applyOperation(dest, src2, Op::None, true);
        ok(sk[0][0] == "5" && sk[0][1] == "99", "Skip Blanks: o trong giu dich");
        // Khong skip: o trong ghi de dest thanh rong.
        auto nsk = pasteops::applyOperation(dest, src2, Op::None, false);
        ok(nsk[0][0].isEmpty() && nsk[0][1] == "99", "Khong skip: o trong ghi de");

        // Nguon khong phai so -> dan nguyen van (Add).
        auto txt = pasteops::applyOperation({{"5"}}, {{"abc"}}, Op::Add, false);
        ok(txt[0][0] == "abc", "Nguon text -> dan nguyen");
        // Dich trong, Add -> coi dich = 0.
        auto de = pasteops::applyOperation({{""}}, {{"7"}}, Op::Add, false);
        ok(de[0][0] == "7", "Dich trong coi nhu 0");

        // applyConstant: ap phep tinh voi hang so
        ok(pasteops::applyConstant("10", Op::Add, 5) == "15", "applyConstant +5");
        ok(pasteops::applyConstant("10", Op::Multiply, 2) == "20", "applyConstant x2");
        ok(pasteops::applyConstant("10", Op::Divide, 0) == "10", "applyConstant chia 0 giu nguyen");
        ok(pasteops::applyConstant("abc", Op::Add, 5) == "abc", "applyConstant text giu nguyen");
        ok(pasteops::applyConstant("", Op::Add, 5) == "", "applyConstant o rong giu nguyen");
    }

    // --- thong ke vung chon ---
    {
        stats::Result r = stats::compute({"10", "20", "abc", "", "30"});
        ok(r.count == 4, "dem 4 o khac rong");
        ok(r.numCount == 3, "3 o so");
        ok(r.sum == 60.0, "tong 60");
        ok(r.avg == 20.0, "trung binh 20");
        ok(r.min == 10.0, "nho nhat 10");
        ok(r.max == 30.0, "lon nhat 30");
        stats::Result e = stats::compute({"x", "y"});
        ok(e.count == 2 && e.numCount == 0 && e.avg == 0, "khong co so -> avg 0");
        ok(e.min == 0 && e.max == 0, "khong co so -> min/max 0");
        stats::Result neg = stats::compute({"-5", "-1", "-9"});
        ok(neg.min == -9.0 && neg.max == -1.0, "so am: min -9 max -1");
        ok(r.median == 20.0, "trung vi le -> 20 (10,20,30)");
        stats::Result m4 = stats::compute({"1", "2", "3", "4"});
        ok(m4.median == 2.5, "trung vi chan -> trung binh 2 giua (2.5)");
        stats::Result m1 = stats::compute({"7"});
        ok(m1.median == 7.0, "1 so -> trung vi = chinh no");
        ok(e.median == 0, "khong co so -> trung vi 0");
        stats::Result mu = stats::compute({"5", "1", "3"}); // chua sap xep
        ok(mu.median == 3.0, "trung vi tu sap xep (1,3,5 -> 3)");
        // stdev mau (n-1): {2,4,4,4,5,5,7,9} -> 2.138...; {1,2,3,4,5} mean3 var=2.5 stdev~1.5811
        stats::Result sd = stats::compute({"1", "2", "3", "4", "5"});
        ok(std::abs(sd.stdev - 1.5811388300841898) < 1e-9, "stdev mau (1..5) ~1.5811");
        ok(m1.stdev == 0, "1 so -> stdev 0");
        ok(e.stdev == 0, "khong co so -> stdev 0");
    }

    // --- AutoSum: dem run so cuoi danh sach ---
    {
        ok(autosum::trailingNumericRun({"a", "1", "2", "3"}) == 3, "run 3 so cuoi");
        ok(autosum::trailingNumericRun({"1", "2", "x", "5"}) == 1, "dut o 'x' -> run 1");
        ok(autosum::trailingNumericRun({"a", "b"}) == 0, "khong co so -> 0");
        ok(autosum::trailingNumericRun({}) == 0, "rong -> 0");
        ok(autosum::trailingNumericRun({"10", "20"}) == 2, "ca hai so -> 2");
    }

    // --- kiem tra du lieu (data validation) ---
    {
        using A = validation::Allow; using O = validation::Op;
        ok(validation::check("5", A::WholeNumber, O::Between, 1, 10), "5 trong [1,10]");
        ok(!validation::check("11", A::WholeNumber, O::Between, 1, 10), "11 ngoai [1,10]");
        ok(!validation::check("5.5", A::WholeNumber, O::Between, 1, 10), "5.5 khong phai so nguyen");
        ok(validation::check("5.5", A::Decimal, O::Greater, 5, 0), "5.5 > 5 (decimal)");
        ok(!validation::check("abc", A::WholeNumber, O::Between, 1, 10), "chuoi -> khong hop le");
        ok(validation::check("", A::WholeNumber, O::Between, 1, 10), "rong -> hop le (ignore blank)");
        ok(validation::check("hello", A::TextLength, O::LessEqual, 10, 0), "do dai 5 <= 10");
        ok(!validation::check("hello world!!", A::TextLength, O::LessEqual, 10, 0), "do dai 13 > 10");

        // tich hop model: setData tu choi gia tri sai
        SpreadsheetModel mm;
        mm.resizeGrid(3, 3);
        validation::Rule vr; vr.top=0; vr.left=0; vr.bottom=2; vr.right=0;
        vr.allow = A::WholeNumber; vr.op = O::Between; vr.v1 = 1; vr.v2 = 100;
        mm.addValidationRule(vr);
        ok(mm.setData(mm.index(0,0), "50", Qt::EditRole) && disp(mm,0,0) == "50", "validation: nhan 50");
        ok(!mm.setData(mm.index(1,0), "999", Qt::EditRole), "validation: tu choi 999");
        ok(disp(mm,1,0) == "", "validation: o van rong sau khi tu choi");
    }

    // --- ghi chu o (note) ---
    {
        SpreadsheetModel mm;
        mm.resizeGrid(3, 3);
        mm.setNote(0, 0, "Ghi chu thu nghiem");
        ok(mm.note(0, 0) == "Ghi chu thu nghiem", "setNote/note round-trip");
        ok(mm.data(mm.index(0, 0), Qt::ToolTipRole).toString() == "Ghi chu thu nghiem", "ToolTipRole tra note");
        ok(mm.note(1, 1).isEmpty(), "o khong co note -> rong");
        mm.setNote(0, 0, "");
        ok(mm.note(0, 0).isEmpty(), "setNote rong -> xoa");
    }

    // --- vung dat ten (named range) ---
    {
        SpreadsheetModel mm;
        mm.resizeGrid(10, 5);
        mm.defineName("DoanhThu", MergeRange{1, 0, 5, 2});
        MergeRange out;
        ok(mm.lookupName("DoanhThu", out), "lookupName tim thay");
        ok(out.top == 1 && out.left == 0 && out.bottom == 5 && out.right == 2, "lookupName tra dung vung");
        ok(!mm.lookupName("KhongCo", out), "ten khong co -> false");
        ok(mm.definedNames().contains("DoanhThu"), "definedNames liet ke");
    }

    // --- kieu o dung san (cell styles) ---
    {
        auto good = cellstyles::style(QString::fromUtf8("T\xE1\xBB\x91t")); // "Tốt"
        ok(good.value("bg").toString() == "#C6EFCE", "kieu Tot -> bg xanh");
        ok(good.value("color").toString() == "#006100", "kieu Tot -> chu xanh dam");
        auto normal = cellstyles::style(QString::fromUtf8("B\xC3\xACnh th\xC6\xB0\xE1\xBB\x9Dng")); // "Bình thường"
        ok(normal.contains("bg") && !normal.value("bg").isValid(), "kieu Binh thuong -> bg null (xoa)");
        ok(cellstyles::names().size() >= 5, "co >=5 kieu");
        ok(cellstyles::style("KhongCo").isEmpty(), "kieu la -> rong");
    }

    // --- di toi dac biet (goto special) ---
    {
        QVector<QVector<QString>> grid = {{"10", "", "=A1+1"}, {"text", "20", ""}};
        using K = gotospecial::Kind;
        ok(gotospecial::find(grid, K::Blanks).size() == 2, "2 o trong");
        ok(gotospecial::find(grid, K::Formulas).size() == 1, "1 o cong thuc");
        ok(gotospecial::find(grid, K::Numbers).size() == 2, "2 o so (10,20)");
        ok(gotospecial::find(grid, K::Text).size() == 1, "1 o van ban");
        ok(gotospecial::find(grid, K::Constants).size() == 3, "3 o hang (10,text,20)");
        auto f = gotospecial::find(grid, K::Formulas);
        ok(f.first().first == 0 && f.first().second == 2, "o cong thuc tai (0,2)");
    }

    // --- current region + last cell (Spec 32) ---
    {
        // Khoi A1:C2 lien tuc; hang 3 trong; D4 la cum roi rac khac.
        QVector<QVector<QString>> grid = {
            {"a", "b", "c", ""},
            {"1", "2", "3", ""},
            {"",  "",  "",  ""},
            {"",  "",  "",  "z"},
        };
        auto reg = gotospecial::currentRegion(grid, 0, 0);
        ok(reg.top == 0 && reg.left == 0 && reg.bottom == 1 && reg.right == 2, "current region A1:C2");
        // bam giua khoi van ra dung vung
        auto reg2 = gotospecial::currentRegion(grid, 1, 1);
        ok(reg2.top == 0 && reg2.bottom == 1 && reg2.right == 2, "current region tu giua khoi");
        // o roi rac D4 -> chinh no
        auto reg3 = gotospecial::currentRegion(grid, 3, 3);
        ok(reg3.top == 3 && reg3.left == 3 && reg3.bottom == 3 && reg3.right == 3, "o roi rac -> chinh no");
        // last cell: hang lon nhat co data = 3 (z), cot lon nhat = 3 (z) -> (3,3)
        auto lc = gotospecial::lastCell(grid);
        ok(lc.first == 3 && lc.second == 3, "last cell (3,3)");
        ok(gotospecial::lastCell({{"", ""}, {"", ""}}).first == -1, "luoi rong -> last cell -1");
    }

    // --- row/col differences (Spec 32) ---
    {
        // Cot 0 la moc. Hang 0: 5,5,9 -> (0,2) khac. Hang 1: 7,7,7 -> khong khac.
        QVector<QVector<QString>> grid = {
            {"5", "5", "9"},
            {"7", "7", "7"},
            {"3", "8", "3"},
        };
        auto rd = gotospecial::rowDifferences(grid, 0, 0, 2, 2, 0);
        // hang 0: (0,2); hang 2: (2,1) khac moc "3"
        ok(rd.size() == 2 && rd.contains(qMakePair(0, 2)) && rd.contains(qMakePair(2, 1)), "row diff (0,2)+(2,1)");
        ok(!rd.contains(qMakePair(0, 0)), "o moc khong tinh");
        // col differences voi moc hang 0: cot 0 moc 5 -> (1,0)=7,(2,0)=3 khac; cot1 moc5 ->(1,1)=7,(2,1)=8; cot2 moc9->(1,2)=7,(2,2)=3
        auto cd = gotospecial::colDifferences(grid, 0, 0, 2, 2, 0);
        ok(cd.size() == 6, "col diff 6 o khac moc hang 0");
        ok(!cd.contains(qMakePair(0, 1)), "o moc hang khong tinh");
    }

    // --- precedents / dependents (Spec 32) ---
    {
        SpreadsheetModel mm;
        mm.resizeGrid(6, 3);
        put(mm, 0, 0, "10");        // A1
        put(mm, 1, 0, "20");        // A2
        put(mm, 2, 0, "=A1+A2");    // A3 phu thuoc A1,A2
        put(mm, 3, 0, "=A3*2");     // A4 phu thuoc A3
        // precedents A3 truc tiep = A1,A2
        auto p = mm.precedents(2, 0, false);
        ok(p.size() == 2 && p.contains(qMakePair(0, 0)) && p.contains(qMakePair(1, 0)), "precedents A3 = A1,A2");
        // precedents A4 all levels = A3, A1, A2
        auto pa = mm.precedents(3, 0, true);
        ok(pa.size() == 3 && pa.contains(qMakePair(2, 0)) && pa.contains(qMakePair(0, 0)), "precedents A4 all = A3,A1,A2");
        // precedents A4 truc tiep = chi A3
        auto pd = mm.precedents(3, 0, false);
        ok(pd.size() == 1 && pd.contains(qMakePair(2, 0)), "precedents A4 direct = A3");
        // dependents A1 truc tiep = A3
        auto d1 = mm.dependents(0, 0, false);
        ok(d1.size() == 1 && d1.contains(qMakePair(2, 0)), "dependents A1 = A3");
        // dependents A1 all levels = A3, A4
        auto d1a = mm.dependents(0, 0, true);
        ok(d1a.size() == 2 && d1a.contains(qMakePair(2, 0)) && d1a.contains(qMakePair(3, 0)), "dependents A1 all = A3,A4");
        // o khong lien quan -> rong
        ok(mm.precedents(0, 2, true).isEmpty(), "o trong khong co precedents");
    }

    // --- named range manager: define/remove + rangeRef (Spec 31) ---
    {
        SpreadsheetModel mm;
        mm.resizeGrid(8, 5);
        mm.defineName("DoanhThu", MergeRange{0, 0, 4, 2}); // A1:C5
        mm.defineName("Mot", MergeRange{2, 1, 2, 1});      // B3 (1 o)
        MergeRange out;
        ok(mm.lookupName("DoanhThu", out), "lookup DoanhThu ok");
        ok(mm.definedNames().size() == 2, "co 2 ten");
        // rangeRef
        ok(SpreadsheetModel::rangeRef(MergeRange{0, 0, 4, 2}) == "A1:C5", "rangeRef vung A1:C5");
        ok(SpreadsheetModel::rangeRef(MergeRange{2, 1, 2, 1}) == "B3", "rangeRef 1 o -> B3");
        // remove
        ok(mm.removeName("Mot"), "remove Mot -> true");
        ok(!mm.lookupName("Mot", out), "Mot da xoa");
        ok(mm.definedNames().size() == 1, "con 1 ten");
        ok(!mm.removeName("KhongCo"), "remove ten khong ton tai -> false");
    }

    // --- Mảng động / spill (Spec 12) ---
    {
        SpreadsheetModel sm;
        sm.resizeGrid(10, 5);
        put(sm, 0, 0, "=SEQUENCE(3)");   // A1 -> tràn dọc A1:A3 = 1,2,3
        ok(disp(sm, 0, 0) == "1", "spill anchor A1=1");
        ok(disp(sm, 1, 0) == "2", "spill A2=2");
        ok(disp(sm, 2, 0) == "3", "spill A3=3");
        ok(disp(sm, 3, 0) == "", "duoi vung spill trong");
        // Ô tràn read-only (không có cờ Editable).
        ok(!(sm.flags(sm.index(1, 0)) & Qt::ItemIsEditable), "A2 read-only");
        ok(sm.flags(sm.index(0, 0)) & Qt::ItemIsEditable, "anchor A1 sua duoc");
        // Biên vùng spill.
        int t, l, b, rr;
        ok(sm.spillRangeAt(1, 0, t, l, b, rr) && t == 0 && l == 0 && b == 2 && rr == 0, "spillRangeAt A2 -> A1:A3");
        ok(!sm.spillRangeAt(5, 0, t, l, b, rr), "o ngoai spill -> false");
        // Bitmask cạnh: A1 có cạnh trên+trái+phải (1|2|8=11); A3 có trái+dưới+phải (2|4|8=14).
        ok(sm.data(sm.index(0, 0), SpreadsheetModel::SpillEdgesRole).toInt() == (1|2|8), "edges A1");
        ok(sm.data(sm.index(2, 0), SpreadsheetModel::SpillEdgesRole).toInt() == (2|4|8), "edges A3");
        // Không ghi đè được ô tràn.
        ok(!sm.setData(sm.index(1, 0), QStringLiteral("x"), Qt::EditRole), "khong ghi o tran");

        // Chặn spill -> #SPILL!: đặt vật cản ở C2 rồi cho C1 spill xuống.
        put(sm, 1, 2, "chan");           // C2 có dữ liệu
        put(sm, 0, 2, "=SEQUENCE(3)");   // C1 muốn tràn C1:C3 -> bị chặn
        ok(disp(sm, 0, 2) == "#SPILL!", "bi chan -> #SPILL!");
        ok(disp(sm, 1, 2) == "chan", "o chan giu nguyen");
        // Bỏ vật cản -> spill hồi phục.
        put(sm, 1, 2, "");
        ok(disp(sm, 0, 2) == "1" && disp(sm, 2, 2) == "3", "bo chan -> spill lai");
        // 2 chiều: SEQUENCE(2,3) -> 2x3.
        put(sm, 5, 0, "=SEQUENCE(2,3)");
        ok(disp(sm, 5, 0) == "1" && disp(sm, 5, 2) == "3" && disp(sm, 6, 0) == "4" && disp(sm, 6, 2) == "6", "spill 2x3");
        // Sửa anchor mất công thức -> vùng tràn biến mất.
        put(sm, 0, 0, "99");
        ok(disp(sm, 0, 0) == "99" && disp(sm, 1, 0) == "" && disp(sm, 2, 0) == "", "xoa anchor -> het spill");
    }

    // --- Spill: ca bien (undo, merge, chen hang) ---
    {
        SpreadsheetModel sm;
        sm.resizeGrid(10, 5);
        // Undo: go =SEQUENCE(3) roi undo -> het spill, o tran sua lai duoc.
        put(sm, 0, 0, "=SEQUENCE(3)");
        ok(disp(sm, 1, 0) == "2", "spill truoc undo");
        sm.undo();
        ok(disp(sm, 0, 0) == "" && disp(sm, 1, 0) == "" && disp(sm, 2, 0) == "", "undo -> het spill");
        ok(sm.flags(sm.index(1, 0)) & Qt::ItemIsEditable, "o cu tran sua lai duoc sau undo");
        sm.redo();
        ok(disp(sm, 1, 0) == "2", "redo -> spill lai");

        // Spill de len o gop -> #SPILL!.
        SpreadsheetModel sg;
        sg.resizeGrid(10, 5);
        sg.toggleMergeRanges({ MergeRange{1, 0, 1, 1} }); // gop B2:... thuc ra A2:B2 (hang1, cot0-1)
        put(sg, 0, 0, "=SEQUENCE(3)");                    // muon tran A1:A3, dung o gop tai hang1
        ok(disp(sg, 0, 0) == "#SPILL!", "tran vao o gop -> #SPILL!");
        // Bo gop -> tran lai.
        sg.toggleMergeRanges({ MergeRange{1, 0, 1, 1} });
        ok(disp(sg, 0, 0) == "1" && disp(sg, 1, 0) == "2", "bo gop -> spill lai");

        // Chen hang phia tren anchor -> spill dich theo, van dung.
        SpreadsheetModel si;
        si.resizeGrid(10, 5);
        put(si, 2, 0, "=SEQUENCE(3)"); // A3 -> A3:A5
        ok(disp(si, 2, 0) == "1" && disp(si, 4, 0) == "3", "spill A3:A5");
        si.insertRows(0, 1);            // chen 1 hang dau -> anchor xuong A4
        ok(disp(si, 3, 0) == "1" && disp(si, 5, 0) == "3", "sau chen hang: spill A4:A6");
    }

    std::printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
