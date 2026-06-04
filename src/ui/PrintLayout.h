#pragma once

// Tiện ích thuần cho việc in (Spec 24) — tách khỏi Qt GUI để kiểm thử được.
namespace printlayout {

// Tỉ lệ thu để khít vùng nội dung (srcW×srcH) vào trang in (pageW×pageH),
// GIỮ tỉ lệ khung và KHÔNG phóng to (clamp ≤ 1.0). Trả 1.0 nếu kích thước không hợp lệ.
inline double fitScale(double srcW, double srcH, double pageW, double pageH)
{
    if (srcW <= 0.0 || srcH <= 0.0 || pageW <= 0.0 || pageH <= 0.0) return 1.0;
    double s = pageW / srcW;
    double sy = pageH / srcH;
    if (sy < s) s = sy;
    if (s > 1.0) s = 1.0; // không phóng to nội dung nhỏ hơn trang
    return s;
}

} // namespace printlayout
