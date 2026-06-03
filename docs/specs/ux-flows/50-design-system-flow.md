# UX Flow — Spec 50 Design System (Ezcel UX standards)

> Spec gốc: [../50-design-system.md](../50-design-system.md)

## Color palette — Excel green brand

```
PRIMARY (brand identity):
┌────────┬───────────┬─────────────────────────────────────────┐
│ Token  │ Hex       │ Usage                                    │
├────────┼───────────┼─────────────────────────────────────────┤
│ brand  │ #107C41   │ App icon, splash, ribbon File menu      │
│ accent │ #217346   │ Active cell border, selection outline,  │
│        │           │ checkbox checked, focus ring             │
│ tint1  │ #1E8E5E   │ Hover on accent                          │
│ tint2  │ #185A36   │ Pressed/active accent                    │
└────────┴───────────┴───────────────────────────────────────────┘

NEUTRAL (UI chrome):
┌────────┬───────────┬─────────────────────────────────────────┐
│ bg0    │ #FFFFFF   │ Workspace, cell content background       │
│ bg1    │ #F3F2F1   │ Ribbon, status bar, side panel           │
│ bg2    │ #E1DFDD   │ Inactive tabs, hover row in lists        │
│ bg3    │ #C8C6C4   │ Divider lines, borders                   │
│ fg0    │ #201F1E   │ Primary text                              │
│ fg1    │ #605E5C   │ Secondary text, disabled labels          │
│ fg2    │ #8A8886   │ Placeholder, helper text                  │
└────────┴───────────┴───────────────────────────────────────────┘

SEMANTIC (state & feedback):
┌────────────┬───────────┬─────────────────────────────────────┐
│ success    │ #28A745   │ Saved indicator, success toast       │
│ warning    │ #FFC107   │ Yellow alert (Warning style)         │
│ danger     │ #DC3545   │ Error alert (Stop style), #REF!     │
│ info       │ #0078D4   │ Hyperlinks, info dialogs            │
│ cf-red     │ #FFC7CE   │ Conditional format fill red (Bad)   │
│ cf-yellow  │ #FFEB9C   │ Conditional format fill yellow      │
│ cf-green   │ #C6EFCE   │ Conditional format fill green (Good)│
│ cf-redfg   │ #9C0006   │ CF text on red bg                    │
│ cf-yelfg   │ #9C5700   │ CF text on yellow bg                 │
│ cf-grnfg   │ #006100   │ CF text on green bg                  │
└────────────┴───────────┴───────────────────────────────────────┘
```

## Typography stack

```
┌────────────────┬─────────────────────────┬─────────────────────┐
│ Use Case        │ Font Stack              │ Notes                │
├────────────────┼─────────────────────────┼─────────────────────┤
│ Cell content    │ Aptos Narrow → Calibri  │ Default 11pt        │
│                 │ → Arial → system        │                      │
│ Cell heading    │ Aptos Display → Aptos   │ For Heading styles  │
│ Cell mono       │ Aptos Mono → Consolas   │ For code data       │
│ UI chrome       │ Segoe UI Variable →     │ Ribbon, menus,      │
│                 │ Segoe UI → Tahoma       │ dialogs, status bar │
│ Title bar       │ Segoe UI Variable Small │ 9pt                  │
│ Menu/Ribbon     │ Segoe UI Variable Body  │ 9pt regular         │
│ Tooltip         │ Segoe UI Variable Body  │ 9pt regular         │
│ Heading H1      │ Segoe UI Variable Display│ 24pt semibold      │
│ Backstage       │ Segoe UI Variable       │ Various sizes        │
└────────────────┴──────────────────────────┴─────────────────────┘

Font sizes (cell context):
8, 9, 10, 11 (default), 12, 14, 16, 18, 20, 22, 24, 26, 28, 36, 48, 72

Font weights:
- Regular (400) — default
- Semibold (600) — emphasis, h2/h3 headings
- Bold (700) — strong emphasis, totals row
- Light (300) — large display text only
```

## Spacing system (8px grid)

