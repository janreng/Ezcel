#pragma once
#include <QString>

// Kiểm tra dữ liệu nhập (Data Validation, port Spec 25). Logic thuần, test được.
// Đợt này: Whole number / Decimal / Text length với phép so sánh. List/Date để sau.
namespace validation {

enum class Allow { Any, WholeNumber, Decimal, TextLength };
enum class Op { Between, NotBetween, Equal, NotEqual, Greater, Less, GreaterEqual, LessEqual };

struct Rule {
    int top, left, bottom, right;
    Allow allow = Allow::Any;
    Op op = Op::Between;
    double v1 = 0, v2 = 0;
    bool contains(int r, int c) const {
        return top <= r && r <= bottom && left <= c && c <= right;
    }
};

// Giá trị có hợp lệ theo điều kiện không. Ô rỗng -> luôn hợp lệ (ignore blank).
bool check(const QString &value, Allow allow, Op op, double v1, double v2);

} // namespace validation
