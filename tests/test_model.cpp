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

    std::printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
