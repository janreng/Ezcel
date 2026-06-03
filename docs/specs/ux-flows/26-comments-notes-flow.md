# UX Flow — Spec 26 Comments & Notes

> Spec gốc: [../26-comments-notes.md](../26-comments-notes.md)

## Comments vs Notes (modern Excel)

```
Comments (modern, threaded):
- Conversation-style with @mentions
- Reply, Resolve, Delete
- Sync to OneDrive/Teams
- Author + avatar + timestamp
- Modern indicator: purple top-right triangle

Notes (legacy, formerly "Comments" pre-2019):
- Single block of text
- No threading
- Author auto-prepended (bold)
- Yellow sticky-note appearance
- Indicator: red top-right triangle

Both can coexist on same cell.
```

## Cell indicators

```
Cell B5 with both Comment + Note:
┌────────────────┐
│ Value      ▲▼  │  ← ▲ red = Note, ▼ purple = Comment
└────────────────┘

Cell with Note only:
┌────────────┐
│ Value   ▲  │  ← red triangle top-right
└────────────┘

Cell with Comment only:
┌────────────┐
│ Value   ▼  │  ← purple/violet triangle top-right
└────────────┘

Hovering over indicator → shows content as tooltip
```

## New Comment flow (modern threaded)

```mermaid
sequenceDiagram
    actor User
    participant Cell
    participant Pane as Comments Pane
    participant Bubble as Comment Bubble
    
    User->>Cell: Right-click B5 → New Comment
    Note over Cell: Or Ctrl+Shift+M, or Review → New Comment
    
    Cell->>Bubble: Show floating comment input bubble
    Bubble->>User: Show text area + "Post" / "Cancel"
    
    User->>Bubble: Type "@Hoang please review this number"
    
    Bubble->>Bubble: Show @-mention autocomplete dropdown
    Bubble->>User: List: Hoang Tran, Hoa Nguyen, etc.
    
    User->>Bubble: Select Hoang
    Bubble->>User: Mention chip inserted, plus notification opt-in
    
    User->>Bubble: Click "Post"
    Bubble->>Cell: Save comment; close bubble
    Cell->>Cell: Add purple triangle indicator
    Cell-->>Pane: Update Comments Pane if open
    
    Note over Pane: @Hoang gets email/Teams notification
```

## Modern comment bubble

```
After right-click → New Comment, floating bubble appears:

           ┌────────────────────────────┐
           │ 🎯 Add a comment for everyone │
           │ ─────────────────────────── │
           │ ┌──────────────────────────┐│
           │ │ @Hoang please review     ││
           │ │ this number              ││
           │ │                            ││
           │ └──────────────────────────┘│
           │                              │
           │ [@] [😊] [📎]    [Cancel] [Post] │
           └──────────────────────────────┘
            ╲
             ╲
              ▼
           ┌────┐
           │ B5 │ ← anchored to cell
           └────┘
```

## @-mention autocomplete

```
User types "@h" in comment:

┌──────────────────────────────┐
│ @h|                            │
│ ─────────────────────────────│
│ ┌──────────────────────────┐ │
│ │ 👤 Hoang Tran            │ │
│ │    hoang@company.com      │ │
│ ├──────────────────────────┤ │
│ │ 👤 Hoa Nguyen            │ │
│ │    hoa@company.com        │ │
│ ├──────────────────────────┤ │
│ │ 👤 Hai Tran              │ │
│ │    hai@company.com        │ │
│ └──────────────────────────┘ │
│                                │
│ [@] [😊] [📎]   [Cancel] [Post]│
└────────────────────────────────┘

Select user → "@HoangTran" chip inserted with avatar
- If user lacks access → option to share workbook with them
```

## Comment thread view

```
After post, click cell B5 → comment thread expands:

           ┌──────────────────────────────────┐
           │ 💬 Thread (2)                     │
           │ ─────────────────────────────── │
           │ 👤 Giang Nguyen           2h ago │
           │    @Hoang please review this num. │
           │                                    │
           │    💬 Reply                        │
           │ ─────────────────────────────── │
           │ 👤 Hoang Tran           45min ago │
           │    Looks correct to me.            │
           │                                    │
           │    💬 Reply                        │
           │ ─────────────────────────────── │
           │ [Type a reply...                 ] │
           │                                    │
           │ [✓ Resolve thread]  [⋯ More]      │
           └──────────────────────────────────┘
            ╲
             ▼
            B5
```

## Comments Pane (Review → Comments)

