# SPEC 49 — Trust Center, Privacy & Security Settings

## Mục tiêu
File → Options → Trust Center: settings về macro security, ActiveX, protected view, privacy options, network access.

## Trạng thái hiện tại
- ✗ Chưa có.

## 49.1 Trust Center Settings

### Trusted Publishers
- List của publishers có signed certificate được trust.
- Macro signed by trusted publisher → tự động enable (skip prompt).

### Trusted Locations
- Folders mà files trong đó → tự động skip Protected View / enable macros.
- Add: path + "Subfolders of this location are also trusted" checkbox.
- Built-in trusted: `%AppData%\Microsoft\Excel\XLSTART\`, `%ProgramFiles%\Microsoft Office\Templates\`.

### Trusted Documents
- Mỗi document từng được user "Enable Content" sẽ remember + auto-enable next time.
- Clear list: button reset.

### Trusted App Catalogs
- Office Add-ins gallery URL.

### Add-ins
- Manage trust cho COM Add-ins, Excel Add-ins (`.xlam`), Office Web Add-ins.
- Disable per add-in.

### ActiveX Settings (4 levels)
- Disable all controls without notification.
- Prompt me before enabling all controls with minimal restrictions.
- Prompt me before enabling Unsafe for Initialization (UFI) controls with additional restrictions and Safe for Initialization (SFI) controls with minimal restrictions.
- Enable all controls without restrictions and without prompting (NOT RECOMMENDED).

⚠ Ezcel **không support ActiveX** ([Spec 37](37-form-controls.md)) → setting này stub disabled.

### Macro Settings (4 levels)
- Disable all macros without notification.
- Disable all macros with notification (default).
- Disable all macros except digitally signed macros.
- Enable VBA macros (NOT RECOMMENDED).
- ☑ Trust access to the VBA project object model.

Ezcel: VBA → Python ([Spec 21](21-vba-macro.md)). Setting tương đương cho Python macro:
- Disable all Python macros.
- Disable all macros with notification (default).
- Disable except signed.
- Enable all.

### Protected View

3 triggers cho Protected View (read-only sandbox):
1. **Files originating from the Internet** ✓.
2. **Files located in potentially unsafe locations** ✓.
3. **Files opened from Outlook attachments** ✓.

UI: yellow bar trên cùng "PROTECTED VIEW — Be careful — files from the Internet can contain viruses. [Enable Editing]".

### Message Bar
- Settings để hiển thị / hide security alerts.

### File Block Settings
- Block opening / saving của file types cụ thể:
  - Excel 4 Workbooks (`.xlw`)
  - Excel 95 Workbooks
  - Excel 97-2003 (`.xls`)
  - dBase III/IV
  - XML 2003
  - Web Pages
  - ...
- Default: Excel 4 + dBase block (security history).

### Privacy Options
- ☑ Enable optional connected experiences (Insights, Recommended Pictures).
- ☑ Enable experiences that analyze content.
- ☑ Enable experiences that download online content.
- ☑ Send Microsoft information about how I use Office (telemetry).
- "Microsoft Privacy Statement" link.

### Form-based Sign-in
- Manage trusted authentication URLs.

### External Content
- Security settings for Workbook Links (other workbooks):
  - Enable automatic update for all Workbook Links (not recommended).
  - Prompt user on automatic update for Workbook Links.
  - Disable automatic update of Workbook Links.
- Linked Data Types ([Spec 38](38-linked-data-types.md)) network access toggle.
- Dynamic Data Exchange (DDE) — legacy, off by default.

## 49.2 Privacy Dashboard

File → Account → Manage Settings → Privacy → web page.

Sections:
- Required diagnostic data toggle.
- Optional diagnostic data toggle.
- Connected experiences (3 categories).
- AI-based experiences (Copilot — [Spec 39](39-copilot-agent.md)).

## 49.3 Sensitivity Labels (Microsoft Purview Information Protection)

Enterprise feature integrated với Microsoft Purview Information Protection (MIP). Cho phép gắn nhãn classification + protection level vào workbook.

### UI access
- Home tab → **Sensitivity** dropdown (bên cạnh File button).
- Labels publish bởi admin qua Purview compliance portal — user chọn từ list:
  - **Public** (no protection)
  - **Internal**
  - **Confidential**
  - **Highly Confidential**
  - **Confidential / Attorney Privileged** (custom per tenant)
- Mỗi label có settings: encryption / content marking (watermark / header / footer) / access scope (user/group/anyone in org/external).

### Behavior khi áp label
- **Encryption**: workbook encrypted with rights-management; chỉ user trong scope mở được. Khác hẳn `Encrypt with Password` ([Spec 29](29-protection.md)) — RMS-based, no password.
- **Content marking**: watermark "CONFIDENTIAL" ngang giữa sheet; header/footer auto.
- **Restrictions**: per-label có thể disable Copy / Print / Forward.

### File format support
- **Support**: Open XML (`.xlsx`, `.xlsm`, `.xlsb`).
- **KHÔNG support**: legacy `.xls`, `.ods`, hoặc other formats.

### Co-authoring
- Co-authoring **enabled** trên file gắn sensitivity label encrypted (Microsoft đã enable từ 2023+) — tenant phải opt-in trong Purview admin.

### Status bar indicator
- Bottom-right status bar có badge `🛡 Confidential` khi label áp.

### Ezcel approach
- **Out of scope MVP** — đòi hỏi MS Entra ID + Purview tenant integration.
- Stub UI: dropdown "Sensitivity" greyed với tooltip "Available when signed in with Microsoft 365 Enterprise account".
- Document trong code rằng file đã có label sẽ:
  - Open: prompt sign-in nếu Ezcel sau này tích hợp MSAL.
  - Save: preserve OOXML `<sensitivityLabel>` element round-trip.

## 49.4 Connected experiences — "send content to Microsoft" toggle

Privacy control quan trọng cho enterprise/Copilot users:
- File → Options → Trust Center → Privacy → "Allow Office to send content to Microsoft for analysis".
- **Off** → disable: DLP policy tips Outlook, auto/recommended labeling, **Microsoft 365 Copilot** (Copilot KHÔNG hoạt động), Smart Lookup, Translate, Designer.
- **On** (default) → tất cả connected services hoạt động.

Ezcel: 1 master toggle Privacy → khi off → disable Copilot, Translate, Smart Lookup, Linked Data Types đồng loạt.

## 49.5 File Block — External Workbook Links update (Oct 2025 → Jul 2026 rollout)

Microsoft đã thêm **`FileBlockExternalLinks`** group policy — mở rộng File Block Settings ra cho **external workbook links**. Rollout: Oct 2025 → Jul 2026.

### Behavior
- Excel workbook reference đến `.xls` (BIFF8) / `.xlsm` macro / `.xlsb` / `.dbf` / `.xlw` qua external link → trả `#BLOCKED!` error hoặc refresh fail.
- Mục đích: chặn attack vector qua link → format risky.
- User vẫn có thể override per-file qua Trust Center → File Block Settings hoặc Enable Editing dialog.

