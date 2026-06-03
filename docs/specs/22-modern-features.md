# SPEC 22 — Tính năng mới Excel 2024-2025

## Mục tiêu
Bù các tính năng mới Excel 365: Checkbox in cell, Regex functions, Focus Cell, TRIMRANGE, Navigation highlighting, Translate, Modernized Grid features. (Copilot / Python in Excel: chỉ khái niệm, không implement full.)

## Trạng thái hiện tại
- ✗ Toàn bộ chưa có.

## 22.1 Checkbox trong ô (§22.1)

- **Insert → Cell Controls → Checkbox** (Insert tab, KHÔNG phải Home tab). Trước 2024 có thể ở Home tab preview build — modern GA đã chuyển sang Insert.
- Giá trị: `TRUE` checked / `FALSE` unchecked.
- Toggle: click ô → checkbox toggle; Space key khi ô selected cũng toggle.
- Dùng trong formula: `=IF(A1, "Done", "Pending")`, `=COUNTIF(A1:A10, TRUE)`.
- Format: màu checkbox đổi qua Format Cells → Font Color.
- Khác Form Control Checkbox cũ: **cell-native**, không phải floating object.

### Implementation
- `_fmt[(r,c)]["checkbox"] = True` → render checkbox icon trong cell (vẽ trong delegate).
- Data: `TRUE` / `FALSE` string trong `_data`.
- Click delegate: detect click vào checkbox area → toggle.
- Space key listener khi mode = Ready, cell có checkbox → toggle.

## 22.2 Regex Functions (§22.2)

| Hàm | Cú pháp | Mô tả |
|---|---|---|
| REGEXTEST | `=REGEXTEST(text, pattern, [case_sensitivity])` | TRUE nếu match; `case_sensitivity`: 0=sensitive (default), 1=insensitive |
| REGEXEXTRACT | `=REGEXEXTRACT(text, pattern, [return_mode], [case_sensitivity])` | `return_mode`: 0=first match (default), 1=all matches (array), 2=capturing groups (array) |
| REGEXREPLACE | `=REGEXREPLACE(text, pattern, replacement, [occurrence], [case_sensitivity])` | `occurrence`: -1 hoặc bỏ qua = replace all, n>0 = thay instance thứ n |

### Examples
- `=REGEXEXTRACT(A1, "\d{3,4}")` → lấy số 3-4 chữ số đầu tiên.
- `=REGEXTEST(A1, "^[\w.]+@[\w]+\.[\w]{2,}$")` → validate email.
- `=REGEXREPLACE(A1, "\d", "*")` → mask số.

### Implementation
- Add 3 hàm vào `formula._FUNCTIONS`, dùng `re` của Python.
- `case_sensitive` default TRUE; FALSE → flag `re.IGNORECASE`.

## 22.3 Python in Excel (§22.3) — concept only

- `=PY(` trong cell → cell vào Python mode (viền xanh lá; icon 🐍).
- Có sẵn: pandas, numpy, matplotlib, sklearn, seaborn.
- Reference Excel data: `xl("A1:C10", headers=True)` → pandas DataFrame.
- Output modes: Python object (hiển thị trong cell) hoặc Excel value (số/text).
- Ctrl+Alt+Shift+M: đổi output type.

> Implementation phức tạp — gắn liền với macro/Python runtime ([Spec 21](21-vba-macro.md)). Phase rất sau.

## 22.4 Copilot trong Excel (§22.4) — concept only

- Yêu cầu API key LLM (Anthropic / OpenAI). Optional dependency.
- Mở: Home → AI Assist button → chat pane phải.
- Prompts: "Highlight sales > 1M", "Add a column for profit margin", "Create a PivotTable showing sales by region".
- Engine: LLM-based, tool calling vào internal API (similar to macro API).

> Phase rất sau. Architectural placeholder bây giờ: nút disabled với tooltip "Coming soon".

## 22.5 Focus Cell (§22.5)

- View → Focus Cell toggle.
- Highlight hàng + cột của active cell bằng màu nhạt.
- Không ảnh hưởng print.
- Setting lưu QSettings.

