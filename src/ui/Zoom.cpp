// Tiện ích mức thu phóng (Spec 11).
#include "ui/Zoom.h"
#include <algorithm>

namespace zoom {

int clamp(int pct) { return std::clamp(pct, kMin, kMax); }

int stepped(int pct, int delta) { return clamp(pct + delta); }

QList<int> presets() { return {25, 50, 75, 100, 150, 200, 400}; }

} // namespace zoom
