# SPEC 54 — Slicer & Timeline (Visual Filter Buttons / Date Slider)

## Mục tiêu
Visual filter primitives: **Slicer** (button group cho categorical filter) + **Timeline** (date range slider). Dùng được với PivotTable, Excel Table, PivotChart; có thể connect nhiều object cùng 1 Slicer.

## Trạng thái hiện tại
- ✗ Chưa có.
- Spec này tách riêng (cross-cutting) — trước đây chỉ nhắc qua trong [Spec 16 Table](16-table.md) và [Spec 18 PivotTable](18-pivot-table.md).

## 54.1 Slicer

### Insert
- **Cho Table**: chọn cell trong Table → Insert → Slicer → dialog list columns → check field(s) → OK.
- **Cho PivotTable**: PivotTable Analyze → Insert Slicer → dialog → OK.
- Mỗi field chọn → 1 Slicer object floating xuất hiện.

### Anatomy
```
┌─ Region ──────────┐
│ [Multi-Select] [×]│ ← header với multi-select + clear filter
├───────────────────│
│ ▣ Bắc              │ ← button highlighted = selected
│ ▢ Trung            │ ← unselected
│ ▣ Nam              │
│ ▢ (Blank)          │
│                    │
└────────────────────┘
```

### Interaction
- **Click button** → filter source (PivotTable / Table) theo value.
- **Ctrl + Click** → multi-select (toggle individual).
- **Shift + Click** → range select.
- **Multi-Select button** (top header, có shortcut Alt+S) → toggle multi-select mode (mặc định: click = single, multi-select on = click = toggle multi).
- **Clear Filter button** (×, top header) → bỏ tất cả filter.
- **Right-click** → context menu: Sort A→Z / Sort by data source order / Report Connections / Slicer Settings / Remove Slicer.

### Slicer Tools contextual tab
- **Slicer Caption**: header text editable.
- **Slicer Settings...**: dialog với Name (technical), Caption (display), Header visibility, Sort and Filter options, hide items with no data, indicate items deleted from data source.
- **Report Connections**: dialog list pivots/tables cùng workbook → checkbox connect Slicer điều khiển nhiều cái.
- **Buttons group**: Columns (1-9 grid), Height, Width.
- **Size group**: total width/height of Slicer.
- **Slicer Styles** gallery: 14 preset Light/Dark; **New Slicer Style...** = duplicate + edit (built-in styles read-only).

### Report Connections
- 1 Slicer → control 1-N PivotTables/PivotCharts/Tables share **same source data / Data Model**.
- Right-click Slicer → Report Connections → dialog checkbox.

## 54.2 Timeline

### Insert
- Chỉ với **date/time field**.
- Insert → Timeline / PivotTable Analyze → Insert Timeline → dialog list date fields → check 1 → OK.

### Anatomy
```
┌─ OrderDate ────────────────────────────────────────────┐
│ [Filter cleared]                              [MONTHS▼]│
│ ─────────────────────────────────────────────────────  │
│ ┌─────────────────────────────────────────────────────┐│
│ │ Jan  Feb  Mar  Apr  May  Jun  Jul  Aug  Sep  Oct... ││ ← Period selector
│ │      ██████████████                                   ││ ← Selected range
│ │      ↑                ↑                                ││ ← Drag handles
│ └─────────────────────────────────────────────────────┘│
│                                                         │
│ 2026                                                    │
└─────────────────────────────────────────────────────────┘
```

### Interaction
- **Drag handles** (2 đầu vùng selected) → mở rộng/thu hẹp range.
- **Click period bar** (1 month) → select duy nhất tháng đó.
- **Shift + Click** range → range from current → click.
- **Level dropdown** (top-right): Years / Quarters / Months / Days — đổi granularity.
- **Scroll**: drag selection di chuyển across time axis.
- **Clear filter button** → bỏ filter.

### Timeline Tools contextual tab
- **Timeline Caption**: header.
- **Timeline Styles** gallery: 12 preset Light/Dark + custom.
- **Show**: Header / Selection Label / Scrollbar / Time Level (toggle visibility).
- **Size**: width/height.
- **Report Connections**: tương tự Slicer — control nhiều pivot cùng date field.

## 54.3 Styling (chung Slicer + Timeline)

### Built-in styles
- 14 Slicer styles + 12 Timeline styles preset.
- Light variants (1-7): light gray theme + accent color.
- Dark variants (1-7): dark gray theme.
- **KHÔNG edit được built-in styles**.

