# SPEC 56 — Sheet View (Modern Collaboration Filter/Sort Views)

## Mục tiêu
Mỗi user trong co-authoring có thể filter/sort riêng cho mình, không ảnh hưởng user khác — qua **Sheet View** (named view per-user). Khác với Custom Views ([Spec 14](14-freeze-split-views.md)) là feature legacy.

## Trạng thái hiện tại
- ✗ Chưa có (Ezcel chưa có co-authoring).

## ⚠ Sheet View vs Custom Views

| Aspect | Sheet View ([Spec 56]) | Custom Views ([Spec 14](14-freeze-split-views.md)) |
|---|---|---|
| Modern | ✓ (Excel 2021+ / M365) | ✗ Legacy (Excel 2010-) |
| Use case | Co-authoring filter/sort per-user | Lưu cả layout: zoom + freeze + hidden + print + filter |
| Storage | Per-user, cloud sync OneDrive | Workbook-wide, lưu trong xlsx |
| Scope | Per-sheet | Workbook |
| Captures | Filter, sort | All view state |
| Requires | Co-authoring + OneDrive/SharePoint | Local file |

Sheet View = **modern replacement** cho narrow use case "collaborative filter/sort".

## 56.1 Khi nào dùng

Workbook trong OneDrive được 2+ user share. User A sort by Date desc; user B muốn sort by Region asc. Không có Sheet View → 2 user xung đột.

Với Sheet View: mỗi user tạo Sheet View riêng → filter/sort áp riêng cho user đó, không ảnh hưởng dataset shared.

## 56.2 Tạo Sheet View

View tab → **Sheet View** group → **New** button.

### Behavior
- Switch sang "Temporary View" mode.
- Indicator: thanh đen mảnh phía trên grid `Sheet View: Temporary View` + dropdown.
- Tất cả sort/filter ngay sau đó → chỉ áp cho view này, không sync sang user khác.
- View bar cũng có button: Save / Exit / Switch / Delete / Options / Keep.

## 56.2.1 Known bug — "default view" shifts

Microsoft đã document (chưa fix tới Sep 2025) bug:
- User A tạo Sheet View đầu tiên (vd "Sort by Date") → save. Default view không đổi.
- User B (different) sau đó tạo Sheet View thứ 2 → save → **default view bị thay đổi cho tất cả users**. Khi mở file lại, mặc định là Sheet View của user B (không phải "no view" trạng thái original).
- Workaround: user B exit về "no view" trước khi save; hoặc user B set "Default" lại explicit.

Ezcel implementation phải tránh: lưu Sheet View → KHÔNG modify default state của workbook trừ khi user explicit set default.

## 56.2.2 Copy Link to Sheet View

Modern feature (rollout 2024-2025): chia sẻ filtered view với collaborator qua URL trực tiếp.

### UX
- Sheet View dropdown → "Copy link to view" → URL clipboard.
- URL format: `https://onedrive/.../workbook.xlsx#viewid=<guid>` → mở file qua URL → auto-switch sang view đó.
- Use case: bạn filter view "Q3 North Sales" → gửi link cho colleague → họ mở thấy đúng view đó.

### Cross-session "See mine / See others"
Khi user mở file đã có Sheet View → prompt "See mine or See others?":
- "See mine" = restore last view user used.
- "See others" = no view (default workbook state).
- **No persistent setting** — Excel hỏi mỗi session, KHÔNG có "Always choose X" preference (xác nhận đến Sep 2025).

## 56.3 Save / Rename

- Sheet View → Temporary View dropdown → type tên mới → Enter.
- Saved view appears trong Sheet View dropdown list.
- Mỗi user thấy đầy đủ list saved views của tất cả user (shared) nhưng chỉ user tạo mới sửa/xóa được.

## 56.4 Switch / Exit

- Switch view: View → Sheet View dropdown → chọn view.
- Exit view (về default shared state): View → Sheet View → **Exit** button hoặc Default View.

## 56.5 Behavior khi co-authoring với mixed users

### User A đang Sheet View, user B không
- User A filter → chỉ A thấy filter; B không bị ảnh hưởng.
- User B sort → user B thấy data sorted; user A trong Sheet View không bị ảnh hưởng.
- Cell edit (data change) → sync cả 2.

### User A không Sheet View, user B không
- User A sort → dialog: "Other people are working on this. Do you want to apply this sort just for you (Sheet View) or for everyone?"
  - **Just for me** → auto-create Sheet View "Temporary" cho user A.
  - **For everyone** → apply default → user B's view also reflects.

