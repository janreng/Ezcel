// Tự điền chuỗi lịch/quý (Spec 05) — Mon->Tue, Jan->Feb, Q1->Q2. Logic thuần, test được.
#pragma once
#include <QString>
#include <optional>

namespace seriesfill {

// Nếu seed thuộc một danh sách xoay vòng đã biết (thứ/tháng/quý), trả về phần tử
// cách seed `pos` bước (xoay vòng), giữ nguyên kiểu hoa/thường của seed.
// Trả nullopt nếu seed không khớp danh sách nào.
std::optional<QString> next(const QString &seed, int pos);

} // namespace seriesfill
