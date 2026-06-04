#pragma once
#include <QAbstractTableModel>
#include <QVector>
#include <QString>
#include <QHash>
#include <QSet>
#include <QVariant>
#include <QFont>
#include <QMap>
#include <QPair>
#include <QStringList>
#include <utility>
#include "formula/Formula.h"
#include <optional>
#include "model/CondFormat.h"
#include "model/Validation.h"
#include "model/CellShift.h"

// Vùng ô gộp [top,left]..[bottom,right] (port tuple (t,l,b,r) của Python).
struct MergeRange {
    int top, left, bottom, right;
    bool operator==(const MergeRange &o) const {
        return top == o.top && left == o.left && bottom == o.bottom && right == o.right;
    }
    bool contains(int r, int c) const {
        return top <= r && r <= bottom && left <= c && c <= right;
    }
};

// Bản port của table_model.py — SpreadsheetModel(QAbstractTableModel).
// P1 (đợt này): dữ liệu thô, tính công thức (cache + chống vòng), định dạng ô
// (_fmt generic), undo/redo, recalc chọn lọc theo đồ thị phụ thuộc, các role
// hiển thị (align/font/màu nền/màu chữ), hiện-công-thức (Ctrl+`).
// Để sau: merge, conditional format, cross-sheet, sort, autofill, copy/paste.
class SpreadsheetModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    using Format = QHash<QString, QVariant>; // định dạng 1 ô (như dict Python)

    explicit SpreadsheetModel(QObject *parent = nullptr);

    // QAbstractTableModel
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    void resizeGrid(int rows, int cols);
    // Chèn/xóa hàng-cột (đều undoable, dời định dạng + ô gộp theo). Port insert/removeRows/Columns.
    void insertRows(int row, int count = 1);
    void removeRows(int row, int count = 1);
    void insertColumns(int col, int count = 1);
    void removeColumns(int col, int count = 1);
    // Chèn/xóa Ô với dịch chuyển một phần (Spec 09): dời nội dung+định dạng+ghi chú
    // theo hướng dir. Trả false nếu vùng ảnh hưởng chạm ô gộp (từ chối để khỏi vỡ merge).
    bool shiftCells(int top, int left, int bottom, int right, cellshift::Dir dir);
    // Nạp toàn bộ lưới từ file (CSV/XLSX): thay dữ liệu, dựng lại deps, xóa undo.
    void loadGrid(const QVector<QVector<QString>> &rows);
    // Toàn bộ ô thô (để lưu file). Hàng/cột rỗng vẫn giữ nguyên kích thước lưới.
    const QVector<QVector<QString>> &grid() const { return m_data; }
    static QString columnLabel(int col);

    // Giá trị ĐÃ TÍNH của ô (resolver cho engine; chống vòng tham chiếu).
    QVariant evalCell(int row, int col) const;
    QVariant cellValue(int row, int col) const { return evalCell(row, col); } // public

    // Bộ phân giải tham chiếu chéo sheet (Sheet1!A1): tra giá trị ô của sheet khác theo tên.
    // MainWindow gắn để 1 sheet đọc được ô của sheet khác. Đổi -> xóa cache để tính lại.
    void setSheetResolver(formula::SheetResolver r) { m_sheetResolver = std::move(r); m_evalCache.clear(); }
    // Xóa cache công thức KHÔNG phát tín hiệu (để sheet khác đổi thì sheet này tính lại).
    void clearEvalCacheOnly() { m_evalCache.clear(); }

    // Định dạng: đặt thuộc tính cho vùng [top,left]..[bottom,right]; value null -> xóa key.
    void setFormat(int top, int left, int bottom, int right, const Format &attrs);
    // Định dạng hiện tại của 1 ô (để đồng bộ nút toolbar / chọn màu sẵn).
    Format formatAt(int row, int col) const { return m_fmt.value(key(row, col)); }

    // Ghi chú ô (note): đặt (text rỗng -> xóa), lấy. Hiện tooltip + dấu tam giác.
    void setNote(int row, int col, const QString &text);
    QString note(int row, int col) const { return m_notes.value(key(row, col)); }

    // Truy vết phụ thuộc (Spec 32): precedents = các ô mà (row,col) tham chiếu tới;
    // dependents = các ô tham chiếu tới (row,col). allLevels=true -> lan truyền BFS.
    QVector<QPair<int, int>> precedents(int row, int col, bool allLevels) const;
    QVector<QPair<int, int>> dependents(int row, int col, bool allLevels) const;

    // Vùng đặt tên (named range): đặt tên cho 1 vùng; tra theo tên.
    void defineName(const QString &name, const MergeRange &range);
    bool lookupName(const QString &name, MergeRange &out) const;
    QStringList definedNames() const { return m_names.keys(); }
    bool removeName(const QString &name);                 // xóa vùng đặt tên; true nếu có

    // Chuỗi địa chỉ kiểu A1 cho một vùng: "A1" nếu 1 ô, "A1:C3" nếu vùng. (Spec 31)
    static QString rangeRef(const MergeRange &r);
    // Toàn bộ định dạng theo (row,col) — để lưu ra file.
    QMap<QPair<int, int>, Format> cellFormats() const;
    // Nạp định dạng từ file (thay toàn bộ, KHÔNG undo — dùng khi mở file).
    void setCellFormats(const QMap<QPair<int, int>, Format> &fmts);

    // Hiện công thức gốc thay vì kết quả (Ctrl+`).
    bool showFormulas() const { return m_showFormulas; }
    void setShowFormulas(bool on);

    // Thay mọi lần xuất hiện find->repl trong dữ liệu thô (undoable). Trả số ô đổi.
    int replaceAll(const QString &find, const QString &repl, bool matchCase = false);

    // Sắp xếp các hàng của vùng [top,left]..[bottom,right] theo cột keyCol
    // (chỉ số cột tuyệt đối, phải nằm trong vùng). Ổn định, kiểu Excel (undoable).
    void sortRange(int top, int left, int bottom, int right, int keyCol, bool ascending);
    // Sắp xếp nhiều cấp (Spec 15): mỗi cấp = {cột tuyệt đối, tăng/giảm}. Áp theo thứ tự ưu tiên.
    void sortRangeMulti(int top, int left, int bottom, int right,
                        const QVector<QPair<int, bool>> &keyCols);

    // Thao tác vùng (đều undoable theo ô).
    void clearRange(int top, int left, int bottom, int right);     // chỉ xóa nội dung
    void clearFormatsRange(int top, int left, int bottom, int right); // chỉ xóa định dạng (Spec 09)
    void clearAllRange(int top, int left, int bottom, int right);  // xóa cả nội dung + định dạng
    void pasteBlock(int top, int left, const QVector<QVector<QString>> &block,
                    int srcAnchorRow = -1, int srcAnchorCol = -1);
    void autofillVertical(int col, int srcTop, int srcBottom, int dstBottom);
    void autofillHorizontal(int row, int srcLeft, int srcRight, int dstRight);

    // Định dạng có điều kiện: thêm quy tắc cho vùng đang chọn; xóa hết.
    void addCondRule(const cond::Rule &rule);
    void clearCondRules();
    int condRuleCount() const { return m_condRules.size(); }

    // Kiểm tra dữ liệu (data validation): thêm/xóa quy tắc; setData sẽ từ chối giá trị sai.
    void addValidationRule(const validation::Rule &rule);
    void clearValidationRules();
    int validationRuleCount() const { return m_validationRules.size(); }

    // Gộp ô (merge). Giữ nội dung ô góc trên-trái, xóa phần còn lại. Đều undoable.
    const QVector<MergeRange> &merges() const { return m_merges; }
    std::optional<MergeRange> mergeAt(int row, int col) const;
    void mergeCells(int top, int left, int bottom, int right);
    void unmergeCells(int top, int left, int bottom, int right); // bỏ mọi vùng giao
    void toggleMerge(int top, int left, int bottom, int right);
    // Gộp/bỏ gộp NHIỀU vùng rời trong 1 bước undo, nhất quán như Excel.
    void toggleMergeRanges(const QVector<MergeRange> &boxes);

    // Undo/redo.
    bool undo();
    bool redo();
    bool canUndo() const { return !m_undo.isEmpty(); }
    bool canRedo() const { return !m_redo.isEmpty(); }

