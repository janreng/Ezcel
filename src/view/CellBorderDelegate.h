#pragma once
#include <QStyledItemDelegate>
#include <QStringList>
#include <QHash>

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

private:
    QAbstractItemView *m_view;
    QStringList m_fnNames;
    QHash<QString, QString> m_fnSigs;
    mutable QWidget *m_activeEditor = nullptr;
};