### Implementation
- Khi bật + active cell đổi: invalidate hàng + cột của active.
- Delegate paint: nếu cell ở cùng hàng hoặc cột với active → blend màu highlight.
- **Hot path** — cần invalidate diện rộng (1 hàng + 1 cột), nhưng chỉ vùng visible nên OK.

## 22.6 TRIMRANGE Function (§22.6)

`=TRIMRANGE(range, [trim_rows], [trim_cols])`

- Loại bỏ hàng/cột trống ở các cạnh của range.
- `trim_rows`: 0=none, 1=trailing, 2=leading, 3=both.
- `trim_cols`: 0=none, 1=trailing, 2=leading, 3=both.
- Thường kết hợp `SORT`, `FILTER`: `=SORT(TRIMRANGE(A1:Z1000))`.

### Implementation
- Add vào `formula._FUNCTIONS`.
- Return type: range (cần dynamic array — Phase 6 [Spec 12](12-formula-system.md)).

## 22.7 Navigation Highlighting (§22.7)

- Khi scroll hoặc navigate, brief highlight row/col header của active cell (~500ms fade).
- Giúp không bị mất vị trí khi scroll nhanh.

### Implementation
- Animation: `QPropertyAnimation` opacity trên row/col header item.
- Trigger: `currentChanged` signal.

## 22.8 Translate & Detect Language (§22.8)

| Hàm | Cú pháp |
|---|---|
| TRANSLATE | `=TRANSLATE(text, [source_lang], [target_lang])` |
| DETECTLANGUAGE | `=DETECTLANGUAGE(text)` → mã ngôn ngữ (`vi`, `en`) |

### Timeline
- **Announced June 2024** preview (Insiders Win + Mac).
- **GA gradual rollout** từ end-2024; tới **v2412 Current Channel (Jan 2025)** đã reach nhiều user.
- M365 home + business + enterprise license đều bao gồm; không cần API key riêng.

### Implementation
- Excel chính thức: dùng **Microsoft Translator** built-in (không cần API key, gói chung với M365 license; sender không thấy cost — Microsoft cover).
- Ezcel implement: cần internet + provider (Microsoft Translator API có free tier 2M chars/tháng / Google Translate API / DeepL / Anthropic). Optional setup trong Settings.
- Nếu offline → `#N/A`.
- Cache theo `(text, source, target)` để giảm call.

## 22.9 Modernized Grid (§22.9) — Web Excel only

- `+` button khi hover header để add row/col.
- Filter Comments theo trạng thái / author / date.

> Ezcel là desktop — feature này map sang:
> - Hover row/col header → button `+` xuất hiện ở edge (Insert).

## 22.A `=IMAGE()` function (Aug 2022 GA, M365 + Web)

Insert image vào cell trực tiếp từ URL — image follow cell size, sort/filter cùng với cell.

### Syntax
`=IMAGE(source, [alt_text], [sizing], [height], [width])`

| Arg | Mô tả |
|---|---|
| `source` | URL (HTTPS bắt buộc), tới file BMP/JPG/GIF/TIFF/PNG/ICO/WEBP |
| `alt_text` | Optional. Text accessibility cho screen reader |
| `sizing` | 0 = fit cell + giữ aspect (default), 1 = fill cell ignore aspect, 2 = giữ original size (overflow), 3 = custom với height + width |
| `height` | Px khi `sizing=3` |
| `width` | Px khi `sizing=3` |

### Examples
- `=IMAGE("https://example.com/logo.png")` — fit cell.
- `=IMAGE("https://...pic.jpg", "Product photo", 1)` — fill ignore aspect.
- `=IMAGE("https://...icon.png", , 3, 32, 32)` — exact 32×32.

### Properties
- HTTPS only (HTTP fails).
- WEBP **không support** trên Web + Android (chỉ Win/Mac).
- Cache: image lưu trong workbook lần đầu fetch; source change sau đó KHÔNG auto re-fetch (trừ khi user remove rồi nhập lại formula).
- Async fetch: cell `#BUSY!` trong khi load.
- Sort/Filter: image follow row khi sort.
- Print: in được.

### Khác `Insert Picture`
- IMAGE = cell-bound (cell-native), không phải floating shape ([Spec 34](34-shapes-images-smartart.md)).
- Update theo formula → có thể `=IMAGE(IF(A1>100, "good.png", "bad.png"))`.

