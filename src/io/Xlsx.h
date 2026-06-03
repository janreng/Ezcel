#pragma once
#include "io/Csv.h" // dùng lại Grid + normalize
#include <QString>
#include <QVector>

// Đọc/ghi XLSX qua thư viện QXlsx (port phần XLSX của io_utils.py).
// Đợt này: giá trị + ô gộp (merge). Định dạng (font/màu/viền/number_format)
// sẽ bổ sung ở đợt sau.
namespace xlsxio {

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
};

// Đọc sheet ĐẦU TIÊN của workbook vào out (app hiện 1 sheet). Trả false nếu lỗi.
bool loadXlsx(const QString &path, Sheet &out);

// Ghi 1 sheet (giá trị + ô gộp) ra file XLSX. Trả false nếu lỗi.
bool saveXlsx(const QString &path, const QString &sheetName,
              const csvio::Grid &rows, const QVector<Merge> &merges);

} // namespace xlsxio