signals:
    void contentChanged();
    void mergesChanged(); // view cập nhật span khi danh sách ô gộp đổi
    void validationFailed(const QString &msg); // nhập sai quy tắc kiểm tra dữ liệu

private:
    struct CellChange { int row, col; QString oldVal, newVal; };
    struct FmtChange  { int row, col; Format oldFmt, newFmt; };
    // Ảnh chụp toàn lưới cho thao tác cấu trúc (chèn/xóa hàng-cột).
    struct Snapshot { QVector<QVector<QString>> data; QHash<qint64, Format> fmt; QVector<MergeRange> merges; };
    struct UndoEntry  {
        QVector<CellChange> cells; QVector<FmtChange> fmts;
        bool hasMerges = false; QVector<MergeRange> mergesOld, mergesNew;
        std::optional<Snapshot> snapBefore, snapAfter; // có giá trị -> thao tác cấu trúc
    };

    QVector<QVector<QString>> m_data;            // lưới thô (chuỗi/công thức)
    mutable QHash<qint64, QVariant> m_evalCache; // giá trị công thức đã tính
    formula::SheetResolver m_sheetResolver;      // tra ô sheet khác (Sheet1!A1)
    mutable QSet<qint64> m_evaluating;           // ô đang tính (vòng lặp)
    QHash<qint64, Format> m_fmt;                 // định dạng theo ô
    QVector<MergeRange> m_merges;                // vùng ô gộp
    QVector<cond::Rule> m_condRules;             // định dạng có điều kiện
    QVector<validation::Rule> m_validationRules; // kiểm tra dữ liệu nhập
    QHash<qint64, QString> m_notes;              // ghi chú theo ô
    QHash<QString, MergeRange> m_names;          // vùng đặt tên
    mutable QHash<QString, QFont> m_fontCache;   // QFont chia sẻ theo style
    mutable QHash<QString, bool> m_fontCacheNull;// style không có font

    // Đồ thị phụ thuộc cho recalc chọn lọc.
    QHash<qint64, QSet<qint64>> m_deps;          // A -> các ô A tham chiếu
    QHash<qint64, QSet<qint64>> m_dependents;    // B -> các ô tham chiếu B

    bool m_showFormulas = false;
    QVector<UndoEntry> m_undo, m_redo;
    static constexpr int kUndoLimit = 100;

    static qint64 key(int row, int col) { return (qint64(row) << 32) | quint32(col); }
    static int keyRow(qint64 k) { return int(k >> 32); }
    static int keyCol(qint64 k) { return int(k & 0xffffffff); }

    QString displayValue(int row, int col) const;
    // Màu theo định dạng điều kiện cho ô (fg=true: màu chữ; false: màu nền). Rỗng nếu không khớp.
    QString condColorFor(int row, int col, bool fg) const;
    int alignmentFlags(int row, int col) const;
    QVariant fontFor(int row, int col) const;
    bool looksNumeric(int row, int col) const;

    void rebuildDeps();
    void updateDeps(int row, int col);
    void recalculate(int row, int col);   // selective BFS từ ô đổi
    void recalculateAll();                 // bulk: xóa toàn bộ cache, vẽ lại
    void pushUndo(UndoEntry entry);
    void applyEntry(const UndoEntry &e, bool useOld); // revert(useOld) hoặc reapply
    // Gộp/bỏ gộp 1 vùng vào m_merges (không đẩy undo); ghi ô bị xóa vào e.cells.
    bool mergeBoxInto(const MergeRange &box, UndoEntry &e);
    bool unmergeBoxInto(const MergeRange &box);
    void toggleMergeRangesImpl(const QVector<MergeRange> &boxes, bool forceMerge);
    Snapshot snapshot() const;                 // chụp toàn bộ data+fmt+merges
    void restoreSnapshot(const Snapshot &s);   // khôi phục (reset model)
    void shiftFmtRows(int row, int count);     // dời khóa _fmt khi chèn/xóa hàng
    void shiftFmtCols(int col, int count);     // dời khóa _fmt khi chèn/xóa cột
    void applyCellChanges(QVector<CellChange> changes); // batch: undo + set + deps + recalc
};
