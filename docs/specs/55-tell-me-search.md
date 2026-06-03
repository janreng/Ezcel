# SPEC 55 — Tell Me / Microsoft Search (Alt + Q)

## Mục tiêu
Search box trên ribbon (giữa Title Bar) — search command, help docs, action, file content. Modern Excel default UX cho "where do I find X".

## Trạng thái hiện tại
- ✗ Chưa có.

## 55.1 Search box position

Modern Excel: **center của Title Bar** (Office 2019+) hoặc **end của ribbon** (older Office).

```
┌─ [≡] [📊] [👤] [Title - Excel]  [🔍 Search________________________]  [_][□][×]
├──────────────────────────────────────────────────────────────────────────
│  File   Home   Insert   Page Layout   Formulas   ...
```

### Shortcut
- **Alt + Q** (focus search box).
- Click box.

## 55.2 Search dropdown (khi focus)

```
┌─ Search ────────────────────────────────────────────────┐
│ [pivot table_________________________________________]   │
│ ──────────────────────────────────────────────────────  │
│ ▸ Best actions for "pivot table"                         │
│   🔢 Insert PivotTable                                   │
│   📊 Insert Recommended PivotTable                       │
│   🔄 Refresh PivotTable                                  │
│ ▸ Get Help on "pivot table"                              │
│   📖 Create a PivotTable to analyze worksheet data       │
│   📖 PivotTable basics                                   │
│ ▸ Find in workbook                                       │
│   "pivot table" in B5, B6 (Sheet1)                       │
│ ▸ Recent files                                           │
│   PivotTable_Sales.xlsx — 2 days ago                     │
│ ▸ People                                                 │
│   [if cloud] Trang Nguyen — recent collaborator          │
└──────────────────────────────────────────────────────────┘
```

### Categories
1. **Best actions**: command direct (click → execute). Vd "insert pivot" → Insert PivotTable.
2. **Get help**: Microsoft docs links.
3. **Find in workbook**: cell content matching (like Find Ctrl+F).
4. **Recent files**: workbook tên matching.
5. **People** (cloud only): collaborators.
6. **Smart Lookup** (deprecated 2024 — replaced by Search).

### Behavior
- Real-time results khi gõ (debounce 200ms).
- Up/Down arrow navigate; Enter execute.
- Esc close dropdown.

## 55.3 Smart suggestions (May 2026 update)

Modern Excel với Copilot license:
- **Curated smart suggestions** xuất hiện trong search box dropdown — Copilot suggest action dựa context (selected cells, recent actions).
- Vd: select range data số → suggestion "Create chart from selection", "Analyze with Copilot", "Apply Conditional Formatting Color Scale".

## 55.4 History

Modern Office (2019+) → **Microsoft Search box** thay "Tell Me" branding:
- Bing Search backend (legacy Tell Me used local index).
- Cloud search results (with M365 license).

Old "Tell Me what you want to do" (Office 2016) → renamed Microsoft Search (Office 2019+).

## 55.5 Settings

File → Options → General → Microsoft Search:
- Enable cloud search (Bing) ✓.
- Search privacy (telemetry opt-in).
- Default scope: Commands / All / Files.

## Implementation note

### Search index
- Build local index khi app start:
  - Mọi ribbon command + alias + tooltip.
  - Recent files (từ QSettings).
  - Open workbook cell content.
- Live update khi cells change.

### Fuzzy match
- Library: `rapidfuzz` (Python, optional) — fast fuzzy string match.
- Hoặc tự viết với Levenshtein distance.
- Score: weighted (action label > tooltip > help text > cell content).

### UI
- `QLineEdit` + dropdown `QListWidget` floating.
- Khi gõ → emit `textChanged` → debounce 200ms → query index → update dropdown.

### Cloud search (optional)
- HTTP API: Microsoft Search / Bing Search API (cần API key).
- Phase muộn.

## Acceptance criteria
1. Alt + Q → focus search box.
2. Gõ "pivot" → dropdown hiện trong 300ms với 5+ kết quả categorized.
3. Click "Insert PivotTable" → trigger command (cùng như Insert → PivotTable).
4. Gõ "borderr" (typo) → fuzzy match suggest "Add Border", "Border styles".
5. Gõ "5000" → "Find in workbook" hiện cells chứa 5000.
6. Esc → close dropdown.
7. Up/Down arrows → navigate; Enter → execute selected.

## Phụ thuộc
- [07 Ribbon](07-ribbon.md) — commands index.
- [32 Find / Replace](32-find-replace-goto.md) — workbook content search.
- [39 Copilot](39-copilot-agent.md) — smart suggestions.
- [51 Start Screen / Backstage](51-start-screen-backstage.md) — recent files index.

## Risk
Thấp-trung bình. Index size manageable; fuzzy match library mature.

## Phase
Phase 5+ (sau khi ribbon + commands stable).
