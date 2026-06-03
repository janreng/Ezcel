#include "io/Xlsx.h"

#include <QFileInfo>
#include <optional>

#include "xlsxdocument.h"
#include "xlsxworksheet.h"
#include "xlsxcell.h"
#include "xlsxcellformula.h"
#include "xlsxcellrange.h"
#include "xlsxformat.h"

#include <QColor>

namespace xlsxio {

// ----------------------------------------------------- chuyển đổi định dạng
// Attrs (dict của app) -> QXlsx::Format (port _build_style).
static QXlsx::Format attrsToXlsx(const Attrs &a) {
    QXlsx::Format f;
    if (a.contains("font")) f.setFontName(a.value("font").toString());
    if (a.contains("size")) f.setFontSize(a.value("size").toInt());
    if (a.value("bold").toBool()) f.setFontBold(true);
    if (a.value("italic").toBool()) f.setFontItalic(true);
    if (a.value("underline").toBool()) f.setFontUnderline(QXlsx::Format::FontUnderlineSingle);
    if (a.value("strike").toBool()) f.setFontStrikeOut(true);
    if (a.contains("color")) f.setFontColor(QColor(a.value("color").toString()));
    if (a.contains("bg")) {
        f.setFillPattern(QXlsx::Format::PatternSolid);
        f.setPatternForegroundColor(QColor(a.value("bg").toString()));
    }
    const QString h = a.value("halign").toString();
    if (h == "left") f.setHorizontalAlignment(QXlsx::Format::AlignLeft);
    else if (h == "center") f.setHorizontalAlignment(QXlsx::Format::AlignHCenter);
    else if (h == "right") f.setHorizontalAlignment(QXlsx::Format::AlignRight);
    const QString v = a.value("valign").toString();
    if (v == "top") f.setVerticalAlignment(QXlsx::Format::AlignTop);
    else if (v == "middle") f.setVerticalAlignment(QXlsx::Format::AlignVCenter);
    else if (v == "bottom") f.setVerticalAlignment(QXlsx::Format::AlignBottom);
    if (a.value("wrap").toString() == "wrap") f.setTextWrap(true);
    if (a.contains("number_format")) f.setNumberFormat(a.value("number_format").toString());
    return f;
}

// QXlsx::Format -> Attrs (port _read_fmt; bỏ giá trị mặc định như Python).
static Attrs xlsxToAttrs(const QXlsx::Format &f) {
    Attrs a;
    if (f.isEmpty() || !f.isValid()) return a;
    if (!f.fontName().isEmpty() && f.fontName() != "Calibri") a.insert("font", f.fontName());
    if (f.fontSize() > 0 && f.fontSize() != 11) a.insert("size", f.fontSize());
    if (f.fontBold()) a.insert("bold", true);
    if (f.fontItalic()) a.insert("italic", true);
    if (f.fontUnderline() != QXlsx::Format::FontUnderlineNone) a.insert("underline", true);
    if (f.fontStrikeOut()) a.insert("strike", true);
    QColor fc = f.fontColor();
    if (fc.isValid() && fc != QColor(Qt::black) && fc.name() != "#000000")
        a.insert("color", fc.name());
    if (f.fillPattern() == QXlsx::Format::PatternSolid) {
        QColor bg = f.patternForegroundColor();
        if (bg.isValid() && bg.name() != "#000000") a.insert("bg", bg.name());
    }
    switch (f.horizontalAlignment()) {
    case QXlsx::Format::AlignLeft: a.insert("halign", "left"); break;
    case QXlsx::Format::AlignHCenter: a.insert("halign", "center"); break;
    case QXlsx::Format::AlignRight: a.insert("halign", "right"); break;
    default: break;
    }
    switch (f.verticalAlignment()) {
    case QXlsx::Format::AlignTop: a.insert("valign", "top"); break;
    case QXlsx::Format::AlignVCenter: a.insert("valign", "middle"); break;
    case QXlsx::Format::AlignBottom: a.insert("valign", "bottom"); break;
    default: break;
    }
    if (f.textWrap()) a.insert("wrap", "wrap");
    QString nf = f.numberFormat();
    if (nf.isEmpty()) {
        // numFmt dựng sẵn của Excel chỉ lưu id (không có formatCode) -> tự ánh xạ lại.
        static const QHash<int, QString> kBuiltin = {
            {1, "0"}, {2, "0.00"}, {3, "#,##0"}, {4, "#,##0.00"},
            {9, "0%"}, {10, "0.00%"},
        };
        nf = kBuiltin.value(f.numberFormatIndex());
    }
    if (!nf.isEmpty() && nf != "General") a.insert("number_format", nf);
    return a;
}

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
    out.formats.clear();
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
                Attrs a = xlsxToAttrs(cell->format());
                if (!a.isEmpty()) out.formats.insert({r - 1, c - 1}, a);
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
              const csvio::Grid &rows, const QVector<Merge> &merges,
              const QMap<QPair<int, int>, Attrs> &formats) {
    QXlsx::Document doc;
    for (int r = 0; r < rows.size(); ++r) {
        for (int c = 0; c < rows[r].size(); ++c) {
            const QString &val = rows[r][c];
            auto fit = formats.constFind({r, c});
            const bool hasFmt = (fit != formats.constEnd());
            if (val.isEmpty() && !hasFmt) continue;
            QXlsx::Format xf = hasFmt ? attrsToXlsx(*fit) : QXlsx::Format();
            // Công thức -> QXlsx tự nhận (mở đầu '='); số -> ghi số; còn lại chuỗi; rỗng+fmt -> ô trống có định dạng.
            if (val.isEmpty()) {
                doc.write(r + 1, c + 1, QVariant(), xf);
            } else if (val.startsWith(QLatin1Char('=')) && val.size() > 1) {
                doc.write(r + 1, c + 1, val, xf);
            } else if (auto num = tryNumber(val)) {
                doc.write(r + 1, c + 1, *num, xf);
            } else {
                doc.write(r + 1, c + 1, val, xf);
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
