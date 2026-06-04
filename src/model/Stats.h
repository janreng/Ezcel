#pragma once
#include <QString>
#include <QVector>

// Thống kê nhanh vùng chọn (như thanh trạng thái bảng tính). Logic thuần, test được.
namespace stats {

struct Result {
    int count = 0;      // số ô khác rỗng
    int numCount = 0;   // số ô là số
    double sum = 0;     // tổng các ô số
    double avg = 0;     // trung bình các ô số (0 nếu không có số)
    double min = 0;     // giá trị nhỏ nhất (0 nếu không có số)
    double max = 0;     // giá trị lớn nhất (0 nếu không có số)
    double median = 0;  // trung vị các ô số (0 nếu không có số)
    double stdev = 0;   // độ lệch chuẩn mẫu (n-1); 0 nếu ít hơn 2 số
};

// Tính từ danh sách giá trị hiển thị của các ô đang chọn.
Result compute(const QVector<QString> &values);

} // namespace stats
