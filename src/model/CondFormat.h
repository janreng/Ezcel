#pragma once
#include <QString>
#include <QVariant>

// Định dạng có điều kiện — logic so khớp THUẦN (test headless được).
namespace cond {

enum class Op { GreaterThan, LessThan, Equal, NotEqual, Between, Contains };

// Một quy tắc: áp cho vùng [top,left]..[bottom,right]; nếu ô thỏa điều kiện thì
// tô nền bg (+ màu chữ color nếu có).
struct Rule {
    int top, left, bottom, right;
    Op op;
    double v1 = 0, v2 = 0;   // ngưỡng số (Between dùng cả v2)
    QString text;            // chuỗi cho Contains
    QString bg, color;       // màu áp khi khớp
    bool contains(int r, int c) const {
        return top <= r && r <= bottom && left <= c && c <= right;
    }
};

// Giá trị ô có thỏa điều kiện không. Contains so chuỗi (không phân biệt hoa/thường);
// các phép còn lại so số (ô không phải số -> không khớp).
bool match(const QVariant &value, Op op, double v1, double v2, const QString &text);

// Thanh dữ liệu (Data Bar): vẽ thanh ngang tỉ lệ theo giá trị ô trong vùng. Áp cho
// vùng [top,left]..[bottom,right], màu thanh `color`.
struct DataBar {
    int top, left, bottom, right;
    QString color;
    bool contains(int r, int c) const {
        return top <= r && r <= bottom && left <= c && c <= right;
    }
};

// Tỉ lệ lấp đầy thanh [0..1] cho `value` trong khoảng [mn,mx]; kẹp ngoài biên;
// mx<=mn -> 1 (mọi giá trị lấp đầy).
double dataBarFraction(double value, double mn, double mx);

} // namespace cond
