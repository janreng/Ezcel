# SPEC 58 — Office Add-ins / AppSource Gallery

## Mục tiêu
Insert → Get Add-ins → AppSource store (Office.js web add-ins) + My Add-ins (installed) + Admin-managed (organization).

## Trạng thái hiện tại
- ✗ Chưa có.

## ⚠ Phân biệt 3 loại Add-ins

| Loại | Tech | Cài qua | Spec |
|---|---|---|---|
| **Office Web Add-ins** (Office.js) | HTML+JS+CSS task pane | Insert → Get Add-ins → AppSource | [Spec 58] (đây) |
| **COM Add-ins** | Compiled DLL (Win-only) | File → Options → Add-ins → Manage COM Add-ins | Out of scope (Win-only, legacy) |
| **Excel Add-ins (XLAM)** | `.xlam` macro file | File → Options → Add-ins → Manage Excel Add-ins | Phase muộn ([Spec 21](21-vba-macro.md)) |

Office Web Add-ins là **modern + cross-platform** (Web/Win/Mac/iPad) → ưu tiên implement nếu có resource.

## 58.1 Insert → Get Add-ins (Office Add-ins dialog)

Modal dialog 3 tab:

```
┌─ Office Add-ins ───────────────────────────────────────┐
│ [Store] [My Add-ins] [Admin Managed]                    │
│ ──────────────────────────────────────────────────────│
│ Categories: Editor's Picks / Productivity / Data /     │
│             Financial Management / Marketing / ...      │
│ Search: [____________________________________________] │
│ ──────────────────────────────────────────────────────│
│ ┌─────────────┐ ┌─────────────┐ ┌─────────────┐       │
│ │ Add-in 1    │ │ Add-in 2    │ │ Add-in 3    │       │
│ │ [thumbnail] │ │ [thumbnail] │ │ [thumbnail] │       │
│ │ Vendor      │ │ Vendor      │ │ Vendor      │       │
│ │ ★ 4.5 / 5   │ │ ★ 4.2 / 5   │ │ ★ 4.8 / 5   │       │
│ │ [Add]       │ │ [Add]       │ │ [Add]       │       │
│ └─────────────┘ └─────────────┘ └─────────────┘       │
└─────────────────────────────────────────────────────────┘
```

### Tab Store (AppSource)
- Cloud-hosted catalog (Microsoft AppSource).
- Categories left sidebar.
- Search box.
- Add-in cards: thumbnail / name / vendor / rating / price (Free / Paid).
- Click card → details modal: description / screenshots / permissions required / vendor info / Add button.

### Tab My Add-ins
- Currently installed add-ins (per user).
- Status: Active / Inactive (disable toggle).
- Remove button.

### Tab Admin Managed
- Organization-deployed add-ins (M365 admin centrally pushed).
- User không remove được; chỉ disable per workbook.

## 58.2 Add-in lifecycle