```
Base unit = 4px (sub-step), main unit = 8px

┌────────┬──────┬─────────────────────────────────┐
│ Token  │ px   │ Usage                            │
├────────┼──────┼─────────────────────────────────┤
│ space-0│ 0    │ flush                            │
│ space-1│ 4    │ icon padding inside button       │
│ space-2│ 8    │ between related controls          │
│ space-3│ 12   │ between unrelated controls        │
│ space-4│ 16   │ between groups in ribbon         │
│ space-5│ 24   │ between sections in dialog       │
│ space-6│ 32   │ between major regions             │
│ space-7│ 48   │ between dialog edge & content    │
│ space-8│ 64   │ large negative space             │
└────────┴──────┴───────────────────────────────────┘

Cell-specific spacing:
- Cell padding inside: 2px left, 2px right
- Header padding: 4px
- Row height default: 20px (15pt)
- Column width default: 64px (8.43 chars)
```

## Iconography

```
Icon system:
- Source: Fluent UI Icons 2.0 (Microsoft 365 unified)
- Sizes: 16px (small/toolbar), 20px (default), 24px (large), 32px (jumbo backstage)
- Style: Outlined (default), Filled (active state)
- Format: SVG (preferred for HiDPI)
- Color: inherits from foreground; defaults to fg0 #201F1E

Common icon sizes per UI element:
┌─────────────────────────┬──────┐
│ Ribbon button (small)    │ 16px │
│ Ribbon button (large)    │ 32px │
│ Toolbar button           │ 16px │
│ Context menu icon        │ 16px │
│ Tab icon                 │ 16px │
│ Cell smart tag           │ 12px │
│ Backstage navigation     │ 20px │
│ Status bar icon          │ 12px │
│ Dialog header icon       │ 24px │
└─────────────────────────┴──────┘

Custom Ezcel icons (where Fluent doesn't have):
- 4 cell mode badges (Ready/Enter/Edit/Point)
- Custom Ezcel logo (Aptos green leaf design)
```

## Component anatomy — Button

```
Standard button (3 states + 1 disabled):

Default (idle):
┌───────────────┐
│ Save           │  bg #FFFFFF, border #C8C6C4, fg #201F1E
└───────────────┘  font Segoe UI 9pt, padding 6px 12px

Hover:
┌───────────────┐
│ Save           │  bg #F3F2F1, border #C8C6C4
└───────────────┘  cursor: pointer

Pressed:
┌───────────────┐
│ Save           │  bg #E1DFDD, border #8A8886
└───────────────┘

Disabled:
┌───────────────┐
│ Save           │  bg #F3F2F1, fg #A19F9D
└───────────────┘  cursor: default

Primary action (e.g., OK):
┌───────────────┐
│ OK             │  bg #217346, fg #FFFFFF
└───────────────┘  hover: bg #1E8E5E

Danger:
┌───────────────┐
│ Delete         │  bg #DC3545, fg #FFFFFF
└───────────────┘  for destructive actions
```

## Component anatomy — Input field

```
Default:
┌──────────────────────────────┐
│ [type here]                   │  bg #FFFFFF, border 1px #C8C6C4
└──────────────────────────────┘  font Segoe UI 9pt, padding 4px 8px
                                    height 28px

Focus:
┌──────────────────────────────┐
│ Sales|                         │  border 2px #217346, bg #FFFFFF
└──────────────────────────────┘  caret blink 1.06s cycle

Error:
┌──────────────────────────────┐
│ 9999999999999                  │  border 2px #DC3545
└──────────────────────────────┘
  ⚠ Value must be ≤ 100             helper text #DC3545

Disabled:
┌──────────────────────────────┐
│ readonly                       │  bg #F3F2F1, fg #A19F9D
└──────────────────────────────┘

Cell input mode (in-grid):
- No border in default state
- 2px #217346 border around active cell
- Direct text editing on cell click + type or F2
```

## Component anatomy — Dialog

```
┌─ Dialog Title ─────────────────────────────────[✕]─┐
│ ┌── Icon (optional) ──┐ ┌── Content Area ──────┐  │
│ │ 24x24 icon            │ │                        │  │
│ │ (severity/type)       │ │ Body text Segoe UI 9pt│  │
│ │                       │ │ multiple paragraphs    │  │
│ └─────────────────────┘ │                         │  │
│                          │ Form controls:          │  │
│                          │ [Label]                  │  │
│                          │ [Input________________]  │  │
│                          │                         │  │
│                          │ ☑ Checkbox option        │  │
│                          └─────────────────────────┘  │
│                                                       │
│                              [Cancel] [OK ●primary]   │
└───────────────────────────────────────────────────────┘

Dialog frame:
- Background: #FFFFFF
- Border: 1px #C8C6C4
- Drop shadow: rgba(0,0,0,0.14) 0 4px 8px
- Min width: 320px, Max width: 800px (resizable for some)
- Header height: 36px
- Footer height: 48px (buttons aligned right)
- Padding: 16px content area

Modal overlay:
- Semi-transparent black 50% covers parent window
- Click overlay → focus jumps back to dialog (no dismiss)
- Esc → cancel (if Cancel exists)
- Enter → OK / default button
```

