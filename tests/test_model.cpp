// Test headless cho SpreadsheetModel (offscreen). CHI in ASCII.
#include "model/SpreadsheetModel.h"
#include "model/TextSearch.h"
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

    std::printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
