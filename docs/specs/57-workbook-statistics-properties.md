# SPEC 57 — Workbook Statistics / Properties / Document Inspector

## Mục tiêu
File properties + statistics + inspection: Review → Workbook Statistics, File → Info → Show All Properties, File → Info → Check for Issues (Document Inspector / Accessibility / Compatibility).

## Trạng thái hiện tại
- ✗ Chưa có (đề cập 1 phần trong [Spec 36 File Formats](36-file-formats-autosave.md) §36.4 và [Spec 41 Accessibility](41-accessibility.md), [Spec 51 Backstage](51-start-screen-backstage.md) §51.2 Info tab).

## 57.1 Workbook Statistics (Review tab — 2020+)

Review → **Workbook Statistics** button → dialog hiển thị summary:

```
┌─ Workbook Statistics ──────────────────────────────────┐
│ Current sheet:  Sheet1                                  │
│ ──────────────────────────────────────────────────────│
│ End of sheet:   AZ150                                   │
│ Words:          1,247                                   │
│ Number of cells with data: 387                          │
│ Number of tables: 2                                     │
│ Number of PivotTables: 1                                │
│ Number of formulas: 142                                 │
│ Number of charts: 3                                     │
│ Number of images: 1                                     │
│ Number of comments: 8                                   │
│ Number of notes: 0                                      │
│ Number of named ranges (sheet scope): 4                 │
│ ──────────────────────────────────────────────────────│
│ Workbook:                                               │
│ ──────────────────────────────────────────────────────│
│ Number of sheets: 5                                     │
│ Number of cells with data: 2,103                        │
│ Total words: 5,432                                      │
│ Number of tables: 7                                     │
│ Number of PivotTables: 3                                │
│ Number of formulas: 678                                 │
│ Number of charts: 9                                     │
│ Number of images: 4                                     │
│ Number of named ranges (workbook scope): 11             │
│                                                  [OK]   │
└─────────────────────────────────────────────────────────┘
```

### Use case
- Audit workbook complexity (vd "1000+ formulas" gợi ý refactor).
- Estimate file size factors.
- Pre-share check.

## 57.2 File → Info → Properties

Side panel right của Info tab ([Spec 51](51-start-screen-backstage.md) §51.2):

### Basic properties (always visible)
- Size (bytes / KB / MB)
- Title
- Tags
- Categories
- Author / Last modified by
- Created / Modified date
- Last printed
- Related Dates / Related People

### "Show All Properties" button → expand:
- Author (multi)
- Last modified by (multi)
- Manager
- Company
- Subject
- Hyperlink base
- Template
- Status (custom: Draft / Final / In Review)
- Comments (file-level, not cell)
- Custom properties (user-defined key-value)

### Edit
- Click giá trị → editable inline.
- "Add a tag" / "Add a category" prompt.

## 57.3 Advanced Properties dialog

Click "Properties ▼" dropdown → **Advanced Properties** → modal dialog với 5 tab:

### Tab General
- File type, location, size, created/modified/accessed.

### Tab Summary
- Title, Subject, Author, Manager, Company, Category, Keywords, Comments, Hyperlink base, Template, "Save preview picture" checkbox.

### Tab Statistics
- Created, Modified, Accessed, Last printed, Last saved by, Revision number, Total editing time.

### Tab Contents
- Document parts: sheet names list, named ranges, ...

### Tab Custom
- User-defined properties:
  - Name dropdown (suggested: Checked by, Client, Date completed, Department, Destination, Disposition, Division, Document number, Editor, Forward to, ...)
  - Type (Text / Date / Number / Yes or No)
  - Value
  - Add / Modify / Delete buttons.
- Custom properties hiển thị trong Show All Properties + Document Inspector.

## 57.4 Document Inspector (File → Info → Check for Issues → Inspect Document)

Modal dialog với checkbox list:
- **Comments and Annotations** — comments, notes
- **Document Properties and Personal Information** — author, company, etc.
- **Data Model** — embedded data model items
- **Content Add-ins / Task Pane Add-ins** — registered add-ins
- **Pivot Tables, PivotCharts, Cube Formulas, Slicers, Timelines** — analytics objects
- **Embedded Documents** — OLE objects
- **Macros, Forms, and ActiveX Controls** — code/controls
- **Links to Other Files** — workbook links ([Spec 53](53-workbook-links-external-refs.md))
- **Real-time Data Functions** — RTD() calls
- **Excel Surveys** — deprecated, just check
- **Defined Scenarios** — what-if
- **Active Filters** — currently applied filters
- **Custom Worksheet Properties** — sheet-level meta
- **Hidden Names** — system named ranges
- **Ink** — pen annotations
- **Hidden Rows and Columns** — non-default hidden
- **Hidden Worksheets** — Hidden sheets ([Spec 10](10-sheet-tabs.md))
- **Invisible Content** — objects format hidden

→ **Inspect** button → results list per checkbox với **Remove All** button cho từng category.

⚠ Remove là irreversible — warning + suggest save backup first.

## 57.5 Inquire add-in / Spreadsheet Compare (Win Enterprise)

**Inquire** = COM add-in mạnh hơn Document Inspector — chỉ có trong **Microsoft 365 Apps for Enterprise** Win (E3/E5/A3/A5 plans). Mac + Web KHÔNG có.

