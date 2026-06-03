// Flash Fill (Spec 05) — suy luật tách/ghép chuỗi từ ví dụ. Logic thuần, test được.
#pragma once
#include <QString>
#include <vector>
#include <optional>

namespace flashfill {

// Cho cột nguồn `sources` và `examples` (kết quả mong muốn cho `examples.size()` hàng đầu),
// suy ra quy luật đơn giản (tách theo dấu phân cách lấy token thứ N, hoặc cắt chuỗi theo vị trí)
// rồi trả về kết quả cho TẤT CẢ hàng. Trả nullopt nếu không nhận ra mẫu.
std::optional<std::vector<QString>> infer(const std::vector<QString> &sources,
                                          const std::vector<QString> &examples);

} // namespace flashfill
