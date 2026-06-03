#pragma once
#include "model/SpreadsheetModel.h" // MergeRange
#include <QVector>

class QTableView;

// Đồng bộ span của QTableView với danh sách ô gộp của model.
// Tách riêng (thuần view) để test headless được.
namespace viewutil {

// Xóa mọi span cũ rồi đặt lại span cho từng vùng gộp.
void applyMergeSpans(QTableView *view, const QVector<MergeRange> &merges);

} // namespace viewutil
