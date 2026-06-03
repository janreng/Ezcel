# PLAN — Ezcel tiến gần Microsoft Excel

> Master plan đồng bộ với `Excel_UX_Spec_Clone_Guide.docx` (Microsoft 365 / 2021 / 2024) **+ research mở rộng tới Excel 2024-2025**. Lấy spec làm chuẩn UX; mỗi tính năng có file riêng trong `docs/specs/`.

## Nguyên tắc

- **Theo Excel bản hiện đại (Microsoft 365 / 2024-2025).** Tính năng legacy (VBA, Notes cũ, Form Controls cũ, ActiveX, BIFF .xls write) chỉ giữ làm tham khảo compat — KHÔNG ưu tiên implement.
- **Spec là chuẩn UX, không phải lịch giao.** Tính năng nào đã có một phần thì spec ghi rõ "đã có" / "thiếu" / "khác Excel".
- **Phase chia theo phụ thuộc kỹ thuật**, không theo độ "đẹp". Sửa cấu trúc dữ liệu trước, UI sau, file format sau cùng.
- **Mỗi spec phải có acceptance criteria đo được** (gõ phím gì → thấy gì). Không acceptance criteria → không thể gọi là xong.
- **Tôn trọng ranh giới module** đã định trong `CLAUDE.md` (model / view / main_window / formula / io_utils).
- **Hot path** (paint, data(), selectionChanged) cấm cấp phát thừa — xem `docs/plans/2026-06-02-performance-optimization.md`.

## Trạng thái Ezcel hiện tại (chốt ngày 2026-06-02)

| Mảng | Ezcel | Excel | Spec |
|---|---|---|---|
| Grid + virtual scroll | ✓ (QTableView) | ✓ | [01](01-grid-engine.md) |
| Cell addressing A1:XFD | một phần (giới hạn theo Qt model) | ✓ | [02](02-cell-system.md) |
| 4 cell modes (Ready/Enter/Edit/Point) | **không có Point/Enter chuẩn**, Edit/Ready có ngầm | ✓ | [03](03-cell-modes.md) |
| Name Box + Formula Bar | có cơ bản (Formula Bar, không Name Box drop-down, không Function Wizard) | ✓ | [04](04-name-box-formula-bar.md) |
| AutoFill / AutoComplete / Flash Fill | AutoFill có, AutoComplete chưa, Flash Fill chưa | ✓ | [05](05-data-entry-autofill.md) |
| Context menus (cell/row/col/sheet) | cell có, row/col/sheet ít | ✓ | [06](06-context-menus.md) |
| Ribbon | có (Home-ish), thiếu nhiều tab + contextual | ✓ | [07](07-ribbon.md) |
| Format Cells dialog 6 tab | thiếu (chỉ ribbon button rời) | ✓ | [08](08-format-cells-dialog.md) |
| Row/Col/Sheet operations | resize/insert/delete/hide có; group/outline chưa | ✓ | [09](09-row-col-operations.md) |
| Sheet tabs (multi sheet) | **chưa** | ✓ | [10](10-sheet-tabs.md) |
| Status Bar | có Sum/Avg/Count cơ bản, thiếu Cell Mode, View buttons | ✓ | [11](11-status-bar.md) |
| Formula engine | ~120 hàm (v0.12.0), chưa có Function Wizard / Error smart tag / Trace | ✓ | [12](12-formula-system.md) |
| Clipboard / Paste Special | Ctrl+C/X/V có, Paste Special chỉ "values" partial | ✓ | [13](13-clipboard-paste-special.md) |
| Freeze / Split / Views | Freeze có; Split/Page Layout/Page Break chưa | ✓ | [14](14-freeze-split-views.md) |
| Filter / Sort | có | ✓ | [15](15-filter-sort.md) |
| Table (Ctrl+T) | chưa | ✓ | [16](16-table.md) |
| Conditional Formatting | chưa (đã có chỗ trong undo snapshot) | ✓ | [17](17-conditional-formatting.md) |
| PivotTable | chưa | ✓ | [18](18-pivot-table.md) |
| Chart | chưa | ✓ | [19](19-chart.md) |
| Power Query | chưa | ✓ | [20](20-power-query.md) |
| VBA / Macro | chưa | ✓ | [21](21-vba-macro.md) |
| Modern (Checkbox / Regex / Python / Copilot / Focus Cell) | chưa | ✓ | [22](22-modern-features.md) |
| Keyboard shortcuts | một phần | ✓ | [23](23-keyboard-shortcuts.md) |

