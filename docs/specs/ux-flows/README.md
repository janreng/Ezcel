# UX Flows — visual diagrams cho từng tính năng

Folder này chứa **UX flow Markdown** cho từng tính năng từ specs gốc (`docs/specs/01-...`).

## Định dạng mỗi file

- **Tên file**: `<spec-num>-<feature>-flow.md` (vd `03-cell-modes-flow.md` ứng với spec 03).
- **Nội dung**:
  - **State diagram** (Mermaid `stateDiagram-v2` hoặc `flowchart`) — render được trên GitHub/VSCode preview.
  - **ASCII UI mockup** cho từng state quan trọng.
  - **Sequence diagram** (Mermaid `sequenceDiagram`) cho interactions phức tạp.
  - **Step-by-step user journey** với keyboard/mouse input mỗi step.
- Mục đích: **bot-main (Slave) khi code có visual ref**, không phải đoán từ text spec.

## Cách dùng

1. Mở spec gốc trước (vd `docs/specs/03-cell-modes.md`) để biết spec functional.
2. Mở file flow tương ứng (vd `ux-flows/03-cell-modes-flow.md`) để biết UX visual.
3. Implement: theo cả 2.

## Mermaid trên GitHub

GitHub render Mermaid native trong markdown. VSCode cần extension "Markdown Preview Mermaid Support".

```mermaid
flowchart LR
    A[State 1] -->|trigger| B[State 2]
```

## Status

Đang viết dần theo priority. Tracker:

