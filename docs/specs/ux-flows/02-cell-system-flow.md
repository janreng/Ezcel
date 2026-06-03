# UX Flow — Spec 02 Cell System & Selection

> Spec gốc: [../02-cell-system.md](../02-cell-system.md)

## Selection states & visual

```
1. Single active cell (READY mode):
   ┌────┬────┬────┐
   │    │    │    │
   ├────┼━━━━┼────┤
   │    ┃ B2 ┃    │ ← active cell có border xanh đậm 2px
   ├────┼━━━━┼────┤   + fill handle ▪ góc dưới phải
   │    │    │    │
   └────┴────┴────┘

2. Multi-cell contiguous range:
   ┌────┬━━━━┯━━━━┓
   │    ┃ B1 │ C1 ┃ ← active cell B1 (đậm hơn)
   ├────┃━━━━┿━━━━┃  
   │    ┃ B2 │ C2 ┃ ← rest highlighted lighter
   ├────┃━━━━┿━━━━┃
   │    ┃ B3 │ C3 ┃
   └────┺━━━━┷━━━━┛ ← fill handle góc dưới phải vùng chọn

3. Non-contiguous (Ctrl+Click):
   ┌────┬━━━━┓────┬━━━━┓
   │    ┃ B1 ┃    ┃ D1 ┃
   ├────┺━━━━┛────┺━━━━┛
   │    │    │    │    │
   ├────┬━━━━┓────┬────┤
   │    ┃ B3 ┃    │    │
   └────┺━━━━┛────┴────┘

4. Entire column (click col header):
   A  ┃━B━┃ C  D
   ━━━┃━━━┃━━━━━
   1  ┃   ┃
   2  ┃   ┃
   3  ┃   ┃
   ... ┃ all ┃ rows selected

5. Entire row (click row header):
   ━━━━━━━━━━━━━━━━━━━━━
    A   B   C   D   ...
   ━━━━━━━━━━━━━━━━━━━━━
   ┃ 5 │ 5 │ 5 │ 5 │  ┃ ← row 5 fully selected
   ━━━━━━━━━━━━━━━━━━━━━

6. Select All (Ctrl+A / corner button):
   All cells in sheet highlighted
```

## Selection extension flows

```mermaid
flowchart TD
    A[Active cell B5] --> B{Action}
    
    B -->|Shift+Click D8| C[Extend selection B5:D8]
    B -->|Shift+Arrow Down| D[Extend by 1 row, B5:B6]
    B -->|Shift+Ctrl+Arrow Right| E[Extend to last non-empty cell in row]
    B -->|Shift+Ctrl+End| F[Extend to last used cell of sheet]
    B -->|Ctrl+Shift+Home| G[Extend back to A1]
    B -->|Ctrl+Shift+Space| H[Extend to Current Region rectangle]
    B -->|Ctrl+A once| H
    B -->|Ctrl+A twice| I[Entire sheet selected]
    
    C --> J[B5 remains active cell - bold border]
    D --> J
    E --> J
    F --> J
    G --> J
    H --> J
```

## Mouse drag selection

```mermaid
sequenceDiagram
    actor User
    participant Grid
    participant Status as Status Bar
    
    User->>Grid: MouseDown on B2
    Grid->>Grid: Set anchor = B2, active = B2
    Grid->>Grid: Show selection rect (just B2)
    
    loop User drags
        User->>Grid: MouseMove to D6
        Grid->>Grid: Update selection rect = B2:D6
        Grid->>Status: Update Count, Sum, Avg in status bar
    end
    
    User->>Grid: MouseUp at D6
    Grid->>Grid: Commit selection B2:D6
    Grid->>Grid: Active = D6 (last touched)
    Grid->>Status: Lock aggregates display
```

## Name Box selection input

```
User type "B5:D10" into Name Box, press Enter:
┌──────────────────────────────────┐
│ B5:D10           ▼ | fx | ...    │
└──────────────────────────────────┘

→ Grid selects range B5:D10, scrolls if needed, active cell = B5

User type "Sales[Revenue]" (named table column):
→ Selects all cells in Sales table's Revenue column

User type "MyRange" (named range):
→ Selects what MyRange refers to
```

## Ctrl+Click multi-range

```mermaid
flowchart TD
    A[Click B2] --> B[Selection = B2]
    B --> C[Drag to C4]
    C --> D[Selection = B2:C4]
    D --> E[Ctrl+Click F5]
    E --> F[Selection = B2:C4 + F5, active = F5]
    F --> G[Ctrl+Drag F5 to G7]
    G --> H[Selection = B2:C4 + F5:G7, active = G7]
    
    H --> I[Status bar: Count = 4+6 = 10 cells]
```

## Current Region (Ctrl+Shift+*)

```
Data layout:
   A    B    C    D    E
1  ID   Name Qty  Price        ← header row
2  1    A    10   100
3  2    B    20   200
4  3    C    30   300
5                              ← blank row
6  4    D    40   400

User clicks B3, presses Ctrl+Shift+*:
→ Selects A1:D4 (current region surrounding B3, bounded by blanks)
→ Row 5 excluded (blank), Row 6 excluded (separate region)
```

## Copy direction during selection

```
Anchor B2, drag MouseDown→Move D5:
selection.top    = min(B2.row, D5.row) = 2
selection.left   = min(B, D)           = B
selection.bottom = max(2, 5)            = 5
selection.right  = max(B, D)            = D
→ Final selection B2:D5

Drag from D5 → B2 (reverse):
→ Same final rectangle B2:D5
→ But active cell stays where mouse currently is (D5 if just released there)
```

## Selection performance hot-path

```mermaid
sequenceDiagram
    participant Mouse
    participant View as SpreadsheetView
    participant Model as SelectionModel
    participant Delegate as CellDelegate
    
    Mouse->>View: mousePressEvent at pixel (300, 150)
    View->>View: Hit test → row 5, col C
    View->>Model: setCurrentIndex(5, C), clear()
    Model-->>Delegate: dataChanged signal (selection)
    Delegate->>View: repaint affected rows only
    
    Note over View: Only viewport rows repaint, not all 1M
```

## Implementation hints cho Slave

- **QItemSelectionModel** built-in: dùng `selectionModel.select(QItemSelection, SelectionFlag)`.
- **Multi-range support**: native Qt selection model handles disjoint ranges via `QItemSelection.merge()`.
- **Active cell vs anchor**: `currentIndex` is active (where typing goes), separate from selection. Selection anchor = first cell in selection.
- **Border drawing trong CellDelegate.paint()**:
  - Selected cells: light fill `#E7F1FB` (Excel-like).
  - Active cell: 2px solid `#217346` border.
  - Selection outer border: 2px solid `#217346` around bounding rect.
- **Fill handle**: small black/green square 6x6 px tại góc dưới phải; hit area extended 12x12 cho mouse.
- **Status bar update**: connect `selectionChanged` signal → recompute Count/Sum/Avg/Min/Max in worker thread for large selections (>10k cells).
- **Ctrl+A logic**:
  - First press: select current region (Ctrl+Shift+*)
  - Second consecutive press: select entire sheet
  - Reset counter on any other action.
- **Performance**: cho 1M × 16k cells, KHÔNG materialize selection as cell list; lưu as `list[QRect]` ranges. Iterate khi cần aggregate.