### Install
1. Click Add → AppSource login (Microsoft account).
2. Permissions consent dialog → user grant.
3. Download manifest XML + cache to `%AppData%\Microsoft\Office\AddIns\`.
4. Add-in available trong My Add-ins.

### Activate
- Add-in xuất hiện trong ribbon (nếu manifest declare ribbon button) hoặc Insert → My Add-ins → click → task pane mở.
- Task pane = right side pane (`QDockWidget`) chứa add-in HTML rendered trong WebView.

### Remove
- My Add-ins tab → ⋯ → Remove. Manifest deleted; ribbon button removed.

## 58.3 Add-in capabilities (Office.js API)

Add-in can:
- Read/write workbook content qua Office.js (`Excel.run(...)`).
- Subscribe events (selection change, data change).
- Show task pane / content add-in (embedded shape) / dialog.
- Call external HTTP APIs.
- OAuth single sign-on với host Office.

## 58.4 Manifest types

### XML manifest (legacy + GA)
- File `manifest.xml` declare metadata + capabilities + permission scope.
- Hosted ở web URL hoặc local file.

### Unified Manifest for Microsoft 365 (JSON, 2024-2026)
- File `manifest.json` — match **Microsoft Teams app manifest schema** → Office add-ins có thể bundle chung gói với Teams apps + Copilot agents + Microsoft 365 apps thành **một app duy nhất** trên admin center.
- Trạng thái rollout (cập nhật Microsoft Learn 2026):
  - **Outlook add-ins**: unified manifest **GA production** (cả developers + AppSource).
  - **Excel + Word + PowerPoint add-ins**: unified manifest còn **preview**.
- Format JSON khác hẳn XML legacy về schema; có tool migrate `office-addin-manifest-converter` (npm).
- Lợi ích M365 deploy: 1 manifest deploy đồng thời Outlook + Teams + Excel; share permission, branding, ribbon entries.
- Future direction Microsoft: dần phase-out XML → JSON unified; chưa có timeline deprecation cụ thể.

### "Available Apps" list (M365 admin center)
- App-store unified hiển thị: Teams apps + Copilot agents + Office Add-ins + M365 apps trong cùng 1 grid.
- Mỗi entry dùng unified manifest → admin deploy 1 lần áp tất cả host.

### Sideload
- Developer testing: File → Options → Trust Center → Trusted Add-in Catalogs → add local URL.

## 58.5 Permission scopes

3 levels (declare trong manifest):
- **Restricted**: cell value read-only.
- **ReadDocument**: read all workbook content.
- **WriteDocument**: read + write.

User thấy permission trong Add dialog → consent required.

## 58.6 Popular add-ins (reference)

- **Wolfram Alpha** — computational data.
- **People Graph** — visualization.
- **Bing Maps** — geo plotting.
- **JotForm** — embedded forms.
- **DataXL / Power Tools** — productivity.
- **Solver Foundation** — optimization.

Custom enterprise add-ins: organization-specific.

## Implementation note

**Phase rất muộn / out of scope MVP.** Reasons:
- Office.js runtime requires Microsoft cloud auth (AppSource login).
- Embedded WebView (QWebEngineView dependency — heavy ~150MB).
- Manifest validation parser.
- API surface compatibility với Office.js (hàng nghìn methods).

### Possible alternative for Ezcel
1. **Plugin system custom** (không phải Office.js):
   - Python plugins (đã có Python macro [Spec 21](21-vba-macro.md)).
   - Manifest: `plugin.json` (name, ribbon entries, permissions, entry point Python module).
   - Distribution: `.ezpkg` zip (Python files + assets + manifest).
   - Catalog: optional Ezcel-hosted gallery hoặc GitHub repos.

2. **Office.js compatibility layer** (very high effort):
   - Map Office.js calls → Ezcel API internal.
   - Reuse AppSource catalog (nếu compatibility cao).

→ Khuyến nghị approach 1 (Python plugins) cho Ezcel; approach 2 reserved cho enterprise edition.

## Acceptance criteria

### If Office.js compatibility
1. Insert → Get Add-ins → AppSource browse + search.
2. Click "Wolfram Alpha" → details → Add → Microsoft account login → consent → cài.
3. Add-in xuất hiện trong My Add-ins + ribbon entry (nếu manifest declare).
4. Click ribbon button → task pane mở với add-in UI.

### Ezcel Python plugin alternative
1. Insert → Get Plugins → Ezcel plugin catalog (GitHub).
2. Click plugin → Add → download `.ezpkg` → install → ribbon entry xuất hiện.
3. Click → task pane Python plugin chạy.

## Phụ thuộc
- [21 Macro (Python)](21-vba-macro.md) — alternative plugin system.
- [07 Ribbon](07-ribbon.md) — plugin ribbon entries.
- [49 Trust Center](49-trust-center-privacy.md) — add-in trust settings.

## Risk
**Out of scope MVP.** Implement Office.js compatibility cực lớn. Python plugin alternative khả thi nhưng cần plugin system riêng — Phase 7+.

## Phase
Out of scope or Phase 7+ (Python plugin alternative).