### UI Trust Center → File Block

Bảng các format có 2 cột mới:
- **Open**: behavior khi mở file format đó (Block / Block + Save in Protected View / Open in Protected View / Allow).
- **Save**: Block / Allow.
- **NEW Linked**: behavior cho external link tới format này (Block / Prompt / Allow).

### Ezcel
- Implement `#BLOCKED!` error code trong calc engine ([Spec 35](35-calculation-engine.md)).
- File Block Settings table: thêm cột "External link" với 3 options.
- Default block list: `.xls`, `.xlw`, `.dbf` (theo Microsoft default).

## Ezcel implementation

### Minimal viable
- Settings dialog Privacy section:
  - ☑ Enable internet features (Translate, Smart Lookup, Linked Data Types, Copilot).
  - ☑ Enable telemetry.
  - Default: opt-IN ban đầu (lần mở app đầu → dialog "Enable connected experiences?").
- Trusted Locations: list folders; new file in trusted → skip Protected View dialog.
- Protected View: nếu file path không trong trusted locations + file vừa download (zone identifier `Zone.Identifier` trên Windows NTFS) → mở read-only với yellow bar "Enable Editing".

### Macro Settings (Python)
- 4 levels như trên.
- Default: "Disable all macros with notification" — file có script → yellow bar.

### File Block
- Implement read-only mode cho `.xls` (BIFF8) — không write.
- Path-based block: extension trong block list → dialog warning.

## Acceptance criteria
1. File → Options → Trust Center → Trust Center Settings → tabs Privacy / Trusted Locations / Macros / Protected View visible.
2. Add `C:\TrustedFolder\` to Trusted Locations → file trong folder mở normal, không có yellow bar.
3. Open file từ Downloads folder (Zone.Identifier) → yellow bar "PROTECTED VIEW — Enable Editing".
4. Macro setting "Disable with notification" → file có Python macro → yellow bar "Macros are disabled. Enable Content".
5. Privacy → disable internet features → Translate / Linked Data Types / Copilot grayed out.
6. Setting persist via QSettings — restart app, settings retained.

## Phụ thuộc
- [21 Macro (Python)](21-vba-macro.md) — Macro settings.
- [38 Linked Data Types](38-linked-data-types.md), [39 Copilot](39-copilot-agent.md) — connected experiences.
- [36 File Formats](36-file-formats-autosave.md) — File Block.

## Risk
Thấp-trung bình. Mostly UI + state. Protected View "sandbox" thực sự khó (cần process isolation) → bắt đầu với read-only mode đủ.

## Phase
Phase 6+ (sau Macro implementation).
