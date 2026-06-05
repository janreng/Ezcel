// Test thuần cho Kiểm tra trợ năng (Accessibility, Spec 41).
#include "model/Accessibility.h"
#include <cstdio>

static int g_pass = 0, g_fail = 0;
static void ok(bool c, const char *n) { if (c) ++g_pass; else { ++g_fail; std::printf("FAIL %s\n", n); } }

static QVector<QString> row(std::initializer_list<const char *> xs) {
    QVector<QString> r;
    for (auto x : xs) r << QString::fromUtf8(x);
    return r;
}

int main() {
    // Bảng tốt: có tiêu đề đủ, không hàng/cột trống xen giữa.
    QVector<QVector<QString>> good = { row({"Tên", "Tuổi"}), row({"An", "20"}), row({"Bình", "30"}) };
    ok(a11y::check(good).isEmpty(), "bang tot -> khong van de");

    // Thiếu tiêu đề cột 2.
    QVector<QVector<QString>> noHdr = { row({"Tên", ""}), row({"An", "20"}) };
    ok(a11y::check(noHdr).size() == 1, "thieu tieu de -> 1 van de");

    // Hàng trống xen giữa.
    QVector<QVector<QString>> blankRow = { row({"Tên", "Tuổi"}), row({"", ""}), row({"An", "20"}) };
    {
        const QStringList r = a11y::check(blankRow);
        bool hasRowIssue = false;
        for (const QString &s : r) if (s.contains(QStringLiteral("Hàng"))) hasRowIssue = true;
        ok(hasRowIssue, "hang trong xen giua -> bao");
    }

    // Cột trống xen giữa.
    QVector<QVector<QString>> blankCol = { row({"Tên", "", "Tuổi"}), row({"An", "", "20"}) };
    {
        const QStringList r = a11y::check(blankCol);
        bool hasColIssue = false;
        for (const QString &s : r) if (s.contains(QStringLiteral("Cột")) && s.contains(QStringLiteral("trống"))) hasColIssue = true;
        ok(hasColIssue, "cot trong xen giua -> bao");
    }

    // Bảng rỗng -> không vấn đề.
    QVector<QVector<QString>> empty = { row({"", ""}), row({"", ""}) };
    ok(a11y::check(empty).isEmpty(), "bang rong -> khong van de");

    std::printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
