#include "io/Xlsx.h"

#include <QFileInfo>
#include <optional>

#include "xlsxdocument.h"
#include "xlsxworksheet.h"
#include "xlsxcell.h"
#include "xlsxcellformula.h"
#include "xlsxcellrange.h"

namespace xlsxio {

// Tên sheet hợp lệ cho Excel (<=31 ký tự, bỏ ký tự cấm). Port _safe_sheet_name.
static QString safeSheetName(const QString &name) {
    QString s = name;
    for (QChar bad : {QChar('['), QChar(']'), QChar(':'), QChar('*'),
                      QChar('?'), QChar('/'), QChar('\\')})
        s.replace(bad, QLatin1Char(' '));
    s = s.trimmed();
    if (s.isEmpty()) s = QStringLiteral("Sheet");
    return s.left(31);
}

// Chuỗi -> số (int nếu nguyên, double nếu có '.'/'e'); nullopt nếu không phải số.
// Port _try_number.
static std::optional<QVariant> tryNumber(const QString &text) {
    bool ok = false;
    if (text.contains(QLatin1Char('.')) || text.contains(QLatin1Char('e'))
        || text.contains(QLatin1Char('E'))) {
        double d = text.toDouble(&ok);
        if (ok) return QVariant(d);
        return std::nullopt;
    }
    qlonglong n = text.toLongLong(&ok);
    if (ok) return QVariant(n);
    return std::nullopt;
}

bool loadXlsx(const QString &path, Sheet &out) {
    QXlsx::Document doc(path);
    if (!doc.load()) return false;
    const QStringList names = doc.sheetNames();
    if (names.isEmpty()) return false;

    doc.selectSheet(names.first());
    out.name = names.first();
    QXlsx::Worksheet *ws = doc.currentWorksheet();

    QXlsx::CellRange dim = doc.dimension();
    int lastRow = qMax(dim.lastRow(), 1);
    int lastCol = qMax(dim.lastColumn(), 1);

    csvio::Grid grid;
    for (int r = 1; r <= lastRow; ++r) {
        QVector<QString> row;
        for (int c = 1; c <= lastCol; ++c) {
            QString text;
            auto cell = doc.cellAt(r, c);
            if (cell) {
                QXlsx::CellFormula f = cell->formula();
                if (f.isValid() && !f.formulaText().isEmpty()) {
                    text = QLatin1Char('=') + f.formulaText();
                } else {
                    QVariant v = cell->value();
                    if (!v.isNull()) text = v.toString();
                }
            }
            row.push_back(text);
        }
        grid.push_back(row);
    }
    out.rows = csvio::normalize(grid);

    out.merges.clear();
    if (ws) {
        const QList<QXlsx::CellRange> mc = ws->mergedCells();
        for (const QXlsx::CellRange &m : mc)
            out.merges.push_back({m.firstRow() - 1, m.firstColumn() - 1,
                                  m.lastRow() - 1, m.lastColumn() - 1});
    }
    return true;
}

bool saveXlsx(const QString &path, const QString &sheetName,
              const csvio::Grid &rows, const QVector<Merge> &merges) {
    QXlsx::Document doc;
    for (int r = 0; r < rows.size(); ++r) {
        for (int c = 0; c < rows[r].size(); ++c) {
            const QString &val = rows[r][c];
            if (val.isEmpty()) continue;
            // Công thức -> để QXlsx tự nhận (chuỗi mở đầu '='); số -> ghi số; còn lại chuỗi.
            if (val.startsWith(QLatin1Char('=')) && val.size() > 1) {
                doc.write(r + 1, c + 1, val);
            } else if (auto num = tryNumber(val)) {
                doc.write(r + 1, c + 1, *num);
            } else {
                doc.write(r + 1, c + 1, val);
            }
        }
    }

    for (const Merge &m : merges)
        doc.mergeCells(QXlsx::CellRange(m.top + 1, m.left + 1, m.bottom + 1, m.right + 1));

    // Đổi tên sheet (sheet mặc định "Sheet1" tạo lười sau lần write đầu tiên).
    const QStringList names = doc.sheetNames();
    if (!names.isEmpty()) doc.renameSheet(names.first(), safeSheetName(sheetName));

    return doc.saveAs(path);
}

} // namespace xlsxio
