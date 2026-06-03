#include "view/CopyVisible.h"
#include <cstdio>

static int g_pass = 0, g_fail = 0;
static void ok(bool c, const char *n) { if (c) ++g_pass; else { ++g_fail; std::printf("FAIL %s\n", n); } }

int main()
{
    QVector<QVector<QString>> block = {
        {"a", "1"},
        {"b", "2"},
        {"c", "3"},
    };
    // Không ẩn -> đủ 3 hàng.
    ok(copyutil::toTsvSkipHidden(block, {}) == QStringLiteral("a\t1\nb\t2\nc\t3"), "khong an: du 3 hang");
    // Ẩn hàng giữa (offset 1) -> chỉ a và c, không có dòng trống.
    ok(copyutil::toTsvSkipHidden(block, {1}) == QStringLiteral("a\t1\nc\t3"), "an hang giua");
    // Ẩn hàng đầu (offset 0) -> bắt đầu từ b.
    ok(copyutil::toTsvSkipHidden(block, {0}) == QStringLiteral("b\t2\nc\t3"), "an hang dau");
    // Ẩn hàng cuối -> không có '\n' thừa ở cuối.
    ok(copyutil::toTsvSkipHidden(block, {2}) == QStringLiteral("a\t1\nb\t2"), "an hang cuoi khong newline thua");
    // Ẩn tất cả -> rỗng.
    ok(copyutil::toTsvSkipHidden(block, {0, 1, 2}).isEmpty(), "an het -> rong");

    std::printf("%d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
