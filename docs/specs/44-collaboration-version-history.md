# SPEC 44 — Co-authoring / Version History / Track Changes / Share

## Mục tiêu
Real-time collaboration (multi-user edit), Version History (timeline restore), Share (link + permissions). Track Changes legacy chỉ giữ làm reference.

## Trạng thái hiện tại
- ✗ Chưa có toàn bộ.

## 44.1 Co-authoring (Real-time multi-user)

### Excel 365 behavior
- File trên OneDrive/SharePoint → multiple users edit cùng lúc.
- Mỗi user có **presence indicator** (avatar + flag colored ở cell selection).
- Edits **broadcast real-time** sang user khác.
- Conflict resolution: OT (Operational Transform) hoặc CRDT.
- Cursor sharing: thấy selection của user khác.

### Excel 365 features
- Status Bar hiển thị danh sách user đang xem/edit.
- Click avatar → jump đến cell user đó đang select.
- Chat pane (in Excel for Web) cho team discussion.

## 44.2 Ezcel approach

**Phase rất muộn / Out of scope MVP.** Requires:
- Backend server (sync, auth, conflict resolution).
- Cell-level diff protocol over WebSocket.
- CRDT or OT algorithm.

### Possible incremental approach
1. **Phase A: Local diff format**. Save diff log thay vì full file → smaller commits, support undo across sessions.
2. **Phase B: Git-based**. Workbook = folder of sheet xlsx + commit diffs. User dùng git merge thủ công.
3. **Phase C: Backend sync**. Real-time server (Phase rất xa).

## 44.3 Version History

### Excel 365: File → Info → Version History
- Pane hiển thị danh sách versions (timestamp + user).
- Click version → mở read-only preview.
- "Restore" button → revert workbook về version đó.

### Ezcel implementation
- **Phase 5+**. Khả thi mà không cần backend.
- Mỗi save → tạo snapshot `{filename}_v{N}_{timestamp}.xlsx.bak` trong `.ezcel-history/` folder cạnh file.
- Cleanup policy: keep last 50 versions, hoặc theo time (last 30 days).
- UI pane: File → Info → Version History → list snapshots → preview / restore.

## 44.4 Track Changes (Legacy)

Excel 2013 trước có Track Changes (Review → Track Changes). Excel 2016+ **removed** (replaced by co-authoring + Version History).

## 44.A Merge conflict resolution (AutoSave + cloud)

Khi co-author + AutoSave bị gián đoạn (network drop, simultaneous conflicting edits) Excel detect merge conflict + cho phép user resolve thủ công.

### Flow
1. Co-authoring đang diễn ra; user A edit cell D5 = 100, user B (offline temporary) edit D5 = 200.
2. User B reconnect → Excel detect conflict → yellow bar:
   ```
   ┌────────────────────────────────────────────────────────┐
   │ ⚠ MERGE CONFLICT  Some of your changes could not be    │
   │ merged automatically.   [ Resolve ]   [ Save a copy ] │
   └────────────────────────────────────────────────────────┘
   ```
3. Click **Resolve** → mở Conflicts tab:
   ```
   ┌─ Conflicts ──────────────────────────────────────────┐
   │ Conflicts: 3                          [Accept all mine]│
   │ ────────────────────────────────────────────────────│
   │ Sheet1!D5                                              │
   │   Your value: 200                                      │
   │   Server value: 100  (by alice@…, 14:30)               │
   │   [Use mine] [Use theirs]                              │
   │ ────────────────────────────────────────────────────│
   │ Sheet1!F12                                             │
   │   Your formula: =SUM(B:B)                              │
   │   Server formula: =SUM(B2:B100)                        │
   │   [Use mine] [Use theirs]                              │
   │ ────────────────────────────────────────────────────│
   │ Chart 'Sales' anchor                                   │
   │   Your: F2     Server: H4                              │
   │   [Use mine] [Use theirs]                              │
   │ ────────────────────────────────────────────────────│
   │                              [Done — Merge & Save]     │
   └────────────────────────────────────────────────────────┘
   ```

### Auto-backup
- Excel **tự lưu unmerged copy** trong 7 ngày tại:
  - Windows: `%localappdata%/Microsoft/Excel/TemporaryBackupFile/`
  - Mac: `~/Library/Containers/com.microsoft.Excel/Data/Library/Application Support/Microsoft/TemporaryBackupFile/`
- User có thể manual mở backup nếu cần so sánh, save copy → permanent location.

### Conflict prevention
- AutoSave luôn ON cho cloud file ([Spec 36](36-file-formats-autosave.md)).
- AutoSave OFF + co-author → cảnh báo + ép user enable.

