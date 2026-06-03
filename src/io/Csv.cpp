#include "io/Csv.h"

#include <QFile>
#include <QTextStream>

namespace csvio {

Grid normalize(Grid rows, int minCols) {
    int width = minCols;
    for (const auto &r : rows) width = qMax(width, int(r.size()));
    for (auto &r : rows) while (r.size() < width) r.push_back(QString());
    if (rows.isEmpty()) rows.push_back(QVector<QString>(width));
    return rows;
}

QChar sniffDelimiter(const QString &sample) {
    static const QChar kCands[] = {QLatin1Char(','), QLatin1Char(';'),
                                   QLatin1Char('\t'), QLatin1Char('|')};
    // Đếm trên dòng đầu, bỏ qua ký tự nằm trong cặp nháy kép.
    int firstLineEnd = sample.indexOf(QLatin1Char('\n'));
    QString line = firstLineEnd < 0 ? sample : sample.left(firstLineEnd);

    QChar best = QLatin1Char(',');
    int bestCount = 0;
    for (QChar cand : kCands) {
        int count = 0;
        bool inQuotes = false;
        for (QChar ch : line) {
            if (ch == QLatin1Char('"')) inQuotes = !inQuotes;
            else if (ch == cand && !inQuotes) ++count;
        }
        if (count > bestCount) { bestCount = count; best = cand; }
    }
    return best;
}

Grid parse(const QString &text, QChar delimiter) {
    Grid rows;
    QVector<QString> row;
    QString field;
    bool inQuotes = false;
    bool sawAny = false; // đã gặp ký tự/ô nào trên hàng hiện tại chưa

    auto endField = [&]() { row.push_back(field); field.clear(); };
    auto endRow = [&]() { endField(); rows.push_back(row); row.clear(); sawAny = false; };

    for (int i = 0; i < text.size(); ++i) {
        QChar ch = text[i];
        if (inQuotes) {
            if (ch == QLatin1Char('"')) {
                if (i + 1 < text.size() && text[i + 1] == QLatin1Char('"')) { field += QLatin1Char('"'); ++i; }
                else inQuotes = false;
            } else {
                field += ch;
            }
            continue;
        }
        if (ch == QLatin1Char('"')) { inQuotes = true; sawAny = true; }
        else if (ch == delimiter) { sawAny = true; endField(); }
        else if (ch == QLatin1Char('\n')) { endRow(); }
        else if (ch == QLatin1Char('\r')) { /* nuốt; CRLF/CR -> kết thúc ở \n hoặc cuối */
            if (i + 1 >= text.size() || text[i + 1] != QLatin1Char('\n')) endRow();
        } else { field += ch; sawAny = true; }
    }
    // Hàng cuối chưa có dấu xuống dòng.
    if (sawAny || !field.isEmpty() || !row.isEmpty()) endRow();
    return rows;
}

Grid loadCsv(const QString &path, bool *ok) {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) { if (ok) *ok = false; return normalize({}); }
    QByteArray bytes = f.readAll();
    f.close();
    if (ok) *ok = true;

    // Bỏ BOM UTF-8 nếu có rồi giải mã UTF-8.
    if (bytes.startsWith("\xEF\xBB\xBF")) bytes.remove(0, 3);
    QString text = QString::fromUtf8(bytes);

    QChar delim = sniffDelimiter(text.left(4096));
    return normalize(parse(text, delim));
}

static QString encodeField(const QString &v) {
    bool needQuote = v.contains(QLatin1Char(',')) || v.contains(QLatin1Char('"'))
                  || v.contains(QLatin1Char('\n')) || v.contains(QLatin1Char('\r'));
    if (!needQuote) return v;
    QString out = v;
    out.replace(QLatin1Char('"'), QLatin1String("\"\""));
    return QLatin1Char('"') + out + QLatin1Char('"');
}

QString toCsv(const Grid &rows) {
    QString out;
    for (const auto &row : rows) {
        for (int c = 0; c < row.size(); ++c) {
            if (c) out += QLatin1Char(',');
            out += encodeField(row[c]);
        }
        out += QLatin1String("\r\n");
    }
    return out;
}

bool saveCsv(const QString &path, const Grid &rows) {
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) return false;
    f.write("\xEF\xBB\xBF"); // BOM UTF-8
    f.write(toCsv(rows).toUtf8());
    f.close();
    return true;
}

} // namespace csvio