## Component anatomy — Tooltip

```
Tooltip (hover for ~500ms):
┌──────────────────────┐
│ Bold (Ctrl+B)         │  bg #2D2C2B, fg #FFFFFF
└──────────────────────┘  font Segoe UI 9pt, padding 4px 8px
                            border-radius 2px
                            drop shadow rgba(0,0,0,0.2)

Position: below cursor + 12px offset
Max width: 240px (wrap to multiple lines)
Auto-dismiss: 5s or mouse leave

Rich tooltip (ScreenTip for ribbon buttons):
┌─────────────────────────────────────────┐
│ ┃ Conditional Formatting                  │ ← title bold
│ ┃ ─────────────────────────────────────│
│ ┃ Easily spot trends and patterns in your  │ ← description
│ ┃ data with bars, colors, and icons       │
│ ┃ that visually highlight important values.│
│ ┃                                          │
│ ┃ Press F1 for more help                   │ ← help hint italic gray
└─────────────────────────────────────────────┘
Used for ribbon buttons (more verbose than simple tooltip).
```

## Component anatomy — Cell selection

```
Active cell:
   ┌════════════┐
   ┃            ┃ ← 2px solid #217346 border
   ┃   B5       ┃   inner content unchanged
   ┃            ┃
   └════════════┘
                ▪ ← fill handle: 6×6 px square #217346, bottom-right corner

Multi-cell selection:
   ┌────┬════════════┐
   │    ┃ B5 (active)┃ ← 1px outer border on every selected cell
   ├────┃────────────┃ ← active cell highlighted differently within selection
   │    ┃ B6         ┃
   │    ┃ B7         ┃
   ├────┃────────────┃
   │    ┃ B8         ┃
   └────┺════════════┘
              ▪ ← fill handle at bottom-right of selection bounding box

Fill colors:
- Selected: bg blend +20% blue (#E7F1FB on white)
- Active cell within selection: bg stays #FFFFFF, distinguishes
```

## Motion / animation

```
Standard durations:
- Micro:    100ms — hover state, button press, cell highlight
- Short:    200ms — dropdown open, menu hover transition
- Medium:   300ms — dialog fade in/out, ribbon collapse
- Long:     500ms — page transition (Backstage View)
- Spinner:  1500ms cycle — busy indicator

Easing:
- ease-out (for entering): cubic-bezier(0, 0, 0.2, 1)
- ease-in (for exiting):   cubic-bezier(0.4, 0, 1, 1)
- ease-in-out:             cubic-bezier(0.4, 0, 0.2, 1)

Examples:
- Cell hover → background fade 100ms ease-out
- Dropdown open → slide down 200ms ease-out + fade
- Dialog open → fade + scale from 0.95→1.0 in 300ms
- Marching ants (copy border) → 200ms cycle (loop)
- Spill border on dynamic array → fade in 200ms

Reduced motion mode:
- Detect Windows "Show animations" setting → disable all transitions
- Substitute opacity 0 → 1 with instant swap
- No marching ants → static dashed border
```

## Accessibility (a11y)

```
WCAG 2.1 AA compliance targets:

Color contrast:
- Body text (fg0 on bg0): #201F1E on #FFFFFF = 17.7:1 (AAA)
- Secondary (fg1 on bg0): #605E5C on #FFFFFF = 5.6:1 (AA)
- Disabled (fg2 on bg0): #8A8886 on #FFFFFF = 3.3:1 (AA Large only)
- Active cell border: #217346 on #FFFFFF = 4.9:1 (AA)
- All semantic colors checked AA

Keyboard navigation:
- All controls reachable via Tab
- Skip-to-content links in Backstage
- Esc closes dialogs, menus
- F6 cycles between panes (grid, ribbon, formula bar, status)
- Arrow keys navigate grid; Ctrl+arrow jumps

Screen reader (NVDA/JAWS/Narrator):
- Cell role: "cell" with row/col coordinates announced
- Active cell announces value + formula + format
- Ribbon: announces tab/group/button hierarchy
- Form fields: associated <label>

Focus indicator:
- Highly visible: 2px solid #217346 ring around focused element
- Always visible (do not :focus-visible hide for keyboard users)

Touch targets:
- Minimum 44×44 px hit area (touch mode)
- Mouse mode can be smaller (24×24)
```

