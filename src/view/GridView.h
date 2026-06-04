#pragma once
#include <QTableView>
#include <QAbstractItemDelegate>

// Lưới chính: QTableView có chỉnh hành vi nhập liệu giống Excel (Spec 03).
// Khi sửa NGAY TRONG Ô rồi nhấn Enter -> commit xong tự nhảy XUỐNG ô dưới
// (QTableView mặc định không di chuyển). Header-only, không cần MOC.
class GridView : public QTableView {
public:
    using QTableView::QTableView;

protected:
    void closeEditor(QWidget *editor, QAbstractItemDelegate::EndEditHint hint) override
    {
        QTableView::closeEditor(editor, hint);
        // SubmitModelCache = đóng editor do nhấn Enter (theo QStyledItemDelegate).
        if (hint == QAbstractItemDelegate::SubmitModelCache && model() && currentIndex().isValid()) {
            const QModelIndex cur = currentIndex();
            const QModelIndex below = model()->index(cur.row() + 1, cur.column());
            if (below.isValid()) setCurrentIndex(below);
        }
    }
};
