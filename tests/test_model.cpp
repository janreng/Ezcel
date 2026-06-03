// Test headless cho SpreadsheetModel (offscreen). CHI in ASCII.
#include "model/SpreadsheetModel.h"
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

    std::printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
