#pragma once
#include <QString>
#include <QVector>

// Thao tác dán đặc biệt — logic thuần (test headless được).
namespace pasteops {

// Chuyển vị khối (hàng <-> cột). Khối lệch cột được chuẩn hóa theo hàng dài nhất.
QVector<QVector<QString>> transpose(const QVector<QVector<QString>> &block);

} // namespace pasteops