### Ezcel implementation
- Add vào `formula._FUNCTIONS`.
- Async fetch via `urllib` (stdlib); cache `QPixmap` per URL hash trong workbook.
- Delegate paint: nếu cell value là `IMAGE_TOKEN(url, sizing)` → draw pixmap theo sizing rule.

## 22.B 14 Dynamic-Array / Text functions (Excel 365 + Excel 2024)

Microsoft thêm bộ 14 hàm dynamic-array + text từ 2022-2024 (M365 GA, cũng có trong Excel 2024 perpetual frozen tại Oct 2024). Đầy đủ:

### Text manipulation (3)

| Hàm | Cú pháp | Mô tả |
|---|---|---|
| TEXTSPLIT | `=TEXTSPLIT(text, col_delim, [row_delim], [ignore_empty], [match_mode], [pad_with])` | Split text thành rows/cols theo delimiter; có thể split 2D |
| TEXTBEFORE | `=TEXTBEFORE(text, delim, [instance], [match_mode], [match_end], [if_not_found])` | Text trước delimiter (instance 1+, -1 = từ cuối) |
| TEXTAFTER | `=TEXTAFTER(text, delim, [instance], [match_mode], [match_end], [if_not_found])` | Text sau delimiter |

### Array stack (2)

| Hàm | Mô tả |
|---|---|
| VSTACK | Concat vertical mảng các array (`=VSTACK(A1:B3, D1:E5)`) — auto-grow khi source tables expand |
| HSTACK | Concat horizontal |

### Array reshape (4)

| Hàm | Mô tả |
|---|---|
| TOROW | `=TOROW(array, [ignore], [scan_by_col])` — flatten 2D → 1 row |
| TOCOL | Flatten 2D → 1 col |
| WRAPROWS | `=WRAPROWS(vector, wrap_count, [pad_with])` — 1D → 2D wrap theo N cols |
| WRAPCOLS | 1D → 2D wrap theo N rows |

### Array select (4)

| Hàm | Mô tả |
|---|---|
| TAKE | `=TAKE(array, rows, [cols])` — lấy N rows/cols từ đầu (rows<0 lấy cuối) |
| DROP | `=DROP(array, rows, [cols])` — bỏ N rows/cols |
| CHOOSEROWS | `=CHOOSEROWS(array, row_num1, ...)` — pick rows theo index 1-based (-1 = từ cuối) |
| CHOOSECOLS | Pick cols |

### Array expand (1)

| Hàm | Mô tả |
|---|---|
| EXPAND | `=EXPAND(array, rows, [cols], [pad_with])` — pad array tới size mới |

### Use cases combo
- `=VSTACK(Region1[#All], Region2[#All], Region3[#All])` → union 3 Tables.
- `=SORT(TEXTSPLIT(A1, ",", ";"))` → split CSV string 2D rồi sort.
- `=CHOOSEROWS(SORT(FILTER(data, data[Region]="North")), 5)` → top 5 sorted filtered.
- `=WRAPROWS(SEQUENCE(20), 5)` → 20 numbers → 4×5 grid.

### Implementation
- Add 14 hàm vào `formula._FUNCTIONS`.
- Cần dynamic array / spill ([Spec 12 Phase 6](12-formula-system.md)) — tất cả return array nên block trên Phase < 6.

## 22.C 7 LAMBDA Helper functions

Excel 365 + 2024:

| Hàm | Cú pháp | Mô tả |
|---|---|---|
| MAP | `=MAP(arr1, [arr2…], lambda)` | Apply lambda từng element, return same-shape array |
| REDUCE | `=REDUCE(init, arr, lambda(acc, val))` | Fold array → scalar |
| SCAN | `=SCAN(init, arr, lambda(acc, val))` | Running fold → array same shape |
| MAKEARRAY | `=MAKEARRAY(rows, cols, lambda(r,c))` | Generate array từ scratch |
| BYROW | `=BYROW(arr, lambda(row))` | Apply lambda từng row → 1-col array |
| BYCOL | `=BYCOL(arr, lambda(col))` | Apply lambda từng col → 1-row array |
| ISOMITTED | `=ISOMITTED(arg)` | TRUE nếu arg trong LAMBDA bị omit |

