#pragma once
#include <QStyledItemDelegate>
#include <QStringList>
#include <QHash>
#include <QVector>
#include <functional>
#include "model/SpreadsheetModel.h" // MergeRange

class QAbstractItemView;

// Vẽ viền xanh 2px (#217346) quanh ô ĐANG CHỌN — dấu hiệu đặc trưng của Excel.
// Kèm popup gợi ý tên hàm khi gõ '=' trong ô (Spec 12).
class CellBorderDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit CellBorderDelegate(QAbstractItemView *view, QObject *parent = nullptr);
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;
    // Chặn commit-khi-mất-focus của QStyledItemDelegate khi đang sửa CÔNG THỨC và
    // focus chuyển sang lưới (người dùng bấm ô khác để "point mode"). Nếu không chặn,
    // Qt commit công thức dở dang (=SUM() trước khi kịp chèn địa chỉ -> #VALUE!.
    bool eventFilter(QObject *obj, QEvent *event) override;
    void setFunctionNames(const QStringList &names) { m_fnNames = names; }
    void setFunctionSignatures(const QHash<QString, QString> &sigs) { m_fnSigs = sigs; }
    // Editor (QLineEdit) đang mở, để "point mode" chèn địa chỉ ô vào công thức. null nếu không.
    QWidget *activeEditor() const { return m_activeEditor; }
    // Các vùng tham chiếu của công thức đang nhập -> vẽ viền nét đứt nhiều màu (point mode).
    void setReferenceRanges(const QVector<MergeRange> &ranges) { m_refRanges = ranges; }
    // Callback khi chữ trong editor ô đổi (để tô viền nét đứt tham chiếu lúc gõ trong ô).
    void setEditTextCallback(std::function<void(const QString &)> cb) { m_onEditText = std::move(cb); }

private:
    QAbstractItemView *m_view;
    QStringList m_fnNames;
    QHash<QString, QString> m_fnSigs;
    mutable QWidget *m_activeEditor = nullptr;
    QVector<MergeRange> m_refRanges;
    std::function<void(const QString &)> m_onEditText;
};
