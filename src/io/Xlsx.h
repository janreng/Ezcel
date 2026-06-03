#pragma once
#include "io/Csv.h" // dùng lại Grid + normalize
#include <QString>
#include <QVector>
#include <QHash>
#include <QVariant>
#include <QMap>
#include <QPair>

// Đọc/ghi XLSX qua thư viện QXlsx (port phần XLSX của io_utils.py).
// Hỗ trợ: giá trị + công thức + ô gộp + ĐỊNH DẠNG (font/cỡ/đậm-nghiêng-gạch/
// màu chữ-nền/căn lề/wrap/number_format). Viền ô để sau (cần model vẽ viền).
namespace xlsxio {

// Định dạng 1 ô = dict khóa->giá trị (cùng kiểu Format của model).
using Attrs = QHash<QString, QVariant>;

// Vùng ô gộp 0-based [top,left]..[bottom,right] (giữ io tách khỏi model).
struct Merge {
    int top, left, bottom, right;
    bool operator==(const Merge &o) const {
        return top == o.top && left == o.left && bottom == o.bottom && right == o.right;
    }
};

struct Sheet {
    QString name;
    csvio::Grid rows;
    QVector<Merge> merges;
    QMap<QPair<int, int>, Attrs> formats; // (row,col) 0-based -> định dạng
};

// Đọc sheet ĐẦU TIÊN của workbook vào out (app hiện 1 sheet). Trả false nếu lỗi.
bool loadXlsx(const QString &path, Sheet &out);

// Ghi 1 sheet (giá trị + ô gộp + định dạng) ra file XLSX. Trả false nếu lỗi.
bool saveXlsx(const QString &path, const QString &sheetName,
              const csvio::Grid &rows, const QVector<Merge> &merges,
              const QMap<QPair<int, int>, Attrs> &formats = {});

// Đọc TẤT CẢ sheet trong workbook (cho nhiều trang tính). Rỗng nếu lỗi.
QVector<Sheet> loadAllSheets(const QString &path);

// Ghi NHIỀU sheet vào một file XLSX. Trả false nếu lỗi.
bool saveSheets(const QString &path, const QVector<Sheet> &sheets);

} // namespace xlsxio