### Custom style
- Slicer Styles gallery → **New Slicer Style** → dialog Modify Slicer Element:
  - Whole Slicer
  - Header
  - Selected Item with Data
  - Selected Item with No Data
  - Unselected Item with Data
  - Unselected Item with No Data
  - Hovered Selected Item
  - Hovered Unselected Item
  - Sorted Ascending / Descending Item
- Set Font / Border / Fill per element.
- Save → custom style xuất hiện trong gallery + áp được như preset.

### Duplicate trick
- Right-click built-in style → Duplicate → mở dialog với current values → modify → save as new custom style.

## 54.4 Layout & arrange

- Slicer/Timeline = shape objects ([Spec 34](34-shapes-images-smartart.md)) — anchor + position behavior.
- Default anchor: "Move and size with cells" — di chuyển/resize cùng cells.
- Right-click → Size and Properties → anchor options.
- Z-order: Bring to Front / Send to Back.
- Group multiple slicers: select Ctrl+click → right-click → Group.

## 54.5 Differences Slicer vs Timeline

| Aspect | Slicer | Timeline |
|---|---|---|
| Field type | Categorical (text / number) | Date / Datetime only |
| Filter mode | Multi-select buttons | Range slider (continuous) |
| Granularity | N/A | Years/Quarters/Months/Days dropdown |
| Multi-Select | Buttons với Ctrl+click | Drag range only |
| Source | Table / PivotTable / Data Model | Same, nhưng cần date column |
| Min source version | Excel 2010 | Excel 2013 |

## Model

```python
class Slicer(Shape):
    type = "slicer"
    source_field: str
    source_data_id: str       # table_name or pivot_name
    selected_values: set
    multi_select_mode: bool
    columns: int
    style: str                # style name
    report_connections: list[str]    # IDs of connected pivots/tables

class Timeline(Shape):
    type = "timeline"
    source_field: str
    source_data_id: str
    level: "years" | "quarters" | "months" | "days"
    selected_start: date
    selected_end: date
    show_header: bool
    show_selection_label: bool
    show_scrollbar: bool
    show_time_level: bool
    style: str
    report_connections: list[str]
```

Sheet: `_slicers: list[Slicer]`, `_timelines: list[Timeline]`.

## Implementation note

### Rendering
- `QWidget` floating overlay; child = `QPushButton` list cho Slicer / custom `QSlider`-like cho Timeline.
- Embed trong grid như Shape — overlay z-order trên cells.
- Style: CSS-like QSS theo style preset; custom style = override.

### Filter propagation
- Khi Slicer/Timeline state change → emit signal `filter_changed(field, values)`.
- Connected pivots/tables subscribe → apply filter → re-render.
- Apply formula recompute (debounced 100ms).

### xlsx mapping
- openpyxl: `Slicer` / `Timeline` chưa fully supported. Cần tự XML write/read theo OOXML schema (`xl/slicers/slicer1.xml`, `xl/timelines/timeline1.xml`).

## Acceptance criteria
1. Tạo PivotTable từ data có Region + Sales. PivotTable Analyze → Insert Slicer → check Region → Slicer xuất hiện với 3 button (Bắc/Trung/Nam).
2. Click "Bắc" → PivotTable filter chỉ hiện Region Bắc.
3. Ctrl + Click "Nam" → Slicer có 2 selected (Bắc + Nam), PivotTable hiển thị cả 2.
4. Multi-Select button toggle → click 1 button không cần Ctrl mà vẫn toggle.
5. Right-click Slicer → Report Connections → connect Slicer này tới PivotTable thứ 2 → click "Bắc" → cả 2 pivot filter.
6. Date field "OrderDate" → Insert Timeline → range slider hiện 12 tháng.
7. Drag handle phải từ Dec → Jun → Timeline range = Jan-Jun, pivot filter date.
8. Level dropdown → Quarters → period bar đổi Q1/Q2/Q3/Q4.
9. Slicer Styles gallery → Duplicate "Dark 1" → modify "Selected Item" fill = đỏ → save as "MySlicerStyle" → áp vào Slicer → button selected có nền đỏ.
10. Save xlsx → reopen → Slicer/Timeline + filter state còn nguyên.

## Phụ thuộc
- [16 Table](16-table.md) — Slicer cho Excel Table.
- [18 PivotTable](18-pivot-table.md) — Slicer/Timeline cho PivotTable.
- [34 Shapes](34-shapes-images-smartart.md) — overlay rendering, anchor.
- [50 Design System](50-design-system.md) — style tokens.

## Risk
Trung bình. UI overlay + filter propagation graph. Custom style editor là dialog phức tạp.

## Phase
Phase 6+ (sau PivotTable + Table).
