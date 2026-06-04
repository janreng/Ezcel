// Chèn/xóa Ô với dịch chuyển một phần (Spec 09): Shift right/down/left/up.
// Logic thuần, test được — chỉ tính "kế hoạch" ánh xạ ô đích <- ô nguồn, không
// đụng tới model. Đọc từ trạng thái CŨ rồi ghi đích nên không sợ ghi đè lẫn nhau.
#pragma once
#include <QVector>

namespace cellshift {

// Hướng dịch chuyển khi chèn (Right/Down tạo khoảng trống) hoặc xóa (Left/Up dồn lại).
enum class Dir { Right, Down, Left, Up };

// Một ô đích lấy nội dung từ ô nguồn (srcR<0 nghĩa là để TRỐNG).
struct Move { int dstR, dstC, srcR, srcC; };

// Dựng kế hoạch cho lưới rows×cols, vùng chọn [top,left,bottom,right] (đã kẹp trong lưới),
// theo hướng dir. Trả danh sách MỌI ô đích trong vùng ảnh hưởng kèm ô nguồn (hoặc trống).
// - Right/Left: ảnh hưởng các HÀNG [top,bottom], mọi cột từ left tới hết.
// - Down/Up:    ảnh hưởng các CỘT [left,right], mọi hàng từ top tới hết.
// Ô bị đẩy ra ngoài mép lưới sẽ mất (giống bảng tính). Vùng [top,left,bottom,right]
// không hợp lệ -> trả danh sách rỗng.
QVector<Move> plan(int rows, int cols, int top, int left, int bottom, int right, Dir dir);

} // namespace cellshift
