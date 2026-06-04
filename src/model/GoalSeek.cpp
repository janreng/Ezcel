#include "model/GoalSeek.h"
#include <cmath>

namespace goalseek {

Result solve(const std::function<double(double)> &f, double target, double guess,
             int maxIter, double tol)
{
    Result r;
    auto g = [&](double x) { return f(x) - target; }; // cần tìm nghiệm g(x)=0

    double x0 = guess;
    double x1 = (guess == 0.0) ? 1.0 : guess * 1.0001 + 1e-6; // điểm thứ 2 lệch nhẹ
    double g0 = g(x0), g1 = g(x1);
    if (std::isfinite(g0) && std::fabs(g0) <= tol) { r.x = x0; r.ok = true; return r; }

    for (int i = 0; i < maxIter; ++i) {
        if (!std::isfinite(g1)) return r;            // hàm phân kỳ
        if (std::fabs(g1) <= tol) { r.x = x1; r.ok = true; return r; }
        const double denom = g1 - g0;
        if (denom == 0.0) return r;                  // đạo hàm xấp xỉ 0 -> không tiến được
        const double x2 = x1 - g1 * (x1 - x0) / denom;
        if (!std::isfinite(x2)) return r;
        x0 = x1; g0 = g1;
        x1 = x2; g1 = g(x1);
    }
    // Hết vòng lặp: chấp nhận nếu đủ gần.
    if (std::isfinite(g1) && std::fabs(g1) <= tol * 100) { r.x = x1; r.ok = true; }
    return r;
}

} // namespace goalseek
