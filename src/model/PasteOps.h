#pragma once
#include <QString>
#include <QVector>

// Thao tác dán đặc biệt — logic thuần (test headless được).
namespace pasteops {

// Chuyển vị khối (hàng <-> cột). Khối lệch cột được chuẩn hóa theo hàng dài nhất.
QVector<QVector<QString>> transpose(const QVector<QVector<QString>> &block);

// Phép tính khi dán đặc biệt (Operation, Spec 13 §13.3).
enum class Op { None, Add, Subtract, Multiply, Divide };

// Gộp khối nguồn `src` vào khối đích hiện tại `dest` theo phép tính `op`.
// dest = giá trị đang có ở vùng đích (cùng kích thước hàng/cột với src).
// - op==None: kết quả = src (trừ khi skipBlanks và ô src trống -> giữ dest).
// - op số học: cả hai ô là số -> tính dest (op) src; ô src trống + skipBlanks
//   -> giữ dest; còn lại (không phải số) -> lấy src. Chia cho 0 -> giữ dest.
// Kết quả luôn cùng hình dạng với src; thiếu dest coi như rỗng (số 0 khi tính).
QVector<QVector<QString>> applyOperation(const QVector<QVector<QString>> &dest,
                                         const QVector<QVector<QString>> &src,
                                         Op op, bool skipBlanks);

// Áp một phép tính với HẰNG SỐ `k` lên một ô: nếu ô là số -> trả `cell op k` dạng chuỗi;
// nếu không phải số (hoặc rỗng) -> giữ nguyên. Chia cho 0 -> giữ nguyên.
QString applyConstant(const QString &cell, Op op, double k);

} // namespace pasteops
