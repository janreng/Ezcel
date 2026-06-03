// Tiện ích mức thu phóng (Spec 11 — phần phải thanh trạng thái). Logic thuần, test được.
#pragma once
#include <QList>

namespace zoom {

constexpr int kMin = 10;   // 10%
constexpr int kMax = 400;  // 400%
constexpr int kStep = 10;  // mỗi nấc 10%

// Giới hạn mức thu phóng vào [kMin, kMax].
int clamp(int pct);

// Tăng/giảm theo nấc rồi giới hạn.
int stepped(int pct, int delta);

// Các mức dựng sẵn cho hộp thoại Thu phóng.
QList<int> presets();

} // namespace zoom
