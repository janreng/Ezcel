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
        {"lang_vi",     {QStringLiteral("Tiếng Việt"),  QStringLiteral("Tiếng Việt")}},
        {"lang_en",     {QStringLiteral("English"),     QStringLiteral("English")}},
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