### Examples
- `=BYROW(A1:C10, LAMBDA(r, SUM(r)))` → 10×1 row sums.
- `=REDUCE(0, A1:A100, LAMBDA(acc, x, IF(x>0, acc+x, acc)))` → sum positives.
- `=SCAN(0, A1:A10, LAMBDA(acc, x, acc+x))` → running total.
- `=MAKEARRAY(10, 10, LAMBDA(r, c, r*10+c))` → multiplication-like grid.
- `=GROUPBY(A:A, B:B, LAMBDA(v, TEXTJOIN(",", TRUE, v)))` → custom aggregation trong GROUPBY ([Spec 18](18-pivot-table.md)).

### Implementation
- Cần engine support LAMBDA as first-class value ([Spec 12](12-formula-system.md) §LAMBDA + custom functions).
- Helper functions evaluate lambda call N times — careful về performance (cache lambda compile).

## 22.10 IMPORTTEXT / IMPORTCSV (Jan 2026 Insiders Win)

Hai hàm import file thành **refreshable dynamic array** chỉ qua 1 formula.

| Hàm | Cú pháp | Mô tả |
|---|---|---|
| IMPORTCSV | `=IMPORTCSV(path_or_url, [delimiter], [encoding], [skip_rows], [headers])` | Load CSV → 2D dynamic array; refresh qua Data → Refresh All hoặc Ctrl+Alt+F5 |
| IMPORTTEXT | `=IMPORTTEXT(path_or_url, [delimiter], [encoding], [skip_rows], [headers])` | Tương tự IMPORTCSV nhưng cho text file generic (TXT/TSV/pipe-delimited) |

### Examples
- `=IMPORTCSV("https://example.com/data.csv")` → spill toàn file.
- `=IMPORTCSV("C:\reports\sales.csv", ",", "utf-8", 1, TRUE)` → skip header row + treat first row as headers.
- `=IMPORTTEXT("data.tsv", CHAR(9))` → tab-delimited.

### Properties
- Cell hiện `#BUSY!` while fetching async (cùng pattern với `=COPILOT()`).
- Dependency: nguồn change → next Refresh All → re-fetch.
- KHÁC `=WEBSERVICE()` (deprecated) hoặc Power Query: nhẹ hơn, 1 formula thay vì query.
- Thay thế use case nhỏ cho [Spec 20 Power Query](20-power-query.md) khi user chỉ cần CSV đơn giản.

### Implementation Ezcel
- Add 2 hàm vào `formula._FUNCTIONS`.
- File path: relative resolve theo workbook path; URL: `urllib` (stdlib, không thêm `requests`).
- Background thread → trả `#BUSY!` placeholder → set spill array khi xong.
- Refresh hook tích hợp `Refresh All` mechanism của Power Query [Spec 20].

## Acceptance criteria
1. Insert → Checkbox vào A1 → ô có checkbox; click toggle TRUE/FALSE; Space cũng toggle.
2. `=COUNTIF(A1:A10, TRUE)` đếm số ô checked đúng.
3. `=REGEXEXTRACT("abc123def", "\d+")` → `123`.
4. View → Focus Cell bật → click D5 → hàng 5 + cột D có background nhạt.
5. Scroll xa cell D5 rồi quay lại → khi D5 visible, row header 5 + col header D flash highlight 500ms.
6. `=TRIMRANGE(A1:Z1000)` → range thực sự không có trailing empty rows/cols.
7. `=TRANSLATE("Xin chào", "vi", "en")` → `"Hello"` (cần API key).
8. `=IMPORTCSV("sales.csv")` → spill dynamic array; sửa file rồi Ctrl+Alt+F5 → re-fetch.
9. `=IMPORTTEXT("data.tsv", CHAR(9))` → tab-delimited split đúng.

## Phụ thuộc
- [12 Formula System](12-formula-system.md) — register REGEX/TRIMRANGE/TRANSLATE.
- [21 VBA / Macro](21-vba-macro.md) — Python in Excel + Copilot tool calling.
- [11 Status Bar](11-status-bar.md) — View Focus Cell toggle.

## Risk
- Checkbox cell-native cần thay đổi delegate hit-testing.
- Focus Cell + Navigation highlight: animation trên hot path; cẩn thận frame drop.
- TRANSLATE: external API → cần fallback graceful khi offline.
