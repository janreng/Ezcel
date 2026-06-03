#include "ui/I18n.h"
#include <QHash>
#include <QPair>

namespace i18n {

static Lang g_lang = Lang::Vi;

// Bảng dịch: key -> {Việt, Anh}. Bổ sung dần khi dịch thêm UI.
static const QHash<QString, QPair<QString, QString>> &table() {
    static const QHash<QString, QPair<QString, QString>> t = {
        {"menu_file",   {QStringLiteral("&Tệp"),        QStringLiteral("&File")}},
        {"menu_edit",   {QStringLiteral("&Sửa"),        QStringLiteral("&Edit")}},
        {"menu_struct", {QStringLiteral("&Cấu trúc"),   QStringLiteral("&Structure")}},
        {"menu_data",   {QStringLiteral("&Dữ liệu"),    QStringLiteral("&Data")}},
        {"menu_view",   {QStringLiteral("&Xem"),        QStringLiteral("&View")}},
        {"menu_help",   {QStringLiteral("&Trợ giúp"),   QStringLiteral("&Help")}},
        {"menu_settings", {QStringLiteral("&Cài đặt"),  QStringLiteral("&Settings")}},
        {"menu_lang",   {QStringLiteral("Ngôn ngữ"),    QStringLiteral("Language")}},
        {"file_new",    {QStringLiteral("&Mới"),        QStringLiteral("&New")}},
        {"file_open",   {QStringLiteral("&Mở..."),      QStringLiteral("&Open...")}},
        {"file_save",   {QStringLiteral("&Lưu"),        QStringLiteral("&Save")}},
        {"file_saveas", {QStringLiteral("Lưu &thành..."), QStringLiteral("Save &As...")}},
        {"file_quit",   {QStringLiteral("T&hoát"),      QStringLiteral("&Quit")}},
        {"edit_undo",   {QStringLiteral("&Hoàn tác"),   QStringLiteral("&Undo")}},
        {"edit_redo",   {QStringLiteral("Làm &lại"),    QStringLiteral("&Redo")}},
        {"edit_cut",    {QStringLiteral("Cắt"),         QStringLiteral("Cut")}},
        {"edit_copy",   {QStringLiteral("Sao chép"),    QStringLiteral("Copy")}},
        {"edit_paste",  {QStringLiteral("Dán"),         QStringLiteral("Paste")}},
        {"edit_paste_special", {QStringLiteral("Dán đặc biệt..."), QStringLiteral("Paste Special...")}},
        {"edit_clear",  {QStringLiteral("Xóa nội dung"), QStringLiteral("Clear contents")}},
        {"edit_find",   {QStringLiteral("Tìm && Thay thế..."), QStringLiteral("Find && Replace...")}},
        {"view_zoom_in",  {QStringLiteral("Phóng to"),  QStringLiteral("Zoom In")}},
        {"view_zoom_out", {QStringLiteral("Thu nhỏ"),   QStringLiteral("Zoom Out")}},
        {"view_zoom_reset", {QStringLiteral("Thu phóng 100%"), QStringLiteral("Zoom 100%")}},
        {"view_show_formulas", {QStringLiteral("Hiện công thức"), QStringLiteral("Show Formulas")}},
        {"help_check_update", {QStringLiteral("Kiểm tra cập nhật"), QStringLiteral("Check for Updates")}},
        {"help_about",  {QStringLiteral("Giới thiệu Ezcel"), QStringLiteral("About Ezcel")}},
        {"help_shortcuts", {QStringLiteral("Phím tắt"), QStringLiteral("Keyboard Shortcuts")}},
        {"help_stats", {QStringLiteral("Thống kê bảng tính"), QStringLiteral("Workbook Statistics")}},
        {"col_shortcut_keys", {QStringLiteral("Phím"), QStringLiteral("Keys")}},
        {"col_shortcut_desc", {QStringLiteral("Hành vi"), QStringLiteral("Action")}},
        {"lang_vi",     {QStringLiteral("Tiếng Việt"),  QStringLiteral("Tiếng Việt")}},
        {"lang_en",     {QStringLiteral("English"),     QStringLiteral("English")}},
        // Sửa
        {"edit_fill_down", {QStringLiteral("Điền xuống"), QStringLiteral("Fill Down")}},
        {"edit_fill_right",{QStringLiteral("Điền phải"),  QStringLiteral("Fill Right")}},
        {"edit_merge",  {QStringLiteral("Gộp / bỏ gộp ô"), QStringLiteral("Merge / Unmerge")}},
        {"edit_replace",{QStringLiteral("Thay thế..."), QStringLiteral("Replace...")}},
        // Cấu trúc
        {"st_ins_row",  {QStringLiteral("Chèn hàng trên"), QStringLiteral("Insert Row Above")}},
        {"st_ins_col",  {QStringLiteral("Chèn cột trái"),  QStringLiteral("Insert Column Left")}},
        {"st_del_row",  {QStringLiteral("Xóa hàng"),       QStringLiteral("Delete Row")}},
        {"st_del_col",  {QStringLiteral("Xóa cột"),        QStringLiteral("Delete Column")}},
        {"st_hide_row", {QStringLiteral("Ẩn hàng"),        QStringLiteral("Hide Rows")}},
        {"st_hide_col", {QStringLiteral("Ẩn cột"),         QStringLiteral("Hide Columns")}},
        {"st_unhide",   {QStringLiteral("Hiện lại (bỏ ẩn)"), QStringLiteral("Unhide")}},
        {"st_fit_col",  {QStringLiteral("Vừa khít độ rộng cột"), QStringLiteral("Autofit Column Width")}},
        {"st_fit_row",  {QStringLiteral("Vừa khít chiều cao dòng"), QStringLiteral("Autofit Row Height")}},
        // Dữ liệu
        {"data_sort_asc",  {QStringLiteral("Sắp xếp tăng dần"), QStringLiteral("Sort Ascending")}},
        {"data_sort_desc", {QStringLiteral("Sắp xếp giảm dần"), QStringLiteral("Sort Descending")}},
        {"data_cond",   {QStringLiteral("Định dạng có điều kiện..."), QStringLiteral("Conditional Formatting...")}},
        {"data_clear_cond", {QStringLiteral("Xóa định dạng có điều kiện"), QStringLiteral("Clear Conditional Formatting")}},
        {"data_filter", {QStringLiteral("Lọc theo cột hiện tại..."), QStringLiteral("Filter by Current Column...")}},
        {"data_clear_filter", {QStringLiteral("Bỏ lọc (hiện tất cả)"), QStringLiteral("Clear Filter (Show All)")}},
        {"data_filter_values", {QStringLiteral("Lọc theo giá trị..."), QStringLiteral("Filter by Values...")}},
        // Menu chuột phải (Spec 06)
        {"ctx_cut",     {QStringLiteral("Cắt"),            QStringLiteral("Cut")}},
        {"ctx_copy",    {QStringLiteral("Sao chép"),       QStringLiteral("Copy")}},
        {"ctx_paste",   {QStringLiteral("Dán"),            QStringLiteral("Paste")}},
        {"ctx_paste_special", {QStringLiteral("Dán đặc biệt..."), QStringLiteral("Paste Special...")}},
        {"ctx_clear",   {QStringLiteral("Xóa nội dung"),   QStringLiteral("Clear Contents")}},
        {"ctx_pick_list", {QStringLiteral("Chọn từ danh sách..."), QStringLiteral("Pick From List...")}},
        {"hdr_insert_row", {QStringLiteral("Chèn hàng phía trên"), QStringLiteral("Insert Rows Above")}},
        {"hdr_delete_row", {QStringLiteral("Xóa hàng"),    QStringLiteral("Delete Rows")}},
        {"hdr_row_height", {QStringLiteral("Chiều cao hàng..."), QStringLiteral("Row Height...")}},
        {"hdr_row_height_prompt", {QStringLiteral("Chiều cao (điểm ảnh):"), QStringLiteral("Height (pixels):")}},
        {"hdr_autofit_row", {QStringLiteral("Tự khít chiều cao hàng"), QStringLiteral("AutoFit Row Height")}},
        {"hdr_hide_row", {QStringLiteral("Ẩn hàng"),       QStringLiteral("Hide Rows")}},
        {"hdr_unhide_row", {QStringLiteral("Hiện lại hàng"), QStringLiteral("Unhide Rows")}},
        {"hdr_insert_col", {QStringLiteral("Chèn cột phía trước"), QStringLiteral("Insert Columns Left")}},
        {"hdr_delete_col", {QStringLiteral("Xóa cột"),     QStringLiteral("Delete Columns")}},
        {"hdr_col_width", {QStringLiteral("Độ rộng cột..."), QStringLiteral("Column Width...")}},
        {"hdr_col_width_prompt", {QStringLiteral("Độ rộng (điểm ảnh):"), QStringLiteral("Width (pixels):")}},
        {"hdr_autofit_col", {QStringLiteral("Tự khít độ rộng cột"), QStringLiteral("AutoFit Column Width")}},
        {"hdr_hide_col", {QStringLiteral("Ẩn cột"),        QStringLiteral("Hide Columns")}},
        {"hdr_unhide_col", {QStringLiteral("Hiện lại cột"), QStringLiteral("Unhide Columns")}},
    };
    return t;
}

void setLang(Lang l) { g_lang = l; }
Lang lang() { return g_lang; }

QString tr(const QString &key) {
    auto it = table().constFind(key);
    if (it == table().constEnd()) return key;
    return g_lang == Lang::Vi ? it->first : it->second;
}

} // namespace i18n
