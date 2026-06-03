// Test headless cho span ô gộp (viewutil::applyMergeSpans). Chi in ASCII.
#include "view/MergeSpans.h"
#include "model/SpreadsheetModel.h"
#include <QApplication>
#include <QTableView>
#include <cstdio>

static int g_pass = 0, g_fail = 0;
static void ok(bool cond, const char *name) {
    if (cond) ++g_pass; else { ++g_fail; std::printf("FAIL %s\n", name); }
}

int main(int argc, char **argv) {
    qputenv("QT_QPA_PLATFORM", "offscreen");
    QApplication app(argc, argv);

    SpreadsheetModel m;
    m.resizeGrid(10, 6);
    QTableView view;
    view.setModel(&m);

    // Gop A1:C2 (3 cot x 2 hang) + mot vung 1 cot nhieu hang E1:E3.
    m.mergeCells(0, 0, 1, 2);
    m.mergeCells(0, 4, 2, 4);
    viewutil::applyMergeSpans(&view, m.merges());

    ok(view.rowSpan(0, 0) == 2 && view.columnSpan(0, 0) == 3, "span A1:C2 = 2x3");
    ok(view.rowSpan(0, 4) == 3 && view.columnSpan(0, 4) == 1, "span E1:E3 = 3x1");
    // O ngoai vung gop khong co span.
    ok(view.rowSpan(5, 5) == 1 && view.columnSpan(5, 5) == 1, "o thuong khong span");

    // Bo gop A1:C2 -> span do bi xoa, span E con.
    m.unmergeCells(0, 0, 1, 2);
    viewutil::applyMergeSpans(&view, m.merges());
    ok(view.rowSpan(0, 0) == 1 && view.columnSpan(0, 0) == 1, "unmerge -> A1 het span");
    ok(view.rowSpan(0, 4) == 3, "vung E1:E3 van con span");

    // Bo het.
    m.unmergeCells(0, 4, 2, 4);
    viewutil::applyMergeSpans(&view, m.merges());
    ok(view.rowSpan(0, 4) == 1, "unmerge het -> E1 het span");

    std::printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
