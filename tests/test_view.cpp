// Test headless cho span ô gộp (viewutil::applyMergeSpans). Chi in ASCII.
#include "view/MergeSpans.h"
#include "view/Visibility.h"
#include "ui/RibbonBar.h"
#include "model/SpreadsheetModel.h"
#include <QApplication>
#include <QTableView>
#include <QToolButton>
#include <QMenu>
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

    // --- an/hien hang-cot ---
    {
        SpreadsheetModel mm;
        mm.resizeGrid(8, 6);
        QTableView v2;
        v2.setModel(&mm);
        viewutil::hideRows(&v2, 2, 3);
        ok(v2.isRowHidden(2) && v2.isRowHidden(3) && !v2.isRowHidden(1), "an hang 2-3");
        viewutil::hideCols(&v2, 1, 1);
        ok(v2.isColumnHidden(1) && !v2.isColumnHidden(0), "an cot 1");
        viewutil::unhideRange(&v2, 0, 0, 5, 5);
        ok(!v2.isRowHidden(2) && !v2.isRowHidden(3) && !v2.isColumnHidden(1), "hien lai het");
    }

    // --- dải lệnh Ribbon: dựng tab/nhóm/nút đúng cấu trúc ---
    {
        RibbonBar rb;
        rb.beginTab(QStringLiteral("Trang đầu"));
        rb.beginGroup(QStringLiteral("Phông"));
        int clicked = 0;
        rb.addButton(QStringLiteral("bold"), QStringLiteral("Đậm"), [&clicked] { ++clicked; });
        rb.beginGroup(QStringLiteral("Căn lề"));
        rb.addButton(QString(), QStringLiteral("Trái"), nullptr); // nút chỉ-chữ (không icon)
        rb.beginTab(QStringLiteral("Dữ liệu"));
        rb.beginGroup(QStringLiteral("Sắp xếp & Lọc"));
        rb.addButton(QString(), QStringLiteral("Lọc"), nullptr);
        rb.finish();

        ok(rb.count() == 2, "ribbon co 2 tab");
        ok(rb.tabTitles() == QStringList({QStringLiteral("Trang đầu"), QStringLiteral("Dữ liệu")}),
           "nhan tab dung thu tu");
        ok(rb.groupTitles(0) == QStringList({QStringLiteral("Phông"), QStringLiteral("Căn lề")}),
           "tab 0 co 2 nhom");
        ok(rb.groupTitles(1) == QStringList({QStringLiteral("Sắp xếp & Lọc")}),
           "tab 1 co 1 nhom");
        // Nút trả về có thật và bấm chạy callback.
        QToolButton *b = rb.addButton(QStringLiteral("copy"), QStringLiteral("Chép"), [&clicked] { ++clicked; });
        ok(b != nullptr, "addButton tra ve nut");
        if (b) b->click();
        ok(clicked == 1, "click nut chay callback");

        // Nút thả menu giữ đúng QMenu được gắn (dropdown kiểu Excel).
        QMenu menu;
        menu.addAction(QStringLiteral("Mục 1"));
        QToolButton *mb = rb.addMenuButton(QString(), QStringLiteral("Tệp"), &menu);
        ok(mb != nullptr && mb->menu() == &menu, "addMenuButton gan dung menu");
        ok(mb && mb->popupMode() == QToolButton::InstantPopup, "menu button instant popup");
    }

    std::printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
