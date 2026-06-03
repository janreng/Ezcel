# SPEC 53 — Workbook Links / External References (modernized pane 2024)

## Mục tiêu
Manage external workbook references (data từ file `.xlsx` khác): tạo, refresh, change source, break, security. Pane modernized 2024 thay legacy "Edit Links" dialog.

## Trạng thái hiện tại
- ✗ Chưa có. Hiện Ezcel chỉ load 1 workbook tại 1 thời điểm.

## 53.1 Workbook Link là gì

Workbook link (previously "external reference") = formula reference đến cell trong workbook KHÁC.

### Syntax
- Source closed: `='C:\path\[Workbook.xlsx]Sheet1'!$A$1` — path đầy đủ + tên file trong `[...]` + sheet + cell absolute.
- Source open: `=[Workbook.xlsx]Sheet1!$A$1` — không cần path.
- Cache value: Excel cache giá trị link trong file destination → vẫn xem được khi source offline (giá trị cũ).

### Tạo link
- Mở 2 workbook. Trong destination cell, gõ `=` → switch sang source workbook → click cell → Enter. Auto generate path.
- Hoặc paste link: copy cell trong source → Paste Special → Link (icon).

## 53.2 Workbook Links pane (modern 2024 — thay Edit Links dialog)

Data → Queries & Connections → **Workbook Links** → pane bên phải:

```
┌─ Workbook Links ────────────────────────────────────┐
│ [🔄 Refresh All]   [⚙ Settings]   [+ Add Link]      │
│ ──────────────────────────────────────────────────  │
│ ▼ Workbook1.xlsx                       ⋯           │
│    Status: ✓ OK (last refresh 14:30)               │
│    Source: C:\Users\...\Workbook1.xlsx             │
│    Refresh: [Always refresh ▼]                     │
│    [🔄 Refresh] [Open Source] [Change Source]      │
│    [Break Link] [Check Status]                      │
│ ──────────────────────────────────────────────────  │
│ ▼ Q1Sales.xlsx                          ⋯          │
│    Status: ⚠ Source not found                       │
│    Source: \\share\team\Q1Sales.xlsx                │
│    [Change Source] [Break Link]                     │
└─────────────────────────────────────────────────────┘
```

### Per-link actions
- **Refresh** — pull lại data từ source.
- **Open Source** — mở source workbook trong Excel mới.
- **Change Source** — pick file mới; mọi formula reference auto-update path.
- **Break Link** — replace formula bằng cached value (data hardcoded, mất link).
- **Check Status** — verify source exists + accessible.

### Refresh settings (per link)
- **Ask to refresh**: dialog hỏi khi mở workbook.
- **Always refresh**: auto refresh khi mở.
- **Don't refresh**: không refresh + không ask.

## 53.3 Update behavior khi mở workbook

### Khi destination open:
1. Excel check links: Ask / Always / Don't.
2. Ask → dialog "This workbook contains links to one or more external sources... Update?" Yes / No / Help.
3. Yes → try fetch source:
   - Source available → refresh values + update cache.
   - Source not found → keep cached values + status warning.

### Trust Center settings (linked với [Spec 49](49-trust-center-privacy.md))
- File → Options → Trust Center → External Content → Security settings for Workbook Links:
  - Enable automatic update for all Workbook Links (not recommended).
  - Prompt user on automatic update.
  - Disable automatic update.

### Security tightening (Oct 2025 → Jul 2026 rollout)
- **NEW `FileBlockExternalLinks` group policy**: Microsoft expand File Block Settings ([Spec 49](49-trust-center-privacy.md)) áp cho external workbook links.
- Block list mặc định format risky: `.xls` (BIFF8), `.xlw`, `.dbf`.
- Khi link tới blocked format → cell trả **`#BLOCKED!` error** (mới — error code riêng) hoặc refresh fail silently.
- Admin override qua group policy; user override qua Trust Center → File Block Settings cột "External link" (Block / Prompt / Allow).
- Mục đích: chặn attack vector qua external link nhúng macro/legacy.

## 53.4 Find external links