## 56.6 Sheet View dropdown

Saved views per sheet:
```
┌─ Sheet View ───────────────────────┐
│ ◆ Temporary View (current, unsaved) │
│ ◇ My filter Bắc                    │
│ ◇ Q1 only                          │
│ ◇ High value items                  │
│ ──────────────────────────────────  │
│ Default View (everyone)             │
└─────────────────────────────────────┘
```

- Click view → switch.
- Right-click view → Rename / Duplicate / Delete (chỉ owner).
- Group icon next to name = view shared visible cho all user; private icon = chỉ user tạo (Excel default: shared).

## 56.7 What Sheet View captures

- Filters trên Excel Tables.
- Filters từ AutoFilter (Data → Filter).
- Sort (single column hoặc multi-level).
- Pivot filters (with caveats).

### KHÔNG capture
- Cell formatting, data values (đó là shared).
- Hidden rows/cols (đó là shared layout).
- Frozen panes (shared).
- Conditional formatting rules (shared definitions, evaluation per user).
- Page Layout settings.
- Zoom level.
- Selected cell (per-user trong session, không persist).

Sheet View **chỉ tập trung filter + sort** — narrow scope.

## 56.8 Requirements

- Excel 2021+ / M365 / Excel for Web / Excel mobile.
- File trong OneDrive / OneDrive for Business / SharePoint Online.
- File format: `.xlsx` / `.xlsm` (KHÔNG `.xls` compat).
- Co-authoring enabled (≥1 collaborator).

## 56.9 Cross-platform

Sheet View available trên **all Excel platforms** sau khi tạo:
- Excel Desktop (Win/Mac)
- Excel for Web
- Excel mobile (iOS/Android — [Spec 52](52-mobile-excel.md))

## Model

```python
class SheetView:
    id: str               # UUID
    sheet_id: int
    owner_user_id: str    # user tạo
    name: str             # "Temporary View" or named
    filters: dict[(col), FilterCriteria]
    sort: list[SortCriteria]
    table_filters: dict[(table_id), TableFilterState]
    pivot_filters: dict[(pivot_id), PivotFilterState]
    is_temporary: bool
    is_shared_visible: bool  # show trong dropdown cho user khác
    created_at: datetime
```

Sheet: `_sheet_views: list[SheetView]`.

## Implementation note

### Storage
- Lưu trong xlsx OOXML extension namespace (mới): `xl/sheetViews/sheetView1.xml`.
- openpyxl chưa support — cần tự XML read/write.

### UX
- Sheet view bar: thin bar trên cùng grid (≈22px), background distinguished (vd light blue).
- Bar có: dropdown name / Save / Exit / Options / Keep / Switch / Delete.

### Filter/Sort interception
- Khi user trong Sheet View mode → sort/filter ops không emit `dataChanged` cross-user signal; chỉ update view-local state.
- Default mode → ops emit shared event.

### Without co-authoring
- Ezcel standalone: Sheet View **vẫn dùng được** như "saved filter/sort presets" per-sheet — không cần collaboration.
- Trigger UI vẫn hoạt động, no conflict resolution needed.

## Acceptance criteria
1. View → Sheet View → New → indicator bar xuất hiện "Sheet View: Temporary View".
2. Filter cột Region = "Bắc" → chỉ áp trong view, default view giữ nguyên.
3. Rename "Temporary View" → "Bắc chỉ" → saved view xuất hiện trong dropdown.
4. Exit → quay về Default View (no filter).
5. Switch lại "Bắc chỉ" → filter restore.
6. Delete view → confirm dialog → view bị xóa.
7. Multi-user (giả lập): user A apply filter → dialog "Just for me or everyone" → "Just for me" → auto-create Sheet View; user B không thấy filter.
8. Save xlsx → reopen → saved Sheet Views còn nguyên.

## Phụ thuộc
- [15 Filter / Sort](15-filter-sort.md) — filter/sort underlying.
- [44 Collaboration](44-collaboration-version-history.md) — co-authoring infrastructure.
- [10 Sheet Tabs](10-sheet-tabs.md) — per-sheet storage.
- [14 Freeze / Views](14-freeze-split-views.md) — phân biệt với legacy Custom Views.

## Risk
- Phase muộn vì cần co-authoring infrastructure cho full value.
- Standalone use case (saved filter presets) vẫn work mà không cần collaboration.
- xlsx schema mới — openpyxl chưa support, cần raw XML.

## Phase
Phase 6+ (sau khi multi-sheet ổn). Standalone version (saved filter presets) → Phase 5.
