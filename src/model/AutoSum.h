#pragma once
#include <QString>
#include <QVector>

// AutoSum — logic thuần (test headless được).
namespace autosum {

// Số ô SỐ liên tục tính từ CUỐI danh sách (vd các ô phía trên ô hiện tại,
// phần tử cuối = ô ngay trên). Dùng để xác định vùng =SUM().
int trailingNumericRun(const QVector<QString> &cells);

} // namespace autosum