## Dark mode (System Setting)

```
Light theme tokens flip in dark mode:

┌────────┬──────────────┬──────────────┐
│ Token  │ Light        │ Dark         │
├────────┼──────────────┼──────────────┤
│ bg0    │ #FFFFFF      │ #1F1F1F      │
│ bg1    │ #F3F2F1      │ #2D2C2B      │
│ bg2    │ #E1DFDD      │ #3B3A39      │
│ bg3    │ #C8C6C4      │ #605E5C      │
│ fg0    │ #201F1E      │ #FFFFFF      │
│ fg1    │ #605E5C      │ #C8C6C4      │
│ fg2    │ #8A8886      │ #A19F9D      │
│ brand  │ #107C41      │ #4FBC79 (brighter green)│
│ accent │ #217346      │ #50B879      │
└────────┴──────────────┴──────────────┘

Dark mode auto-detect:
- Windows: HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Themes\Personalize\AppsUseLightTheme
- Toggle via File → Account → Office Theme: Use system / White / Colorful / Dark Gray / Black

Cells maintain user's chosen fg/bg in dark mode (don't auto-flip user data colors).
```

## Component library inventory (for Slave to implement)

```
Core widgets:
1. Button (default/primary/danger/icon-only/dropdown)
2. Input (text/number/search/password)
3. Combo (editable + readonly)
4. Checkbox & Radio
5. Toggle switch
6. Slider (zoom, color)
7. Dropdown menu
8. Tab (top-level ribbon + dialog tabs)
9. Tree view (file explorer, function categories)
10. List view (font picker, recent files)
11. Table widget (Name Manager, Find All result)
12. Tooltip (simple) + ScreenTip (rich)
13. Toast notification (saved, error)
14. Spinner / progress bar
15. Dialog (modal + non-modal)
16. Pane (side dock — Format/Comments/Field List)
17. Card (Linked Data Type, Comment thread)
18. Color picker (theme aware)
19. File chooser (Open/Save)
20. Floating action menu (Mini Toolbar, Quick Analysis)
```

## Reference images folder

```
docs/specs/_assets/ contains:
- Excel official screenshots (where available)
- Annotated mockups
- Color swatches
- Icon set previews
- Component reference mockups

When implementing, Slave should:
1. Check ux-flows/ for visual UX flow
2. Check _assets/ for reference image
3. Check this spec (50) for design system tokens
4. Apply tokens consistently
```

## Implementation hints cho Slave

- **CSS-like theme in Qt**: use `QSS` (Qt Style Sheets) for tokens; centralize in `app.qss`.
  ```css
  /* Tokens defined as Qt properties */
  * { font-family: "Segoe UI Variable"; font-size: 9pt; }
  
  QPushButton { 
    background: #FFFFFF; 
    border: 1px solid #C8C6C4; 
    padding: 6px 12px;
    color: #201F1E;
  }
  QPushButton:hover { background: #F3F2F1; }
  QPushButton:pressed { background: #E1DFDD; }
  QPushButton[primary="true"] { 
    background: #217346; 
    color: #FFFFFF; 
  }
  ```
- **Theme system**: load `themes/light.qss` or `themes/dark.qss` based on system setting; expose toggle in File → Account.
- **Color tokens in Python**:
  ```python
  class Tokens:
      BRAND = "#107C41"
      ACCENT = "#217346"
      BG0 = "#FFFFFF"
      # ...
  ```
- **Fonts**: bundle Aptos Narrow + Aptos Display in installer; register on app start with `QFontDatabase.addApplicationFont()`.
- **Icons**: bundle Fluent UI SVG set; load via `QIcon.fromTheme()` or custom resource path.
- **Animations**: `QPropertyAnimation` for smooth transitions; check `QGuiApplication.styleHints().showAnimations()` for reduced motion.
- **Accessibility**: implement `QAccessibleInterface` for custom widgets; set `accessibleName`, `accessibleDescription`.
- **Dark mode**: poll Windows registry on app startup + listen to `WM_SETTINGCHANGE`; swap theme accordingly.
- **Token documentation**: keep this spec (50) as single source of truth; update tokens here, propagate to qss files.