### Ezcel approach
- Phase 6+ khi có sync backend.
- TemporaryBackupFile pattern → reuse cho AutoRecover ([Spec 36](36-file-formats-autosave.md)) nhưng key theo session ID.

## 44.B Loop components — embed Excel (2026)

Microsoft Loop tích hợp ngày càng sâu với Excel:

### Loop hierarchy
- **Component** (smallest): table / checklist / task-list / paragraph / poll — sync live wherever shared.
- **Page**: canvas chứa nhiều components.
- **Workspace**: project space group nhiều pages + permissions.

### Excel ↔ Loop
- **Export Table → Loop**: right-click Table → "Send to Loop" → component được tạo trong Loop app, mọi nơi share (Teams chat / Outlook / Word) sync cùng table.
- **Embed Loop table → Excel**: Insert tab → Loop Component → pick component từ workspace → embed; bidirectional sync (edit trên Excel → update Loop, vice versa).
- **Teams Collaborative Notes** (Feb 2026 rollout): meeting notes là Loop components, có thể chứa Excel table snippet — sync với Excel file gốc.

### Excel export from Loop
- Loop table → right-click → "Open in Excel" hoặc "Download as CSV".

### Ezcel approach
- Out of scope MVP (đòi hỏi Microsoft Graph + Loop API tích hợp).
- Phase 8+: implement opt-in connector → push/pull table data với Loop workspace REST API.

## 44.5 Show Changes — extended history (May 2026, Web)

Microsoft mở rộng **Show Changes** Excel for Web từ **30 ngày** → **1 năm** retention.

### Show Changes pane
- Review → Show Changes → pane phải.
- List changes timeline: ai sửa gì, khi nào, range.
- Filter: by sheet / range / user / time window.
- Click change → grid scroll + highlight cell + tooltip diff.

### Copilot attribution (May 2026)
- Khi collaborator dùng Edit with Copilot, change card có **Copilot icon + visual flag**.
- Hover → "Edited with Copilot by <user> on <date>" + prompt summary nếu user share.
- One-turn Copilot indicator (sau prompt): xem [Spec 39 §39.3.2](39-copilot-agent.md).

### Ezcel implementation
- Phase 5+. Khả thi standalone (không cần backend) qua diff log:
  - Mỗi save event ghi diff vào `.ezcel-history/changes.jsonl` (one JSON per line, append-only).
  - Schema: `{ts, user, sheet, range, before, after, source: "user"|"copilot"|"python_macro", copilot_prompt?: str}`.
  - 1 year retention: rotate sang `.ezcel-history/changes-{YYYY-Q}.jsonl.gz` quarterly + auto-delete > 4 quarters.
- Show Changes pane: lazy-load jsonl theo filter window.

### Behavior (legacy)
- Bật → mọi edit lưu vào "change log".
- Highlight cells bị thay đổi với border + comment "User X changed B5 from 10 to 20 on 2026-06-02".
- Accept / Reject Changes dialog.

### Ezcel approach
**Out of scope.** Modern Excel bỏ.

## 44.5 Share (File → Share)

### Excel 365
- "Share" button góc trên phải.
- Send link to: People / Group.
- Permission: Can edit / Can view / Can review.
- "Anyone with the link" / "People in your organization" / "Specific people".
- Expiration date.
- Password.
- Block download.

### Ezcel
**Out of scope** without backend. Stub button → "Share is not available in standalone mode" dialog.

Alternative: "Save to OneDrive" hyperlink (đưa user ra ngoài app).

## 44.6 Comments (Threaded — modern collaboration)

Xem [Spec 26 Comments](26-comments-notes.md). Comment thread + @mention là collaboration primitive cơ bản.

### Without backend
- Comments lưu trong workbook xlsx.
- Author = current Windows user (Settings → User Name).
- Open file → comments của user khác hiển thị.

## Acceptance criteria

### Version History (Phase 5+)
1. File → Info → Version History → pane hiển thị 5 snapshots gần nhất (Ezcel auto-save).
2. Click snapshot → mở read-only preview window.
3. "Restore this version" → workbook revert; old version snapshot trước restore.

### Co-authoring
- N/A trừ khi có backend.

### Track Changes
- N/A — không implement (legacy).

## Phụ thuộc
- [36 File Formats / AutoSave](36-file-formats-autosave.md) — AutoSave + AutoRecover infrastructure overlap.
- [26 Comments & Notes](26-comments-notes.md) — threaded comments với author info.

## Risk
- Real-time co-authoring: backend phức tạp → out of scope MVP.
- Version History local: dễ làm; chỉ cần disk space cho snapshots.

## Phase
- Version History: Phase 5+.
- Co-authoring / Share: out of scope.
- Track Changes legacy: out of scope (bỏ theo modern Excel).
