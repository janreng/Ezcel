#pragma once

// Hỗ trợ nút lọc ▼ trên ô tiêu đề bảng (Spec 16 / AutoFilter). Hình học thuần để test.
namespace tablefilter {

// Bề rộng vùng bấm của nút lọc ở mép phải ô tiêu đề (pixel).
inline int arrowZone() { return 16; }

// Click (px,py) có rơi vào vùng nút lọc ở mép phải ô [x,y,w,h] không?
inline bool arrowHit(int x, int y, int w, int h, int px, int py, int zone = 16)
{
    if (py < y || py > y + h) return false;
    return px >= x + w - zone && px <= x + w;
}

} // namespace tablefilter
