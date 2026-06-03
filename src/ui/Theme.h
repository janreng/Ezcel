#pragma once
#include <QString>
#include <QFont>

// Bảng màu/đo lường theo Microsoft Excel — nguồn: docs/specs/50-design-system.md.
// KHÔNG hardcode màu ở nơi khác; lấy hằng ở đây. Header-only (chỉ hằng).
namespace theme {

// --- Font (Spec 50.2) ---
// UI: Segoe UI (Windows). Nội dung ô: Aptos Narrow 11pt (mặc định Excel 365),
// fallback Calibri/Segoe UI nếu máy chưa có.
inline QFont uiFont() {
    QFont f; f.setFamilies({QStringLiteral("Segoe UI"), QStringLiteral("Arial")});
    f.setPointSize(9); return f;
}
inline QFont cellFont() {
    QFont f; f.setFamilies({QStringLiteral("Aptos Narrow"), QStringLiteral("Calibri"),
                            QStringLiteral("Segoe UI")});
    f.setPointSize(11); return f;
}

// --- Excel signature green ---
inline const QString ExcelGreen      = QStringLiteral("#107C41"); // brand primary
inline const QString SelectionBorder = QStringLiteral("#217346"); // viền ô đang chọn (2px)
inline const QString SelectionFill   = QStringLiteral("#E8F5EE"); // nền vùng chọn
inline const QString HeaderSelected  = QStringLiteral("#CEEAD9"); // header của cột/dòng đang chọn

// --- Neutrals (Office) ---
inline const QString TextPrimary  = QStringLiteral("#323130");
inline const QString Gridline     = QStringLiteral("#E1DFDD");
inline const QString Divider      = QStringLiteral("#C8C6C4");
inline const QString HoverBg      = QStringLiteral("#EDEBE9");
inline const QString RibbonBg     = QStringLiteral("#F3F2F1");
inline const QString CellBg       = QStringLiteral("#FFFFFF");

// --- Định dạng có điều kiện (preset Excel) ---
inline const QString CfRedBg = QStringLiteral("#FFC7CE"), CfRedText = QStringLiteral("#9C0006");
inline const QString CfYellowBg = QStringLiteral("#FFEB9C"), CfYellowText = QStringLiteral("#9C6500");
inline const QString CfGreenBg = QStringLiteral("#C6EFCE"), CfGreenText = QStringLiteral("#006100");

// --- Sizing ---
inline constexpr int RowHeight = 22;
inline constexpr int ColWidth  = 90;

// Stylesheet cho lưới + header (giống Excel: gridline nhạt, vùng chọn xanh nhạt,
// header xám nền, header đang chọn xanh).
inline QString tableStyle() {
    return QStringLiteral(
        "QTableView { gridline-color: %1; background: %2; selection-color: %3; }"
        "QTableView::item:selected { background: %4; color: %3; }"
        "QHeaderView::section { background: %5; color: %3;"
        "  border: none; border-right: 1px solid %1; border-bottom: 1px solid %1; padding: 2px 4px; }"
        "QHeaderView::section:checked { background: %6; color: %7; }"
        "QTableCornerButton::section { background: %5; border: 1px solid %1; }")
        .arg(Gridline, CellBg, TextPrimary, SelectionFill, RibbonBg, HeaderSelected, ExcelGreen);
}

// Stylesheet cho thanh công cụ (dải ribbon nền xám nhạt, nút bo nhẹ, hover xám,
// dấu ngăn nhóm). Áp cho từng QToolBar.
inline QString toolbarStyle() {
    return QStringLiteral(
        "QToolBar { background: %1; border: none; border-bottom: 1px solid %2; spacing: 2px; padding: 3px; }"
        "QToolBar::separator { width: 1px; background: %3; margin: 3px 5px; }"
        "QToolButton { padding: 4px 8px; border: 1px solid transparent; border-radius: 3px; color: %4; }"
        "QToolButton:hover { background: %5; border: 1px solid %2; }"
        "QToolButton:pressed, QToolButton:checked { background: %6; border: 1px solid %2; }")
        .arg(RibbonBg, Gridline, Divider, TextPrimary, HoverBg, HeaderSelected);
}

} // namespace theme
