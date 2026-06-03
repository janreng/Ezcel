# SPEC 23 — Keyboard Shortcuts (Bảng đầy đủ)

## Mục tiêu
Match đầy đủ shortcut Excel, ưu tiên 90% phím thường dùng. Mọi shortcut đổi được qua [Settings → Customize Shortcuts](#) (đã có hạ tầng `shortcuts.py`).

## Trạng thái hiện tại
Theo README:
- ✓ Ctrl+N/O/S/Z/Y/F/H/C/X/V/D/R/A, Delete, Ctrl+Shift+V.
- ✓ Ctrl+Shift+Arrow (mở rộng vùng chọn).
- ✓ Ctrl+9 / Ctrl+0 hide row/col (chưa verify).
- ✗ F2, F4, F5, F11, F12, F1 — chưa.
- ✗ Shift+F3 (Function Wizard), Alt+= (AutoSum), Ctrl+\` (Show Formulas), Ctrl+; / Ctrl+Shift+; (date/time).
- ✗ Format shortcuts: Ctrl+1, Ctrl+B/I/U/5, Ctrl+Shift+$/%/#/@/!/^/&/_.
- ✗ Ctrl+Spacebar / Shift+Spacebar (entire col/row).
- ✗ Ctrl+Shift+L (toggle filter), Ctrl+T (table), Ctrl+E (Flash Fill), Ctrl+Q (Quick Analysis).
- ✗ Page navigation: Page Up/Down, Ctrl+Page Up/Down, Alt+Page Up/Down.

## 23.1 Navigation (§23.1)

| Phím | Hành vi |
|---|---|
| Arrow Keys | Di chuyển 1 ô |
| Ctrl + Arrow | Nhảy đến ô cuối có data (hoặc ô đầu nếu đang ở ô trống) |
| Home | Về đầu hàng (cột A) |
| Ctrl + Home | Về A1 |
| Ctrl + End | Về ô cuối có data (bottom-right) |
| Page Up / Page Down | Cuộn 1 màn hình |
| Ctrl + Page Up / Down | Sheet trước / sau ([Spec 10](10-sheet-tabs.md)) |
| Alt + Page Up / Down | Cuộn trái / phải 1 màn hình |
| F5 / Ctrl + G | Go To dialog |
| Ctrl + F | Find |
| Ctrl + H | Replace |
| Tab | Sang phải (trong selection: vòng) |
| Shift + Tab | Sang trái |

## 23.2 Selection (§23.2)

| Phím | Hành vi |
|---|---|
| Shift + Arrow | Mở rộng 1 ô |
| Shift + Ctrl + Arrow | Đến mép data |
| Shift + Home | Mở rộng về đầu hàng |
| Shift + Ctrl + Home | Mở rộng về A1 |
| Shift + Ctrl + End | Mở rộng đến cuối data |
| Ctrl + Shift + * | Current region (vùng data liên tục quanh active) |
| Ctrl + A | 1: select Table; 2: select sheet |
| Ctrl + Spacebar | Entire column |
| Shift + Spacebar | Entire row |
| Ctrl + Click | Thêm ô/range vào selection |

## 23.3 Editing (§23.3)

| Phím | Hành vi |
|---|---|
| F2 | Vào Edit mode |
| Delete | Xóa nội dung (giữ format) |
| Ctrl + Z | Undo |
| Ctrl + Y / F4 | Redo / repeat last action |
| Ctrl + X / C / V | Cut / Copy / Paste |
| Ctrl + Alt + V | Paste Special dialog |
| Ctrl + D | Fill Down |
| Ctrl + R | Fill Right |
| Ctrl + Enter | Xác nhận, giữ selection (multi-fill) |
| Alt + Enter | Line break trong ô |
| Ctrl + ; | Chèn ngày hôm nay |
| Ctrl + Shift + ; | Chèn giờ hiện tại |
| Ctrl + - | Delete row/col/cell dialog |
| Ctrl + Shift + + | Insert row/col/cell dialog |
| Ctrl + \` | Toggle hiện formula / kết quả |

## 23.4 Formatting (§23.4)

| Phím | Hành vi |
|---|---|
| Ctrl + 1 | Format Cells dialog ([Spec 08](08-format-cells-dialog.md)) |
| Ctrl + B | Bold |
| Ctrl + I | Italic |
| Ctrl + U | Underline |
| Ctrl + 5 | Strikethrough |
| Alt + H + H | Fill Color picker |
| Alt + H + FC | Font Color picker |
| Ctrl + Shift + $ | Currency format |
| Ctrl + Shift + % | Percentage format |
| Ctrl + Shift + # | Date format (d-mmm-yy) |
| Ctrl + Shift + @ | Time format (h:mm AM/PM) |
| Ctrl + Shift + ! | Number, 2 decimal + comma |
| Ctrl + Shift + ^ | Scientific |
| Ctrl + Shift + & | Outline border |
| Ctrl + Shift + _ | Remove border |

## 23.5 Formula & Function (§23.5)

| Phím | Hành vi |
|---|---|
| = | Bắt đầu formula |
| Alt + = | AutoSum (`=SUM(...)` tự động) |
| Shift + F3 | Insert Function (Function Wizard) |
| F4 | Toggle ref: `A1` → `$A$1` → `A$1` → `$A1` → `A1` |
| Ctrl + Shift + Enter | Array formula (legacy) |
| F9 | Evaluate selection trong formula |
| Shift + F9 | Calculate active sheet |
| Ctrl + Alt + F9 | Recalculate tất cả sheets |
| Ctrl + F3 | Name Manager |
| Ctrl + Shift + F3 | Create Names from Selection |

## 23.6.0 Mac shortcut differences

Excel 365 trên macOS từ 2020+ **đã align** đa số shortcut với Windows (F4 toggle absolute ref, Ctrl+Shift+L filter, Ctrl+B/I/U, Ctrl+1...). Nhưng vẫn có khác biệt:

| Windows | Mac | Mô tả |
|---|---|---|
| Ctrl + ... | **Cmd (⌘) + ...** | Đa số: Cmd+C, Cmd+V, Cmd+S, Cmd+Z... |
| Alt + ... | **Option (⌥) + ...** | Một số: Cmd+Option+V = Paste Special |
| Alt + F (menu KeyTip) | **không có** | Mac KHÔNG support Windows-style Alt KeyTips ribbon navigation |
| F2 (Edit cell) | **Ctrl + U** (hoặc F2 với Fn) | Mac Function keys cần Fn modifier mặc định |
| F4 (toggle absolute ref) | **F4 với Fn** hoặc Cmd+T trong formula | |
| F5 (Go To) | **Ctrl + G** hoặc Fn+F5 | |
| F11 (chart sheet) | **Fn + F11** | |
| Ctrl + ; (date) | **Ctrl + ;** (vẫn dùng Ctrl, không phải Cmd) | |
| Ctrl + Page Up/Down (sheet) | **Option + ←/→** | Mac không có Page Up/Down hardware key đầy đủ |
| Ctrl + Home/End | **Fn + ←/→** hoặc **Cmd + ←/→** | |
| Right-click | **Ctrl + click** hoặc **trackpad 2-finger tap** | |
| Insert hyperlink: Ctrl + K | **Cmd + K** | |
| Find: Ctrl + F | **Cmd + F** | |
| Function Wizard: Shift + F3 | **Shift + F3 với Fn** hoặc **Cmd + Option + R** | |
| AutoSum: Alt + = | **Cmd + Shift + T** | |
| Show Formulas: Ctrl + \` | **Ctrl + \`** (giữ Ctrl, không phải Cmd) | |
| Close workbook: Ctrl + W | **Cmd + W** | |
| Close app: Alt + F4 | **Cmd + Q** | |

### Function keys behavior trên Mac
- Default Mac keyboard: F1-F12 mapped sang system controls (brightness/volume/...).
- User phải:
  - Cách 1: nhấn `Fn + F<n>` mỗi lần (system control + function key).
  - Cách 2: System Settings → Keyboard → ☑ "Use F1, F2, etc. keys as standard function keys" — đổi default.

### Mac-specific shortcuts
| Phím | Hành vi |
|---|---|
| Cmd + , | Open Excel Preferences (≈ File → Options trên Win) |
| Cmd + Q | Quit Excel app |
| Cmd + H | Hide Excel window |
| Cmd + Option + H | Hide other apps |
| Cmd + M | Minimize window |
| Cmd + ` | Switch between Excel windows |
| Cmd + Tab | Switch app |

## 23.6 Workbook & Sheet (§23.6)

| Phím | Hành vi |
|---|---|
| Ctrl + N | New workbook |
| Ctrl + O | Open |
| Ctrl + S | Save |
| Ctrl + Shift + S | Save As |
| Ctrl + W | Close workbook |
| Ctrl + F4 | Close workbook (window) |
| Alt + F4 | Close Excel app (Windows) |
| Cmd + Q | Quit Excel (Mac) |
| Ctrl + P | Print |
| Shift + F11 | Insert new sheet |
| Ctrl + F1 | Show/Hide Ribbon |
| Alt + F11 | Macro Editor ([Spec 21](21-vba-macro.md)) |
| Alt + F8 | Macro list |
| F1 | Help |
| F12 | Save As dialog |

## Other quick

| Phím | Hành vi |
|---|---|
| Ctrl + Shift + L | Toggle AutoFilter |
| Ctrl + T | Create Table |
| Ctrl + E | Flash Fill |
| Ctrl + Q | Quick Analysis |
| Ctrl + L | Create Table (alternate) |
| Ctrl + K | Insert Hyperlink |

## Implementation note

- Mở rộng `shortcuts.DEFAULTS` trong `shortcuts.py` thêm các `cmd_id` còn thiếu.
- Mỗi cmd_id phải có label trong `i18n.STRINGS`.
- Settings → Customize Shortcuts dialog (đã có) cho phép user remap.
- Conflicts: nếu user remap trùng phím khác → cảnh báo và bỏ phím cũ.
- KeyTips (Alt → letter overlay trên ribbon): Phase sau (cần infra key handler tree).

## Acceptance criteria
1. F2 → Edit mode; F4 trong Edit toggle ref types.
2. Ctrl+1 → Format Cells dialog.
3. Alt+= với A1:A10 có số → A11 = `=SUM(A1:A10)`.
4. Ctrl+; → A1 = ngày hôm nay (`2026-06-02`).
5. Ctrl+Shift+$ → cell format Currency.
6. Ctrl+Spacebar → toàn cột active selected.
7. Settings → Customize Shortcuts → đổi Ctrl+B sang Ctrl+Alt+B → Ctrl+B không còn bold, Ctrl+Alt+B bold.
8. F5 → Go To dialog; gõ `Z100` → Enter → nhảy.

## Phụ thuộc
Tất cả spec khác (mỗi shortcut bind vào command).

## Risk
Thấp. Hạ tầng đã có; chỉ thêm cmd_id + binding.