> Bảng trạng thái trên mới liệt kê spec 01–23. Repo có **51 spec** (24–51:
> print, validation, comments, data-tools, what-if, protection, themes, named
> ranges, sparklines, file-formats, copilot, ... đến start-screen) — xem
> `README.md` và các Phase 6–7 bên dưới. Rà soát chất lượng: `../SPEC_AUDIT.md`.

## Phase roadmap

### Phase 1 — Core UX gap-close (1-2 tuần)
Mục tiêu: hành xử đúng *kiểu Excel* với những gì đã có. **Không** thêm tính năng lớn — sửa hành vi cho khớp spec.

- [03 Cell Modes](03-cell-modes.md) — implement state machine Ready/Enter/Edit/Point + Status Bar indicator
- [04 Name Box + Formula Bar](04-name-box-formula-bar.md) — Name Box navigate (gõ A1 → nhảy), dropdown Named Ranges, expand button, Confirm/Cancel button
- [11 Status Bar](11-status-bar.md) — Cell Mode label, right-click customize, View buttons + Zoom slider chuẩn
- [02 Cell System](02-cell-system.md) — multi-range selection (Ctrl+Click), Name Box hiểu `A1:B3,D5,F1:F10`
- [23 Keyboard Shortcuts](23-keyboard-shortcuts.md) — bù bảng phím tắt thiếu (Ctrl+Shift+arrow đã có; bổ sung F2/F5/Shift+F3/Ctrl+`/Alt+=)

### Phase 2 — Format Cells dialog + định dạng nâng cao (2-3 tuần)
Đã có roadmap cũ phase 2 — gộp + cập nhật theo spec mới.

- [08 Format Cells Dialog](08-format-cells-dialog.md) — dialog 6 tab (Number / Alignment / Font / Border / Fill / Protection)
- [13 Clipboard & Paste Special](13-clipboard-paste-special.md) — Paste Special dialog đầy đủ (Values / Formulas / Formats / Transpose / Skip Blanks / Add/Sub/Mul/Div)
- [09 Row/Col Operations](09-row-col-operations.md) — Group/Outline, Hide/Unhide đầy đủ, AutoFit cột/hàng
- [06 Context Menus](06-context-menus.md) — Mini Toolbar floating + menu chuẩn cho row/col/sheet header

### Phase 3 — Multi-sheet + Tables (3-4 tuần)
Refactor kiến trúc + thêm 2 container.

- [10 Sheet Tabs](10-sheet-tabs.md) — `Workbook` class, tab bar, tham chiếu `Sheet1!A1` trong formula
- [16 Table (Ctrl+T)](16-table.md) — Table container + Structured References + Total Row
- [14 Freeze / Split / Views](14-freeze-split-views.md) — Split view, Page Layout / Page Break Preview, Multiple Windows

### Phase 4 — Formula UX cao cấp + Conditional Formatting (2-3 tuần)

- [12 Formula System (UX)](12-formula-system.md) — Function Wizard (Shift+F3), autocomplete dropdown, ScreenTip, error smart tag, Trace Precedents/Dependents, Evaluate Formula
- [17 Conditional Formatting](17-conditional-formatting.md) — 5 loại preset + custom formula rule + Manage Rules + Stop If True priority
- [05 AutoFill / AutoComplete / Flash Fill](05-data-entry-autofill.md) — AutoComplete cột text, Flash Fill Ctrl+E, Pick From Dropdown List

### Phase 5 — Filter / Sort nâng cao + Modern features + Data Tools (2-3 tuần)

- [15 Filter / Sort](15-filter-sort.md) — multi-level sort dialog, Text/Number/Date filter sub-menu, Sort by Color
- [22 Modern features](22-modern-features.md) — Checkbox in cell, REGEX functions, Focus Cell, TRIMRANGE
- [26 Comments & Notes](26-comments-notes.md) — Threaded Comments (priority); legacy Notes minimal
- [27 Data Tools](27-data-tools.md) — Text to Columns, Remove Duplicates, Subtotal, Consolidate, Forecast Sheet
- [30 Themes & Cell Styles](30-themes-cell-styles.md) — Theme cascade + Cell Styles gallery + Custom Lists
- [33 Sparklines](33-sparklines.md) — Line / Column / Win-Loss
- [36 File Formats / AutoSave / AutoRecover](36-file-formats-autosave.md)
- [40 Quick Analysis](40-quick-analysis.md) — Ctrl+Q + Recommended Charts/PivotTables

### Phase 6 — Reporting & analytics + AI (sau cùng)
Phụ thuộc đa số module trước.

- [18 PivotTable](18-pivot-table.md)
- [19 Chart](19-chart.md)
- [20 Power Query](20-power-query.md)
- [21 Macro (Python)](21-vba-macro.md) — không VBA, dùng Python
- [28 What-If Analysis](28-what-if-analysis.md) — Goal Seek / Scenario / Solver / Data Table
- [29 Protection](29-protection.md) — Sheet / Workbook / Encryption
- [38 Linked Data Types](38-linked-data-types.md) — Stocks / Geography / Currency (modern Excel 365)
- [39 Copilot / Agent Mode](39-copilot-agent.md) — AI integration (NEW 2024-2025)

### Phase 7 — Ribbon hoàn chỉnh + In ấn + Objects (parallel với Phase 6)

- [07 Ribbon](07-ribbon.md) — full tab Home / Insert / Page Layout / Formulas / Data / Review / View + Contextual tabs (Table Design, Chart Design, PivotTable Analyze)
- [24 Print & Page Setup & Export PDF](24-print-page-setup.md)
- [34 Shapes / Images / SmartArt / Hyperlinks](34-shapes-images-smartart.md)
- [37 Form Controls](37-form-controls.md) — legacy compat, deprioritize

---

## Cách dùng plan này

1. Đọc spec của tính năng cần làm — spec là **single source of truth** cho UX.
2. Spec chỉ rõ "đã có" / "thiếu" / "khác Excel"; chỉ làm phần "thiếu" + "khác Excel".
3. Mỗi PR commit một acceptance criterion (hoặc gom nhỏ). Reference spec line trong commit message.
4. Khi spec lệch với hành vi Excel thật → cập nhật spec, không sửa code chệch.
5. Tính năng ngoài phạm vi 7 phase trên: tạo spec riêng khi cần.

## Cập nhật so với research 2024-2026

- **Copilot in Excel** ([Spec 39](39-copilot-agent.md)) — Agent Mode (GA Dec 2025 Web + Win, T1/2026 Mac), COPILOT function trong formula, Formula Completion AI, **Edit with Copilot GA March 2026**, **Python in Edit with Copilot April 2026**, Claude Opus 4.6/4.7 + GPT-5.5 model selector, **May 2026 Copilot entry consolidation + ribbon restore + =COPILOT web search**.
- **Linked Data Types value tokens** ([Spec 38](38-linked-data-types.md)) — Formula Bar phân biệt linked vs text (mới 2024); **Wolfram types đã gỡ 06/2023**; Refinitiv → LSEG.
- **PIVOTBY / GROUPBY** functions — native (đã có trong [Spec 12](12-formula-system.md) nhóm dynamic array).
- **Python in Excel** — đã có ở [Spec 22](22-modern-features.md) (`=PY(`); phân biệt với Python Macro local [Spec 21](21-vba-macro.md).
- **Focus Cell, Navigation Highlighting** — [Spec 22](22-modern-features.md).
- **Modernized Get Data dialog** — [Spec 20](20-power-query.md).
- **3D Maps RETIRED** bởi Microsoft 2024-2025 — [Spec 46](46-3d-maps.md) đánh dấu out of scope vĩnh viễn.
- **Office Scripts** chạy Win/Mac/Web từ 2022 (không chỉ web) — [Spec 43](43-office-scripts.md) sửa premise; March 2026 UI redesign.
- **Comments pane redesign** April 2026 (mobile-first, filter, bulk, @mention) — [Spec 26](26-comments-notes.md).
- **Aptos Narrow** thay Calibri làm default cell font 2023 — [Spec 50](50-design-system.md).
- **Edit data label text in Excel for the web** — out of scope desktop.

## Research wave Iter 14 (Jun 2026) — new findings + revised specs

Nguồn: Microsoft Tech Community (Excel blog Jan-May 2026), MS365 Copilot blog, Microsoft 365 Roadmap, MS Learn Office release notes, neowin.net, geeky-gadgets.com, releasebot.io, blog.imseankim.com, office-watch.com, ICAEW.

### Spec 39 Copilot — bổ sung
- **Agent Mode multi-model switcher GA** (Apr 22 2026): UI cho user chọn OpenAI vs **Anthropic Claude** trực tiếp trong pane. Engagement +67%, retention +50%, satisfaction +65%.
- **§39.3.1**: single-prompt multi-tab workbook generation + validation loops.
- **§39.3.2 Chat/Edit switcher + Plan with Copilot + Show Changes one-turn indicator + Understand Changes diff panel** (April).
- **§39.3.3 Python in Edit with Copilot + Think Deeper mode** (April, Win/Mac/Web).
- **§39.3.4 Federated Copilot connectors**: LSEG + Moody's GA Excel May; HubSpot/Notion/Canva/Linear chưa GA Excel.

### Spec 22 Modern features — bổ sung
- **§22.10 IMPORTTEXT / IMPORTCSV** (Jan 2026 Insiders Win): refreshable dynamic array import; `#BUSY!` async; thay use case nhỏ cho Power Query.

### Spec 20 Power Query — bổ sung
- **§20.4 Modern Get Data dialog** (Preview May 2026): unified search + dark mode + a11y keyboard-first + cross-platform consistency Fabric/PowerBI/Excel + Microsoft Dataverse tables tiered list.

### Spec 21 VBA/Python Macro — bổ sung
- **§21.7 Python in Edit with Copilot**: tận dụng macro runtime làm sandbox cho Copilot Python execution; Think Deeper dedicated worksheet pattern.

### Spec 27 Data Tools — bổ sung
- **§27.10 Clean Data button (AI Copilot, April 2026)**: detect 7 issue types (whitespace / capitalization / date format / number format / duplicates / missing / outliers); per-card preview + apply; overlap với Analyze Data (Clean = fix, Analyze = insight).

### Spec 19 Chart — bổ sung
- **§19.5 Editable chart label text on Web** (May 2026): parity với desktop; Ezcel desktop đã có sẵn, note compat.

### Spec 44 Collaboration — bổ sung
- **§44.5 Show Changes extended history** (May 2026 Web): 30d → 1 year retention + Copilot attribution indicator card.

### Spec 35 Calc Engine — bổ sung
- **§35.A Descriptive error cards** (Jan 2026 Win Insiders): hover error → popup với probable cause + suggested fix button (e.g. `SUMM` → SUM); matrix cho 8 error codes.

### Spec 30 Themes — bổ sung
- Default theme bundle 2024 (Aptos): **new color palette với WCAG AA contrast** + **thicker shape outlines mặc định** 0.75pt → 1.5pt + refreshed 14 chart styles.

### Misc findings (chưa map spec riêng, đã note PLAN)
- Microsoft Forms ↔ Excel desktop integration GA Win Current Channel (Nov 2024+), Mac coming soon — out of MVP scope; nếu cần thêm vào Spec 22 hoặc Spec 27 sau.
- Edit with Copilot extended language support 50+ (March 2026).
- Excel for Mac Forms real-time sync (May 2026 specific).
- "Show Changes" web 1-year retention (May 2026).

## Research wave Iter 15 (Jun 2026) — deeper Microsoft Learn + Support coverage

### Spec 22 Modern features — bổ sung
- **§22.A `=IMAGE()` function** (Aug 2022 GA): URL → cell-bound image, sizing 0/1/2/3 + height/width, HTTPS only, WEBP not on Web/Android, async `#BUSY!`. Khác Insert Picture (cell-native vs floating shape).
- **§22.B 14 dynamic-array / text functions** đầy đủ catalog (M365 + Excel 2024 perpetual): TEXTSPLIT, TEXTBEFORE, TEXTAFTER, VSTACK, HSTACK, TOROW, TOCOL, WRAPROWS, WRAPCOLS, TAKE, DROP, CHOOSEROWS, CHOOSECOLS, EXPAND. Use case combo.
- **§22.C 7 LAMBDA helpers**: MAP, REDUCE, SCAN, MAKEARRAY, BYROW, BYCOL, ISOMITTED. Cần engine treat LAMBDA as first-class value.
- **§22.8 TRANSLATE/DETECTLANGUAGE** — clarify GA timeline (June 2024 preview → v2412 Jan 2025 Current Channel GA cho nhiều users); Microsoft cover cost qua M365 license (không cần API key user-side).

### Spec 39 Copilot — bổ sung
- **§39.4.1 Natural Language Formulas** (May 2026 GA M365): chip inline "Ask Copilot for a formula ✨" khi gõ `=`; popover textbox prompt → generate. "Modify formula" cho existing formula.
- **§39.5 Recommended PivotTables Copilot-powered** (2026): Business/Enterprise — pane Copilot suggestions thay dialog cũ; mỗi suggestion có mini preview + why + 1-click apply; auto-flag trends/outliers/correlation. Non-Copilot vẫn dùng dialog cũ.

### Spec 20 Power Query — bổ sung
- **§20.5 Power Query for Web GA 2025**: full editor + import wizard + M Advanced Editor + refresh authenticated sources (SQL/SharePoint/OData/OneDrive). Local file source: chưa GA, trên roadmap. Subscriber gating: view+refresh = all subs, full editor = Business/Enterprise.

### Spec 18 PivotTable — bổ sung
- Note Copilot-powered Recommended PivotTables replacing dialog cũ for Business/Enterprise; fallback dialog cho non-Copilot + Excel 2024 perpetual.

### Spec 44 Collaboration — bổ sung
- **§44.A Merge conflict resolution** (AutoSave): Resolve button → Conflicts tab per-conflict accept/reject; TemporaryBackupFile (Win: `%localappdata%/Microsoft/Excel/TemporaryBackupFile/`, Mac: `~/Library/Containers/.../TemporaryBackupFile/`) lưu 7 ngày.
- **§44.B Loop components ↔ Excel** (Feb 2026 Teams Collaborative Notes rollout): Loop component hierarchy (Component → Page → Workspace); Send Table to Loop / Embed Loop Table in Excel; Teams meeting notes = Loop sync với Excel.

### Misc finding wave 15
- **Excel 2024 perpetual vs M365**: Excel 2024 frozen Oct 2024 feature set, security only until Oct 2029. **Không có**: Copilot, GROUPBY/PIVOTBY, Python in Excel, =COPILOT, 14 new dynamic array fns (TEXTSPLIT/VSTACK/...), Natural Language Formulas, Federated Connectors. **M365 only**.
- **Analyze Data (free native AI)**: Excel native non-Copilot Insights — free cho all M365 subscribers, không cần Copilot license. 64% Insiders survey nói Analyze Data đủ dùng (không upgrade Copilot).
- **=COPILOT() function as natural language interface**: type plain English thay vì formula syntax; gradient between "ghost text suggestion" (§39.4) vs "explicit chip" (§39.4.1).
- **Format Painter copies CF rules** — vanilla Excel behavior, double-click → multi-paste mode.

### Compat notes added
- Specs nên distinguish "M365 only" vs "available in Excel 2024 perpetual": dynamic arrays cơ bản (FILTER/SORT/UNIQUE/XLOOKUP) **có** trong Excel 2024; 14 new functions (TEXTSPLIT/VSTACK/...) + LAMBDA helpers **có** trong 2024; Copilot / GROUPBY-PIVOTBY / Python in Excel **không có** trong 2024 perpetual.

## Research wave Iter 16 (Jun 2026) — economics + add-ins + UX nits

### Spec 26 Comments — bổ sung
- **5-sided purple pentagon shape** visual marker chính thức (thay vì "purple bubble" cũ).
- **3-dot menu** hover comment: Edit / Delete Thread / Resolve. Resolve **preserve** data (hide pentagon, filter "Resolved" in pane).
- Edit history: timestamp giữ original + "(edited HH:MM)" suffix, không full version history (vs Google Docs).

### Spec 35 Calc Engine — bổ sung
- **§35.B MTR detailed config**: Excel hỗ trợ **1-1024 threads** (không phụ thuộc CPU core count); Excel có thể oversubscribe.
- Engine tách thread-safe vs non-thread-safe cells (volatile NOW()/RAND() trong contexts, COM UDF, etc).
- Ezcel: threading + GIL release qua C ext, hoặc multiprocessing pool. Option UI 1-1024 default `os.cpu_count()`.

### Spec 39 Copilot — bổ sung
- **§39.6 Economics**: Nov 2026 unified Copilot Credits model — AI Builder seeded credits removed; tất cả AI billing qua Copilot Credits (~0.5 credit/4200 tokens).
- `=COPILOT()` limits: 100 calls/10min Frontier preview, 8k input + 4k output token typical.
- Ezcel bring-your-own-key model tránh subscription overhead.

### Spec 45 Power Pivot — bổ sung
- **§45.6 CUBE functions**: 7 hàm CUBEVALUE / CUBEMEMBER / CUBESET / CUBESETCOUNT / CUBERANKEDMEMBER / CUBEKPIMEMBER / CUBEMEMBERPROPERTY. Cần Power Pivot Data Model hoặc external OLAP cube (SSAS).
- **Convert PivotTable → CUBE Formulas**: PivotTable Analyze → OLAP Tools → freeform dashboard cell layout.

### Spec 50 Design System — bổ sung
- **§50.0 Dark mode platform parity matrix**:
  - Win 365 **full dark mode (cells + grid) Mar 2026** Insiders → Current.
  - Mac chỉ ribbon + menus dark, cells light (auto follow system).
  - Web + Mobile chỉ chrome dark.
- Ezcel desktop nên implement full dark grid để parity Win modern (tốt hơn Mac).

### Spec 56 Sheet View — bổ sung
- **Known bug** (vẫn open Sep 2025): user thứ 2 tạo Sheet View → default view của workbook bị thay đổi cho everyone. Ezcel implementation phải tránh.
- **§56.2.2 Copy Link to Sheet View** + URL `#viewid=<guid>`.
- "See mine / See others" prompt mỗi session, no persistent preference (chưa fix).

### Spec 57 Workbook Stats — bổ sung
- **§57.5 Inquire add-in / Spreadsheet Compare** (Win Enterprise E3/E5 only): 6 tools — Workbook Analysis / Workbook Relationship / Worksheet Relationship / Cell Relationship / Compare Files / Clean Excess Cell Formatting.
- Spreadsheet Compare standalone exe Win-only Enterprise.
- Ezcel approach: subset (Analysis + Cell Relationship + Compare Files + Clean Excess Formatting) Phase 7-8.

### Misc finding wave 16
- **Find & Replace native regex KHÔNG support** — chỉ wildcards `?` `*` `~`; regex thực thông qua VBA hoặc REGEX functions (§22.2).
- **Excel for Mac Touch Bar** support (legacy MacBook Pro hardware) — quick formatting controls; deprecated trên MacBook Pro M-series sau Apple bỏ Touch Bar.
- **6 modern chart types** confirmed (Excel 2016+ vẫn current): Waterfall, Histogram (Pareto), Box & Whisker, Treemap, Sunburst, Funnel. Spec 19 đã list.
- **Outlook ↔ Excel sync** chính chủ Microsoft chỉ qua **Power Automate** flow (Create Event V2) — không có in-app direct integration.

## Tham chiếu

- Doc gốc: `Excel_UX_Spec_Clone_Guide.docx` (bản extract: `bot-server-dedicated/.claude/channels/discord/inbox/spec_extracted.txt`)
- Web research 2024-2026: techcommunity.microsoft.com (What's New in Excel monthly), microsoft.com/microsoft-365/blog (Copilot April 22 GA), learn.microsoft.com/officeupdates, microsoft.com/microsoft-365/roadmap, howtogeek.com, neowin.net, windowsreport.com, geeky-gadgets.com, releasebot.io, blog.imseankim.com, office-watch.com, ICAEW Excel community
- Roadmap cũ: `docs/roadmap.md`
- Performance plan: `docs/plans/2026-06-02-performance-optimization.md`
- CLAUDE.md (architecture & conventions): repo root
