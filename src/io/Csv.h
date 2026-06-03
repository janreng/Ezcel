#pragma once
#include <QString>
#include <QVector>

// Đọc/ghi CSV (port phần CSV của io_utils.py). Thuần dữ liệu, chỉ phụ thuộc
// Qt Core — không dính GUI/model nên test headless được.
//
// XLSX (giữ định dạng + ô gộp) sẽ làm ở đợt sau, cần thư viện OOXML.
namespace csvio {

using Grid = QVector<QVector<QString>>;

// Chuẩn hóa thành ma trận chữ nhật toàn chuỗi (port _normalize).
// Bảo đảm tối thiểu minCols cột và ít nhất 1 hàng.
Grid normalize(Grid rows, int minCols = 1);

// Đoán dấu phân cách trong số {',', ';', '\t', '|'} theo hàng đầu (ngoài dấu
// nháy); mặc định ',' nếu không rõ. Port tinh giản csv.Sniffer.
QChar sniffDelimiter(const QString &sample);

// Phân tích văn bản CSV (RFC 4180: nháy kép, "" thoát nháy, xuống dòng trong ô).
Grid parse(const QString &text, QChar delimiter);

// Đọc file CSV: bỏ BOM, đoán dấu phân cách, parse, normalize. Đặt *ok nếu mở được.
Grid loadCsv(const QString &path, bool *ok = nullptr);

// Kết xuất CSV (dấu phẩy, CRLF, nháy khi cần, "" thoát nháy).
QString toCsv(const Grid &rows);

// Ghi file CSV kèm BOM UTF-8 (để Excel mở đúng tiếng Việt). Trả false nếu lỗi.
bool saveCsv(const QString &path, const Grid &rows);

} // namespace csvio
