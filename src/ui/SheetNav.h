// Điều hướng trang tính (Spec 10 — Navigate). Logic thuần, test được.
#pragma once

namespace sheetnav {

// Tính chỉ số trang kế tiếp khi nhấn Ctrl+PageUp/Down.
//   cur   : chỉ số hiện tại (0-based)
//   count : tổng số trang
//   delta : +1 (trang sau) / -1 (trang trước)
// Quay vòng: quá cuối -> về đầu, trước đầu -> về cuối. Trả về cur nếu count<=0.
int wrapIndex(int cur, int count, int delta);

} // namespace sheetnav
