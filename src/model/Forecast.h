// Dự báo theo xu hướng tuyến tính + trung bình trượt (Forecast, Spec 27). Logic thuần, test được.
#pragma once
#include <QVector>
#include <QPair>

namespace forecast {

// Đường hồi quy bình phương tối thiểu y = slope*x + intercept.
struct Line { double slope = 0.0; double intercept = 0.0; bool ok = false; };

// Khớp đường thẳng từ x[],y[] cùng độ dài (>=2, mẫu số khác 0). ok=false nếu không đủ.
Line linearFit(const QVector<double> &x, const QVector<double> &y);

// Dự báo `periods` kỳ tiếp theo bằng đường hồi quy. x tương lai nối tiếp theo BƯỚC ĐỀU
// trung bình của x lịch sử (mặc định +1 nếu chỉ 1 điểm). Trả danh sách (x_tương_lai, y_dự_báo).
QVector<QPair<double, double>> linearForecast(const QVector<double> &x, const QVector<double> &y, int periods);

// Trung bình trượt cửa sổ `window`: phần tử i = TB của y[i-window+1..i] khi i>=window-1,
// các vị trí trước đó trả NaN (chưa đủ cửa sổ). window<=0 -> rỗng.
QVector<double> movingAverage(const QVector<double> &y, int window);

} // namespace forecast
