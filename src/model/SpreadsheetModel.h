#pragma once
#include <QAbstractTableModel>
#include <QVector>
#include <QString>
#include <QHash>
#include <QSet>
#include <QVariant>
#include <QFont>
#include <optional>

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
    // Nạp toàn bộ lưới từ file (CSV/XLSX): thay dữ liệu, dựng lại deps, xóa undo.
    void loadGrid(const QVector<QVector<QString>> &rows);
    // Toàn bộ ô thô (để lưu file). Hàng/cột rỗng vẫn giữ nguyên kích thước lưới.
    const QVector<QVector<QString>> &grid() const { return m_data; }
    static QString columnLabel(int col);

    // Giá trị ĐÃ TÍNH của ô (resolver cho engine; chống vòng tham chiếu).
    QVariant evalCell(int row, int col) const;
    QVariant cellValue(int row, int col) const { return evalCell(row, col); } // public

    // Định dạng: đặt thuộc tính cho vùng [top,left]..[bottom,right]; value null -> xóa key.
    void setFormat(int top, int left, int bottom, int right, const Format &attrs);
    // Định dạng hiện tại của 1 ô (để đồng bộ nút toolbar / chọn màu sẵn).
    Format formatAt(int row, int col) const { return m_fmt.value(key(row, col)); }

    // Hiện công thức gốc thay vì kết quả (Ctrl+`).
    bool showFormulas() const { return m_showFormulas; }
    void setShowFormulas(bool on);

    // Thao tác vùng (đều undoable theo ô).
    void clearRange(int top, int left, int bottom, int right);
    void pasteBlock(int top, int left, const QVector<QVector<QString>> &block,
                    int srcAnchorRow = -1, int srcAnchorCol = -1);
    void autofillVertical(int col, int srcTop, int srcBottom, int dstBottom);
    void autofillHorizontal(int row, int srcLeft, int srcRight, int dstRight);

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
    mutable QSet<qint64> m_evaluating;           // ô đang tính (vòng lặp)
    QHash<qint64, Format> m_fmt;                 // định dạng theo ô
    QVector<MergeRange> m_merges;                // vùng ô gộp
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