- Data → Workbook Links pane (modern).
- Hoặc Find (Ctrl+F): search formula chứa `.xlsx]` hoặc `[`.
- Hoặc Name Manager: external links có thể tạo named ranges chỉ về source.
- Hidden links: chart series source / conditional formatting / data validation source / pivot data source — cần check từng nguồn ([Spec 53.5]).

## 53.5 Where external links hide

External refs có thể trong:
1. **Cell formulas** — most common.
2. **Defined names** (Named Ranges) — Refers To = external.
3. **Chart series** — Series Values = external range.
4. **Conditional formatting rules** — Use formula = external ref.
5. **Data validation List source** — `=[Wb.xlsx]Sheet1!$A$1:$A$10`.
6. **PivotTable source data** — external workbook.
7. **Object linking** (OLE — out of scope).

Workbook Links pane list TẤT CẢ các loại trên (modern improvement vs Edit Links cũ chỉ hiện cell formula).

## 53.6 Break Link

**Cảnh báo**: irreversible (sau khi save). Action:
- Formula chứa link → replaced bằng evaluated value tại thời điểm break.
- KHÔNG ảnh hưởng cell không có link.
- Save backup trước khi break (Excel không tự backup).

Dialog confirm: "Breaking links permanently converts formulas and external references to their existing values. Undo break links button cannot reverse this action. You should save your workbook first."

## 53.7 Change Source workflow

User scenario: source workbook bị rename / move → tất cả link broken.
- Workbook Links pane → click link broken → Change Source... → file picker.
- Excel mass-replace mọi formula reference: `[OldName.xlsx]Sheet1!A1` → `[NewName.xlsx]Sheet1!A1` (giữ sheet + cell).
- Edge case: sheet name không match → error "Cannot update some links" → manual fix.

## Model (nếu/khi Ezcel implement)

```python
class WorkbookLink:
    source_path: str                # absolute path
    source_name: str                # basename
    last_modified: datetime
    last_refresh: datetime
    refresh_mode: "ask" | "always" | "never"
    status: "ok" | "source_not_found" | "stale" | "error"
    cached_values: dict[(sheet, addr), value]    # cache khi source offline
    references: list[Reference]     # nơi link xuất hiện (cell/name/chart/cf/...)

class Reference:
    type: "cell" | "name" | "chart" | "cf" | "validation" | "pivot"
    location: str   # human readable: "Sheet1!A1" / "Chart 'Sales' series 2"
```

Workbook: `_external_links: list[WorkbookLink]`.

## Implementation note

- Formula parser ([Spec 12](12-formula-system.md)) cần token EXTERNAL_REF: `'\[([^\]]+)\]([^']+)'!([A-Z]+\d+)`.
- Resolver: khi gặp external ref → check cache → return cached value; async fetch source background nếu refresh due.
- File watcher: monitor source path → mark dirty khi source change.
- Pane UI: `QDockWidget` bên phải; list items + per-item action menu.

## Acceptance criteria
1. Mở workbook có link đến `Source.xlsx` → dialog "Update?" → Yes → values pull.
2. Source offline → values vẫn hiển thị (cached), status "Source not found".
3. Data → Workbook Links → pane list 1 link.
4. Click ⋯ → Change Source → pick file mới → mọi formula update path.
5. Break Link → confirm dialog → formulas convert to values; pane list rỗng.
6. Refresh mode "Always refresh" → mỗi lần mở workbook auto refresh.
7. External ref trong chart series → workbook links pane list "Chart 'Sales' series" reference.

## Phụ thuộc
- [12 Formula System](12-formula-system.md) — parse external ref token.
- [49 Trust Center](49-trust-center-privacy.md) — security settings.
- [35 Calculation Engine](35-calculation-engine.md) — recalc external dependencies.
- [10 Sheet Tabs](10-sheet-tabs.md) — multi-workbook context.

## Risk
- File access permissions (network share, OneDrive).
- Source unavailable → graceful degradation với cached values.
- Path resolution: relative vs absolute; portable workbook khi share.

## Phase
Phase 6+ (sau multi-sheet + advanced formula).
