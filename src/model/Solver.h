#pragma once
#include <functional>
#include <cmath>

// Solver — Bộ giải tối ưu (Spec 50), bản 1: tối ưu hàm 1 biến trên một khoảng.
// Logic thuần để kiểm thử (không phụ thuộc model/Qt). UI nối f(x) = giá trị ô mục tiêu
// khi đặt ô biến = x.
namespace solver {

enum class Goal { Max, Min, Target };

// Điểm số cần CỰC ĐẠI hoá theo mục tiêu: Max -> f; Min -> -f; Target -> -|f-target|.
inline double score(double fx, Goal goal, double target)
{
    switch (goal) {
    case Goal::Max:    return fx;
    case Goal::Min:    return -fx;
    case Goal::Target: return -std::fabs(fx - target);
    }
    return fx;
}

// Tối ưu f trên [lo,hi]: quét thô `coarse` điểm tìm điểm tốt nhất, rồi tinh chỉnh cục bộ
// bằng tìm kiếm tam phân (ternary) quanh điểm đó. Trả x tốt nhất (đã kẹp trong [lo,hi]).
inline double optimize1D(const std::function<double(double)> &f, double lo, double hi,
                         Goal goal, double target = 0.0, int coarse = 200, int refine = 100)
{
    if (hi < lo) std::swap(lo, hi);
    if (hi - lo < 1e-12) return lo;

    // 1) Quét thô.
    double bestX = lo, bestS = score(f(lo), goal, target);
    const int n = coarse < 2 ? 2 : coarse;
    for (int i = 1; i <= n; ++i) {
        const double x = lo + (hi - lo) * i / n;
        const double s = score(f(x), goal, target);
        if (s > bestS) { bestS = s; bestX = x; }
    }

    // 2) Tinh chỉnh tam phân quanh điểm tốt nhất (1 bước lưới mỗi bên).
    const double step = (hi - lo) / n;
    double a = std::max(lo, bestX - step), b = std::min(hi, bestX + step);
    for (int i = 0; i < refine && b - a > 1e-12; ++i) {
        const double m1 = a + (b - a) / 3.0, m2 = b - (b - a) / 3.0;
        if (score(f(m1), goal, target) < score(f(m2), goal, target)) a = m1;
        else b = m2;
    }
    const double x = (a + b) / 2.0;
    return score(f(x), goal, target) >= bestS ? x : bestX;
}

} // namespace solver