```
┌─ Comments ──────────────────────────────────┐
│ [+ New]                  [⚙ Filter] [✕ Close]│
│ ─────────────────────────────────────────── │
│ Sort: [Newest first ▼]   Show: [All ▼]      │
│ ─────────────────────────────────────────── │
│                                                │
│ Sheet1!B5                                      │
│ 👤 Giang  @Hoang please review...     2h ago │
│ ├─ 👤 Hoang  Looks correct           45min ago│
│ └─ 💬 Reply...                                │
│ [✓ Resolve]                                    │
│ ─────────────────────────────────────────── │
│                                                │
│ Sheet1!D12                                     │
│ 👤 Trang  Confirm formula here?       1h ago │
│ └─ 💬 Reply...                                │
│ [✓ Resolve]                                    │
│ ─────────────────────────────────────────── │
│                                                │
│ Sheet2!A1                                      │
│ 👤 Lan  Should we hide this row?      30m ago │
│ └─ 💬 Reply...                                │
│ [✓ Resolve]                                    │
│                                                │
│ ✅ Resolved (collapsed)                        │
│ ▶ Sheet1!F22  · 1 resolved thread            │
└────────────────────────────────────────────────┘

Click any thread → grid jumps to that cell + highlights
```

## Note flow (legacy)

```mermaid
sequenceDiagram
    actor User
    participant Cell
    participant Note as Note Box
    
    User->>Cell: Right-click B5 → New Note
    Note over Cell: Or Shift+F2, or Review → Notes → New Note
    
    Cell->>Note: Show yellow sticky-note overlay above cell
    Note->>User: "Giang:" (auto-prefilled author bold)
    
    User->>Note: Type note text
    User->>Cell: Click outside → save & close
    
    Cell->>Cell: Add red triangle indicator
    
    User->>Cell: Hover B5 → Note shows as tooltip
    User->>Cell: Right-click → Edit Note → modify
    User->>Cell: Right-click → Delete Note → remove
```

## Legacy note appearance

```
            ┌──────────────────┐
            │ Giang:            │ ← author auto bold
            │ This number from   │
            │ Q3 report, page 5. │
            │                    │
            └──────────────────┘
             ╲
              ╲
               ▼
            ┌────┐
            │ B5 │ ← anchor; small leader line
            └────┘ ▲ ← red triangle indicator top-right
```

## Show/Hide all notes/comments

```
Review tab → Notes ▼:
┌─────────────────────────────┐
│ ➕ New Note (Shift+F2)       │
│ 📝 Edit Note                  │
│ ❌ Delete Note                │
│ 👁 Show/Hide Note             │
│ 👁 Show All Notes              │ ← show all permanently (not on hover)
│ 🔄 Convert to Comments        │ ← bulk migration to threaded
└──────────────────────────────────┘

Review tab → Comments ▼:
┌─────────────────────────────┐
│ ➕ New Comment (Ctrl+Shift+M) │
│ 🗑 Delete Comment              │
│ ▶ Previous Comment             │
│ ◀ Next Comment                  │
│ 👁 Show Comments Pane           │
│ ✓ Resolve Comment              │
│ 🔄 Reopen Resolved              │
└──────────────────────────────────┘
```

## Print comments option

```
Page Setup → Sheet tab → Print group → Comments dropdown:

[At end of sheet         ▼]
  ├ (None)
  ├ At end of sheet      ← appendix list
  ├ As displayed on sheet ← inline near cells
  
Effect:
- "At end of sheet": adds extra page listing all comments + cell refs
- "As displayed": prints visible comment boxes overlaid on grid
```

## Implementation hints cho Slave

- **Two separate data stores**:
  ```python
  sheet._notes: dict[(r, c), Note]
  sheet._comments: dict[(r, c), CommentThread]
  
  class CommentThread:
      messages: list[Message]
      resolved: bool
      mentions: list[str]
      
  class Message:
      author: User
      body: str  # markdown w/ mention syntax
      created_at: datetime
      edited_at: datetime | None
  ```
- **Indicators**: render triangles in `CellDelegate.paint()`:
  - Note triangle: red (#C00000), 5x5 px top-right.
  - Comment triangle: purple (#7030A0), 5x5 px top-right (offset if both present).
- **Tooltip on hover**: `QToolTip.showText()` with formatted content; for threaded comment → show first message + "+N replies".
- **Modern comment bubble**: `QFrame` floating overlay; non-modal, accepts input even with grid active.
- **@-mention**: `QCompleter` triggered after "@"; data from User directory (local users / M365 directory).
- **Comments Pane**: `QDockWidget` right; build tree with `QTreeWidget` (Sheet → Thread → Messages).
- **Resolve**: set `resolved=True`; collapse in pane; hide indicator (still in data).
- **Sync (M365)**: if signed in → POST comment changes to Microsoft Graph API; receive realtime updates via WebSocket / SignalR.
- **Conversion**: Convert Note → Comment = create thread with single message, author = note author, body = note text, delete note.
- **Print**: extend print pipeline (Spec 24) to optionally render comment block per page or appendix.