### Enable
File → Options → Add-ins → Manage: **COM Add-ins** → Go → ☑ **Inquire** → OK → ribbon tab **Inquire** xuất hiện.

### 6 tools trên Inquire tab

| Tool | Mô tả |
|---|---|
| **Workbook Analysis** | Generate report đầy đủ: count formulas / errors / external refs / linked workbooks / hidden sheets / array formulas / volatile / hidden names / ... — xuất ra workbook mới với mỗi category 1 sheet. |
| **Workbook Relationship** | Diagram đồ thị: workbook hiện tại → các workbook reference (external links) + nodes/edges. |
| **Worksheet Relationship** | Diagram intra-workbook: sheet nào reference sheet nào qua formula. |
| **Cell Relationship** | Diagram: 1 cell → các cells precedent/dependent (sâu hơn Trace Precedents [Spec 12](12-formula-system.md) — gồm cross-sheet, multi-level). |
| **Compare Files** | Diff 2 workbook side-by-side — color-coded cell differences (formula change / value change / format change / structure change). Sản phẩm tách: **Spreadsheet Compare 2024**. |
| **Clean Excess Cell Formatting** | Xóa formatting "ghost" ở cells không có data — giảm file size + recalc time. Excel hay accumulate format past UsedRange. |

### Spreadsheet Compare (standalone)
- Riêng app `SpreadsheetCompare.exe` cài cùng Office, Win-only Enterprise.
- Compare 2 files với granular diff (cell-level) + report HTML/Excel.

### Ezcel approach
- Phase 7+. Implement subset:
  - **Workbook Analysis**: dễ — walk model, count categories, produce report.
  - **Cell Relationship**: tận dụng dependency graph từ [Spec 12 Trace Precedents](12-formula-system.md).
  - **Compare Files**: open 2 workbook + diff cell-by-cell + color overlay. Phase 8+ medium effort.
  - **Clean Excess Cell Formatting**: walk used range, identify cells with format but no data + no neighbor data → strip _fmt entry.
- Workbook Relationship / Worksheet Relationship: graph-rendering qua `networkx` + `QGraphicsView`.

### Use case
- Pre-share workbook → remove sensitive personal info (author / comments / hidden sheets).
- Compliance.

## 57.5 Compatibility Checker (File → Info → Check for Issues → Check Compatibility)

Modal dialog kiểm tra workbook tương thích Excel cũ:
- Default check: All older Excel versions.
- Dropdown: select target versions (Excel 97-2003 / Excel 2010 / 2013 / 2016 / 2019 / 2021).
- Result list: features không tương thích.
  - Vd: "This workbook contains 5 dynamic array formulas. These will be converted to single-cell formulas in Excel 2019."
  - "Conditional formatting rule X is not supported in Excel 2010."
- "Copy to New Sheet" button → tạo sheet "Compatibility Report" với list issues.

### Use case
- Save workbook để share với user dùng Excel cũ.

## 57.6 Accessibility Checker

Xem [Spec 41 Accessibility](41-accessibility.md). File → Info → Check for Issues → **Check Accessibility** → mở Accessibility pane.

## Implementation note

### Workbook Statistics engine
- Iterate sheets + cells:
  - Count non-empty cells (`len(data)` after filter).
  - Count cells starting với `=` → formula count.
  - Count cells trong _tables → tables.
  - Count `_charts`, `_pivots`, `_shapes` lists.
  - Words: tokenize string cells split by whitespace.
- Cache result với invalidation on dataChanged.
- Hot path: nếu lớn (1M+ cells) → run async background.

### Document Inspector
- Mỗi checkbox = function `inspect_<category>(workbook) → list[Issue]`.
- Remove: function `remove_<category>(workbook, issues)`.
- Run in worker thread.

### Properties storage
- xlsx OOXML `docProps/core.xml` (Dublin Core) + `docProps/app.xml` (Office app props) + `docProps/custom.xml` (custom).
- openpyxl: `workbook.properties` (built-in) + `workbook.custom_doc_props` (custom).

## Acceptance criteria
1. Review → Workbook Statistics → dialog hiện đúng count cho current sheet và workbook.
2. File → Info → Properties side panel → click "Title" → editable → save → properties pane update.
3. Show All Properties → 10+ field xuất hiện thay vì 5.
4. Advanced Properties → Custom tab → Add "Project: Q1 Budget" → save → property lưu vào xlsx.
5. Reopen xlsx in Excel real → custom property "Project" còn nguyên.
6. Check for Issues → Inspect Document → check Comments + Hidden Worksheets → Inspect → result list.
7. Remove All Comments → workbook không còn comment.
8. Check Compatibility → target Excel 2010 → list features incompatible (vd dynamic arrays).

## Phụ thuộc
- [36 File Formats](36-file-formats-autosave.md) — properties storage.
- [41 Accessibility](41-accessibility.md) — Check Accessibility integration.
- [51 Backstage](51-start-screen-backstage.md) — Info tab.
- [53 Workbook Links](53-workbook-links-external-refs.md) — Links to Other Files inspector.

## Risk
Thấp. Mostly read + display. Inspector remove cần backup prompt + reversible undo plan.

## Phase
Phase 5+ (sau khi có format model + properties storage).
