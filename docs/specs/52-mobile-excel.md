# SPEC 52 — Excel Mobile (iOS / iPad / iPhone / Android)

## Mục tiêu
Touch-first UX cho mobile Excel — gesture, layout adaptive, accessory support. Áp dụng nếu/khi Ezcel có port mobile; hiện tại **out of scope MVP** nhưng spec làm base.

## Trạng thái hiện tại
- ✗ Ezcel là PySide6 desktop — không có mobile port.
- Spec này là **reference + future-planning**: nếu sau này port qua iOS/Android (Qt for Mobile / Flutter / native), pattern UX phải khớp Excel mobile.

## 52.1 Touch Gestures (chung iPad + iPhone)

| Gesture | Hành vi |
|---|---|
| **Single tap** cell | Select cell (active) |
| **Double tap** cell | Open cell for entry, hiện keyboard |
| **Tap + hold** cell | Context menu (giống right-click desktop) |
| **Pinch** | Zoom in/out (10%-400%) |
| **Spread** (pinch out) | Zoom in |
| **Two-finger drag** | Scroll grid |
| **Single finger drag** trên cell | Select range (sau khi tap-hold initiate) |
| **Tap selection handles** + drag | Mở rộng selection (top-left và bottom-right handles xuất hiện sau khi select) |
| **Swipe left/right** trên sheet tab bar | Navigate sheets |
| **Three-finger swipe up** | Undo (iOS gesture) |
| **Three-finger swipe down** | Redo (iOS gesture) |
| **Shake device** | Undo (legacy iOS) |
| **Long-press column/row header** | Resize handle xuất hiện |

## 52.2 iPad-specific

### Layout
- Ribbon hiển thị **full** nhưng compact (touch target 44pt minimum).
- Split View / Slide Over support (multitask với app khác).
- External keyboard (Magic Keyboard / Smart Folio Keyboard): hầu hết Windows shortcut work với Cmd thay Ctrl.

### Apple Pencil support
- **Inking**: vẽ trên grid như stylus pen ([Spec 48](48-touch-pen-ink.md)).
- **Ink to Math**: vẽ formula → recognize → typeset.
- **Ink to Shape**: vẽ rough shape → clean shape.
- **Scribble**: viết tay vào cell → recognize text → fill cell value.
- **Annotations**: review/comment dùng pencil draw markup.

### Trackpad support (Magic Keyboard/iPad Air+)
- Cursor di chuyển như chuột.
- Multi-touch trackpad: 2-finger swipe scroll, pinch zoom.
- Right-click = 2-finger click hoặc Ctrl + click.

## 52.3 iPhone-specific

### Layout
- Ribbon **collapsed** thành bottom toolbar (5-6 essential icons).
- Cell content panel **fullscreen** khi tap để edit.
- Auto-rotate landscape cho wide tables.
- Sheet tab bar collapsed sang menu icon.

### iPhone unique features
- **Capture data from picture** (Excel mobile feature unique): chụp ảnh bảng giấy/printed table → OCR + structure recognition → insert vào sheet như Table. Insert tab → "Data from Picture" button.
- **Quick formula**: thanh số trên keyboard có common functions (SUM, AVG, MIN, MAX).
- **Voice input**: dictate cell content qua microphone.

## 52.4 Android-specific

Tương tự iOS nhưng:
- KHÔNG có Apple Pencil — thay bằng Samsung S Pen (Galaxy Note/Tab) với gestures riêng.
- Material Design 3 layout (thay vì iOS layout).
- Android-specific share sheet integration.

## 52.5 Mobile-only feature limitations

### Hạn chế trên mobile
- **VBA Macros**: không chạy (security + perf). Office Scripts cũng không.
- **Advanced Conditional Formatting**: tạo CF rule mới hạn chế; edit rule cũ OK.
- **PivotTable**: create hạn chế; refresh existing OK.
- **Power Query / Power Pivot**: không có.
- **Complex Charts**: subset (Column/Bar/Line/Pie/Area), không có Waterfall/Funnel/Histogram editor.
- **Add-ins**: phụ thuộc compatibility từng add-in.
- **Print**: limited cho mobile printer protocols.

### Best for mobile (per Microsoft positioning)
- Reviewing data.
- Quick edits cells.
- Real-time collaboration (Comments / Co-authoring qua OneDrive).
- Sharing.
- KHÔNG nên cho complex authoring/analysis — dùng desktop.

## 52.6 File access

- iCloud Drive (iOS) / Google Drive (Android) / **OneDrive** (primary) / SharePoint.
- Local file storage limited (sandbox per app).
- Open from another app via Share Sheet → "Open in Excel".

## 52.7 Mobile-specific UI patterns

- **Floating Action Button (FAB)**: tạo cell/row mới (Android Material).
- **Bottom Sheet Modal**: format options, filter options thay vì dropdown panel desktop.
- **Pull to refresh**: cập nhật co-authoring sync.
- **Swipe action** trên cell: swipe left → delete; right → format.

## Implementation note (nếu port)

### Tech stack options
1. **PySide6 + Qt for Mobile**: cùng codebase desktop. Hạn chế: Qt mobile UX không native, scroll/gesture phải tự handle.
2. **Flutter**: rewrite UI, share business logic qua Python via channel. Native feel tốt hơn.
3. **React Native + JS engine**: rewrite hoàn toàn; formula engine port sang JS hoặc bridge.
4. **Native (Swift iOS / Kotlin Android)**: best UX, max effort.

Khuyến nghị: stick PySide6 desktop, **không port mobile MVP**. Re-evaluate sau khi desktop ổn.

## Acceptance criteria (nếu/khi port)

### iPad
1. Tap A1 → select A1 (active border xanh #217346).
2. Double-tap A1 → keyboard hiện, cursor trong cell.
3. Tap-hold A1 → mini context menu (Cut/Copy/Paste/Format).
4. Pinch out → zoom in; pinch in → zoom out.
5. 2-finger drag → scroll.
6. Apple Pencil draw trên grid → ink stroke render.
7. Magic Keyboard Cmd+B → Bold active cell.

### iPhone
1. Tap → select; double tap → edit.
2. Bottom toolbar 5 icons: New / Open / Save / Format / Insert.
3. Tap Insert → bottom sheet modal với Cell Controls / Picture / Chart / ... options.
4. Insert → Data from Picture → camera → capture table → preview structured data → Insert.
5. Three-finger swipe up → Undo.

## Phụ thuộc
- [48 Touch Mode / Pen / Ink](48-touch-pen-ink.md) — overlap heavy.
- [50 Design System](50-design-system.md) — token size + touch target ≥ 44pt.
- [44 Collaboration](44-collaboration-version-history.md) — real-time co-auth quan trọng hơn ở mobile.

## Risk
**Out of scope MVP.** Nếu sau port: effort lớn (UX rewrite + gesture handling + accessory APIs).
