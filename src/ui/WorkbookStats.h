// Thống kê bảng tính (Spec 57.1) — đếm ô/dữ liệu/công thức/từ. Logic thuần, test được.
#pragma once
#include <QString>
#include <vector>

namespace wbstats {

// Kết quả thống kê cho một tập ô (một trang hoặc cả workbook).
struct Result {
    int cellsWithData = 0; // ô khác rỗng
    int formulas = 0;      // ô bắt đầu bằng '='
    int numbers = 0;       // ô là số (không phải công thức)
    int words = 0;         // tổng số từ trong các ô văn bản (không phải công thức/số)
};

// Phân tích danh sách nội dung ô (chuỗi). Ô rỗng bị bỏ qua.
Result analyze(const std::vector<QString> &cellTexts);

// Cộng dồn (để gộp nhiều trang thành thống kê workbook).
Result &add(Result &dst, const Result &src);

} // namespace wbstats
