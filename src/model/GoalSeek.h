// Dò mục tiêu (Goal Seek, Spec 28): tìm x sao cho f(x) = target. Logic thuần, test được.
#pragma once
#include <functional>

namespace goalseek {

struct Result {
    double x = 0.0;
    bool ok = false; // true nếu hội tụ trong sai số
};

// Giải f(x) = target bằng phương pháp cát tuyến (secant), khởi đầu từ `guess`.
// `f` là hàm 1 biến (vd giá trị ô công thức theo ô nhập). Trả ok=false nếu không hội tụ
// (lặp quá maxIter, đạo hàm 0, hoặc kết quả vô hạn).
Result solve(const std::function<double(double)> &f, double target, double guess,
             int maxIter = 100, double tol = 1e-7);

} // namespace goalseek