| Spec | UX Flow | Note |
|---|---|---|
| 03 Cell Modes | ✓ [03-cell-modes-flow.md](03-cell-modes-flow.md) | State machine 4 modes (Phase 1 critical) |
| 04 Name Box & Formula Bar | ✓ [04-name-box-formula-bar-flow.md](04-name-box-formula-bar-flow.md) | Navigate / autocomplete dropdown |
| 05 AutoFill / Flash Fill | ✓ [05-autofill-flow.md](05-autofill-flow.md) | Fill handle + pattern recognition |
| 08 Format Cells Dialog | ✓ [08-format-cells-flow.md](08-format-cells-flow.md) | 6-tab dialog Ctrl+1 |
| 12 Formula System | ✓ [12-formula-system-flow.md](12-formula-system-flow.md) | Entry flow + Function Wizard |
| 13 Clipboard / Paste Special | ✓ [13-paste-special-flow.md](13-paste-special-flow.md) | Paste Options icon-bar + dialog |
| 17 Conditional Formatting | ✓ [17-conditional-formatting-flow.md](17-conditional-formatting-flow.md) | Rule creation + Manage Rules |
| 39 Copilot / Agent Mode | ✓ [39-copilot-flow.md](39-copilot-flow.md) | Pane / Agent plan / Edit with Copilot |
| 51 Start Screen / Backstage | ✓ [51-start-screen-flow.md](51-start-screen-flow.md) | App startup + Backstage navigation |
| 02 Cell System Selection | ✓ [02-cell-system-flow.md](02-cell-system-flow.md) | Multi-range, Ctrl+Click, Ctrl+A, Name Box jump |
| 06 Context Menus + Mini Toolbar | ✓ [06-context-menus-flow.md](06-context-menus-flow.md) | Cell/Row/Col/Sheet menus + Mini Toolbar position |
| 11 Status Bar | ✓ [11-status-bar-flow.md](11-status-bar-flow.md) | Aggregates + Zoom + AutoSave + Mode |
| 15 Filter / Sort | ✓ [15-filter-sort-flow.md](15-filter-sort-flow.md) | Filter dropdown, multi-level sort, Advanced |
| 18 PivotTable | ✓ [18-pivot-table-flow.md](18-pivot-table-flow.md) | Field List, drag-drop, Value Field Settings |
| 25 Data Validation | ✓ [25-data-validation-flow.md](25-data-validation-flow.md) | 3-tab dialog + List dropdown + cascading |
| 32 Find / Replace / Go To | ✓ [32-find-replace-goto-flow.md](32-find-replace-goto-flow.md) | Find All panel + Go To Special + wildcards |
| 55 Tell Me / Search | ✓ [55-tell-me-search-flow.md](55-tell-me-search-flow.md) | Microsoft Search + action index + Smart Lookup |
| 19 Chart | ✓ [19-chart-flow.md](19-chart-flow.md) | Insert dialog, Recommended, Chart Tools, Format Pane, Move |
| 22 Modern features | ✓ [22-modern-features-flow.md](22-modern-features-flow.md) | Dynamic arrays, spill, #, XLOOKUP, LAMBDA, GROUPBY/PIVOTBY |
| 26 Comments & Notes | ✓ [26-comments-notes-flow.md](26-comments-notes-flow.md) | Modern threaded vs legacy note, @mentions, Comments pane |
| 54 Slicer & Timeline | ✓ [54-slicer-timeline-flow.md](54-slicer-timeline-flow.md) | Slicer panel, cross-filter, Timeline drag, Report Connections |
| 56 Sheet View | ✓ [56-sheet-view-flow.md](56-sheet-view-flow.md) | Personal filter/sort view, collaboration scenario, scope |
| 07 Ribbon | ✓ [07-ribbon-flow.md](07-ribbon-flow.md) | Tabs/groups, KeyTips Alt navigation, Customize Ribbon, contextual tabs, touch mode |
| 10 Sheet Tabs | ✓ [10-sheet-tabs-flow.md](10-sheet-tabs-flow.md) | Add/rename/color, drag reorder, Group mode multi-select, hide/unhide |
| 16 Table (ListObject) | ✓ [16-table-flow.md](16-table-flow.md) | Ctrl+T, Table Design tab, structured refs, calculated columns, AutoExpand, Total Row |
| 23 Keyboard Shortcuts | ✓ [23-keyboard-shortcuts-flow.md](23-keyboard-shortcuts-flow.md) | Top-50, F4 ref toggle, mode routing, KeyTips, Customize Keyboard |
| 27 Data Tools | ✓ [27-data-tools-flow.md](27-data-tools-flow.md) | Text to Columns wizard, Remove Duplicates, Consolidate, Relationships |
| 28 What-If Analysis | ✓ [28-what-if-analysis-flow.md](28-what-if-analysis-flow.md) | Goal Seek, Scenario Manager + Summary, Data Table 1/2-var, Solver, Forecast Sheet |
| 01 Grid Engine | ✓ [01-grid-engine-flow.md](01-grid-engine-flow.md) | Grid anatomy, virtualization, scroll, HiDPI, touch, limits |
| 09 Row/Col Operations | ✓ [09-row-col-operations-flow.md](09-row-col-operations-flow.md) | Insert/Delete dialogs, auto-fit, Group/Outline, Subtotal |
| 14 Freeze/Split/Views | ✓ [14-freeze-split-views-flow.md](14-freeze-split-views-flow.md) | Freeze 4-quadrant, Split bars, New Window, Side by Side, Custom Views |
| 20 Power Query | ✓ [20-power-query-flow.md](20-power-query-flow.md) | Get Data → Editor → M Applied Steps, Group/Merge/Append |
| 30 Themes & Cell Styles | ✓ [30-themes-cell-styles-flow.md](30-themes-cell-styles-flow.md) | Theme Colors/Fonts/Effects, Cell Styles gallery, Aptos default |
| 31 Named Ranges | ✓ [31-named-ranges-flow.md](31-named-ranges-flow.md) | Name Box / Define Name / Name Manager / F3 Paste / Create from Selection |
| 50 Design System | ✓ [50-design-system-flow.md](50-design-system-flow.md) | Color tokens, typography, spacing, components, a11y, dark mode |
| 21 Python in Excel (no VBA) | ✓ [21-vba-macro-flow.md](21-vba-macro-flow.md) | =PY() cells, Code Editor, UDFs, Record Macro Python, debugger |
| 33 Sparklines | ✓ [33-sparklines-flow.md](33-sparklines-flow.md) | Line/Column/Win-Loss inline charts, axis options, group/markers |
| 34 Shapes Images SmartArt | ✓ [34-shapes-images-smartart-flow.md](34-shapes-images-smartart-flow.md) | Insert gallery, Background Remove AI, SmartArt Text Pane, Selection Pane |
| 35 Calc Engine | ✓ [35-calculation-engine-flow.md](35-calculation-engine-flow.md) | Dep graph, topo sort, volatile, F9/Shift+F9/Ctrl+Alt+F9, iterative, multi-threaded |
| 36 File Formats AutoSave | ✓ [36-file-formats-autosave-flow.md](36-file-formats-autosave-flow.md) | xlsx/xlsm/xlsb/xls/csv/pdf, AutoSave indicators, AutoRecover, encryption AES-256, Inspect Doc |
| 38 Linked Data Types | ✓ [38-linked-data-types-flow.md](38-linked-data-types-flow.md) | Stocks/Geography/custom, Data Card, =A1.Field syntax, FIELDVALUE, refresh modes |
| 41 Accessibility | ✓ [41-accessibility-flow.md](41-accessibility-flow.md) | Accessibility Checker, Read Aloud, Alt Text + AI gen, screen reader, contrast tools, high contrast mode |
| 44 Collaboration Version History | ✓ [44-collaboration-version-history-flow.md](44-collaboration-version-history-flow.md) | Share dialog, presence avatars, real-time selection display, Version History, Show Changes pane, Teams |
| 24 Print & Page Setup | ✓ [24-print-page-setup-flow.md](24-print-page-setup-flow.md) | Backstage Print, Page Setup 4 tabs, Custom Header tokens, Page Break Preview drag, Export PDF |
| 29 Protection | ✓ [29-protection-flow.md](29-protection-flow.md) | Sheet/Workbook/Range protection, password algorithms, Mark as Final, Encrypt with Password, behavior matrix |
| 37 Form Controls | ✓ [37-form-controls-flow.md](37-form-controls-flow.md) | Insert dropdown, Check/Option/List/Combo/Scroll/Spin/Button, Group Box mutex, Assign Macro |
| 40 Quick Analysis | ✓ [40-quick-analysis-flow.md](40-quick-analysis-flow.md) | ⚡ button placement, 5-tab popup, live preview snapshot/restore, Recommended Charts/Pivots, Analyze Data |
| 42 Proofing / Translate | ✓ [42-proofing-translate-flow.md](42-proofing-translate-flow.md) | F7 dialog, wavy underline, AutoCorrect engine, Math AutoCorrect, Translate pane, Thesaurus, Set Proofing Language |
| 43 Office Scripts | ✓ [43-office-scripts-flow.md](43-office-scripts-flow.md) | Decision record + detection InfoBar + Office Script ↔ Python mapping + Convert with Copilot |
| 45 Power Pivot / DAX | ✓ [45-power-pivot-flow.md](45-power-pivot-flow.md) | Manage window, Data/Diagram/Measure views, relationship drag, DAX subset, KPIs, hierarchies, Ezcel pandas backend |
| 47 Camera / Snapshot | ✓ [47-camera-snapshot-flow.md](47-camera-snapshot-flow.md) | QAT setup, crosshair-place flow, Paste as Linked Picture, auto-update debounce, Picture Format ribbon, xlsx round-trip |
| 46 3D Maps (retired) | ✓ [46-3d-maps-flow.md](46-3d-maps-flow.md) | Decision record + detection InfoBar + Convert to Map Chart + passthrough preservation |
| 48 Touch / Pen / Ink | ✓ [48-touch-pen-ink-flow.md](48-touch-pen-ink-flow.md) | Touch Mode toggle, Draw tab + Pen Toolbox, Ink to Text Pen (2024+), Ink to Shape rules, Ink to Math modal, Lasso, Ink Replay, Apple Pencil settings |
| 49 Trust Center / Privacy | ✓ [49-trust-center-privacy-flow.md](49-trust-center-privacy-flow.md) | Trust Center 10+ tabs, Macro Settings (Python), Protected View MOTW, File Block 3-column (NEW external link), Sensitivity Labels (Purview), Privacy connected-experiences cascade |
| 52 Mobile Excel | ✓ [52-mobile-excel-flow.md](52-mobile-excel-flow.md) | Gesture cheat-sheet, iPad/iPhone/Android layouts, Apple Pencil + Scribble, Capture Data from Picture, Mac shortcut alignment, Forms live sync (May 2026 Mac) |
| 53 Workbook Links | ✓ [53-workbook-links-flow.md](53-workbook-links-flow.md) | Workbook Links pane modern (replaces Edit Links), Update on open flow, 6 surfaces hide table, Change Source, Break Link confirm, status badges, NEW #BLOCKED! external link |
| 57 Workbook Statistics | ✓ [57-workbook-statistics-flow.md](57-workbook-statistics-flow.md) | Stats dialog per-sheet/workbook, Properties side panel, Advanced Properties 5 tabs (Custom typed), Document Inspector 18 categories, Compatibility Checker rule list, OOXML mapping |
| 58 Add-ins / AppSource | ✓ [58-add-ins-appsource-flow.md](58-add-ins-appsource-flow.md) | Office Add-ins dialog 3 tabs, install lifecycle, manifest XML vs Unified JSON status matrix, permission scopes, Ezcel Python plugin (.ezpkg) alternative full UX |

Mục tiêu: 58/58 spec đều có UX flow. **ĐÃ HOÀN THÀNH ✓ (58/58)** tính đến Iter 14 (June 2026).
