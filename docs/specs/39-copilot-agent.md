# SPEC 39 — Copilot trong Excel (Pane, COPILOT function, Agent Mode, Formula Completion)

## Mục tiêu
AI assistant tích hợp sâu vào Excel theo cập nhật 2024-2025: side pane chat, in-formula `=COPILOT(...)`, Agent Mode (multi-step), Formula Completion AI suggest khi gõ `=`.

## Trạng thái hiện tại
- ✗ Chưa có.

## 39.1 Copilot Pane (Home → Copilot button)

### UI
- Pane bên phải màn hình (như Field List PivotTable).
- Chat conversation panel.
- Input textbox + Send button + Voice button (Windows).
- Suggested prompts chips trên cùng:
  - "Show insights about my data"
  - "Highlight rows where Sales > 1000"
  - "Add a column with profit margin"
  - "Create a PivotTable showing sales by region"
  - "Summarize this data"
  - "Forecast next quarter"

### Capabilities (single-action)
- Apply Conditional Formatting via natural language.
- Add calculated column.
- Sort/Filter.
- Create chart.
- Create PivotTable.
- Insight generation (trend analysis, anomaly detection).
- Cell-level Q&A: "What's the average?"

### UX flow
1. User: "Highlight rows where Sales > 1000 in red"
2. Copilot:
   - Identifies range
   - Proposes action (preview)
   - User: Apply / Modify / Discard
3. Apply → actually executes via internal API + adds to undo stack.

## 39.2 COPILOT Function (Excel 365 2025+)

Excel signature thực: `=COPILOT(prompt_part1, [context1], [prompt_part2], [context2], ...)` — prompt và context **đan xen nhau**, có thể spill mảng động, **không có tham số model**. Hiện ở kênh Frontier/Insider, giới hạn ~100 lần gọi / 10 phút.

### Examples
- `=COPILOT("Translate to English: " & A1)` — translate cell A1.
- `=COPILOT("Sentiment of: " & A1)` — classify sentiment.
- `=COPILOT("Summarize this paragraph", A1:A10)` — summarize multi-cell.
- `=COPILOT("Extract phone number: " & A1)` — entity extraction.

### Properties
- Async: cell hiện `#BUSY!` while waiting; auto-update khi response. (`#BUSY!` là lỗi async thật của Excel; KHÔNG có `#LOADING`.)
- Cache: same prompt → cached response (giảm cost).
- Cost meter: Status Bar hiển thị token usage.
- Rate limit handling.

### Model Selector — ⚠ CHƯA xác minh / đặc thù Ezcel
> Excel/Copilot **không** công bố UI cho user chọn model LLM — Microsoft quản lý
> phía sau. Phần dưới là **suy đoán + thiết kế riêng Ezcel**, tên/version model chỉ
> là placeholder, đừng hard-code như fact Excel. Việc cho chọn provider
> (Anthropic / OpenAI) là tính năng đặc thù Ezcel; danh sách thật lấy từ runtime.

### Ezcel design (riêng — không phải Excel API)
Ezcel có thể chọn cho thêm tham số model cuối formula cho convenience:
- `=COPILOT(prompt, [context], [model_id])` — `model_id` = string optional.
- Nhưng KHUYẾN NGHỊ giữ tương thích Excel: model qua Settings, không qua formula.

## 39.3 Agent Mode (2025-2026)

**Update T1/2026:** Agent Mode **generally available** trên Excel Windows + Excel for Web (Dec 2025) + Excel Mac (rollout T1/2026). Excel for Web có full Power Query experience tích hợp với Agent.

**Dec 2025:** Copilot Chat khả dụng trong **modern workbooks lưu local** (không cần OneDrive).

**Feb 2026:** Agent Mode + Copilot Chat query **locally stored Excel files** trên Windows + Mac; mở rộng cho user **EU** (Current Channel + Monthly Enterprise Channel).

**March 2026:** **Edit with Copilot** GA (M365 Copilot license) — Copilot trực tiếp chỉnh sửa cells/range mà không cần Agent plan; hỗ trợ full standard Copilot languages (50+); Claude Opus 4.6 trong model selector (M365 Premium / Enterprise).

**April 2026:**
- **Edit with Copilot** thêm: Chat/Edit switcher, step-by-step reasoning hiển thị, "Understand changes" panel review diff.
- **Python in Edit with Copilot**: Copilot dùng Python in-place cho advanced analysis (transform, visualization, multi-step) — không cần leave workbook.
- Model selector: GPT-5.5, Claude Opus 4.7 mới.
- Comments pane redesign (mobile-first): thread dễ tìm/đọc/act ([Spec 26](26-comments-notes.md)).

**May 2026:**
- **Copilot entry points consolidated**: chỉ còn 2 entry chính trên Windows + Mac (giảm clutter). Sau feedback tiêu cực, Microsoft **cho phép restore Copilot vào ribbon** thay vì chỉ trên canvas.
- **Copilot panel dock option**: reposition panel khi đang work.
- **Smart suggestions** + **refreshed keyboard shortcuts** (keyboard-first design).
- **=COPILOT web search**: Excel Insiders Windows/Mac + Frontier customers web — `=COPILOT()` có thể search web và ground results từ live data.
- **Show Changes card** Excel for Web: Copilot edits attribute trong Show Changes (transparency).
- **Microsoft 365 Copilot new design** (May 28 2026): unified design language across Word/Excel/PowerPoint Copilot panes.

**June 2026:**
- **Agentic capabilities GA**: Copilot agentic plan-execute-refine multi-step work GA trên Word + Excel + PowerPoint (theo April 22 2026 announcement, rollout WW June).
- **Copilot Notebooks → Excel agent**: tạo Excel spreadsheet trực tiếp từ Copilot Notebook (rolled out Frontier May → WW June).
- **=COPILOT web search + live data grounding** GA — enrich table với current info, lookup company details, pull benchmarks từ formula.
- **Show Changes** improvement: collaborator change with Copilot → Show Changes card có **Copilot attribution indicator** (visual flag + Copilot icon).

### 39.3.1 Agent Mode GA detail (April 22 2026)

- **Single-prompt workbook**: `"Create a quarterly revenue analysis workbook"` → agent build multi-tab spreadsheet (data + analysis + charts) trong 1 lượt; auto-generate complex formulas (VLOOKUP, pivots, structured models); chạy **validation loops** verify calc logic trước khi finalize.
- **Plan-execute-refine**: agent lập step plan trước, **show step list to user**, user approve → execute từng bước có dependency awareness; áp formatting đồng nhất whole workbook.
- **Multi-model reasoning** (key new): **model switcher** trong Copilot pane — chọn giữa **OpenAI** (default GPT-5.5) hoặc **Anthropic Claude** (Opus 4.6 / 4.7); user-controlled per session. Đây là Microsoft chính thức expose model choice trong Excel UI lần đầu (tách biệt với phần "Ezcel design" suy đoán cũ ở §39.2).
- **Engagement** (Microsoft công bố): trong 30 ngày preview Excel agent +67% engagement, +50% retention, +65% user satisfaction.

### 39.3.2 Edit with Copilot — Chat/Edit Switcher (April 2026)

- **Switcher UX** trong Copilot pane header: toggle 2 mode:
  - **Chat only**: trả lời câu hỏi, KHÔNG sửa file.
  - **Allow editing**: chủ động apply changes vào workbook.
- **Plan with Copilot**: trước khi modify, pane hiển thị **step-by-step plan** (numbered list) — user review + adjust trước, rồi execute. Quan trọng cho changes phức tạp.
- **Show Changes one-turn indicator**: sau khi Copilot edit, các cell bị thay đổi được highlight visual indicator **trong 1 turn** (đến next prompt) — user có thể revert hoặc tinh chỉnh ngay. Khác với Show Changes full pane (year-history, [Spec 44](44-collaboration-version-history.md)).
- **Understand changes panel**: side panel review diff trước/sau edit, cho user accept/reject từng change.

### 39.3.3 Python in Edit with Copilot (April 2026, Win/Mac/Web)

- Copilot có thể apply **Python-powered techniques** in-place: transform data, generate visualization, multi-step task — KHÔNG cần leave workbook.
- Output: chart / PivotTable / new column / new sheet được tạo qua Python execution (cloud sandbox).
- **Think Deeper mode** (Win/Mac/Web rolling out): toggle "Think Deeper" trong pane → Copilot dùng reasoning model, sinh **structured plan + Python code + step-by-step explanation** trong dedicated Python worksheet. Tốt cho complex data question.
- Liên quan tới [Spec 21 §21.7 Python in Edit with Copilot](21-vba-macro.md).

### 39.3.4 Federated Copilot Connectors (May-June 2026)

- **Wave 1 partners** (announced Feb 2026, GA late May): Canva, HubSpot, Intercom, Linear, **LSEG**, **Moody's**, Notion + others.
- **In Excel specifically**: **LSEG + Moody's** live data có thể được Copilot pull trực tiếp vào workbook qua federated MCP (Model Context Protocol). Web + Windows + Mac, commercial M365 Copilot license.
- HubSpot / Notion / Canva — có trong Copilot Chat / Researcher nhưng **chưa GA trong Excel** tính đến May 2026.
- Use case: `"Pull latest LSEG closing prices for AAPL, MSFT, NVDA into column B"` → connector fetch live → spill array.
- Trust labels: mỗi cell có connector source badge để audit.

Multi-step autonomous task — Copilot lập kế hoạch + thực thi nhiều action liên tiếp.

### Trigger
- Copilot pane → switch tab "Agent".
- Hoặc dropdown trên pane: ⚡ Agent.

### Example task
User: "Create a sales analysis dashboard from this data"

Agent plan (hiển thị step list):
1. Read source data range.
2. Create PivotTable summarizing by Product × Region.
3. Insert Line Chart for trend over time.
4. Add Conditional Formatting to highlight top performers.
5. Create summary text box with insights.

User: Approve plan → execute step-by-step (animation cell highlight) → done with summary.

### Web search trong Agent (2025)
- Agent có thể `web_search("Q3 2025 industry benchmarks")` → kéo vào sheet.
- Trust source labels.

## 39.4 Formula Completion với Copilot

Khi user gõ `=`, AI suggest hoàn chỉnh formula dựa workbook context.

### UX
- Gõ `=` → ghost text mờ hiện trong cell:
  - Cell context: header column = "Total Sales", neighbor = "Q1", "Q2", "Q3"
  - AI suggest: `=SUM(B2:D2)`
  - Tab accept; tiếp tục gõ override.
- Hoặc gõ formula partial: `=SUM(` → AI hoàn nốt args dựa context.

### Settings
- File → Options → Copilot → Formula completion: On / Off / Suggest only on Tab.

## 39.4.1 Natural Language Formulas (May 2026 GA, M365)

Khác với Formula Completion (ghost suggestion theo context), Natural Language Formulas cho user **describe what they want in plain English** trực tiếp inline.

### UX flow
1. Gõ `=` trong cell hoặc Formula Bar.
2. Một **chip inline** "Ask Copilot for a formula ✨" xuất hiện cạnh con trỏ.
3. Click chip → popover nhỏ với textbox prompt:
   ```
   ┌─ Ask Copilot ──────────────────────────────────┐
   │ Describe the formula you need:                  │
   │ [Calculate profit margin for each product line]│
   │                                                  │
   │            [Cancel]    [Generate ✨]            │
   └──────────────────────────────────────────────────┘
   ```
4. Generate → Copilot phân tích context (column headers, data types, neighbors) + sinh formula.
5. Preview formula trong tooltip; user Accept → fill cell; Reject → quay lại empty `=`.

### Modify existing formula
- Click cell có formula → Copilot chip "Modify formula" → describe change ("convert to absolute references", "add IFERROR wrapper", "explain this") → Copilot rewrite.

### Inline qua `=COPILOT()` function
- Hoặc dùng trực tiếp: `=COPILOT("Calculate profit margin", A:A, B:B)` — formula always re-evaluates.

### Settings
- Copilot pane settings → Natural Language Formulas: On / Off / Show chip on `=` only.

## 39.5 Recommended PivotTables — Copilot-powered (2026)

Microsoft đang **replace dialog "Recommended PivotTables" cũ** ([Spec 18](18-pivot-table.md)) bằng Copilot suggestions cho Business + Enterprise users:

- Insert → PivotTable → khi data lớn / phức tạp → Copilot pane mở với suggested layouts + insights inline.
- Mỗi suggestion có:
  - Mini chart preview.
  - 1-line "why this is useful" explanation.
  - One-click apply.
- Trends + outliers + correlation auto-flagged ở mỗi suggestion.

### Behavior cho non-Copilot users
- Vẫn fallback dialog Recommended PivotTables ([Spec 40 §40.3](40-quick-analysis.md)).
- Excel 2024 perpetual: **luôn** dùng dialog cũ (không có Copilot).

### Ezcel
- Phase Copilot integration: thay dialog bằng Copilot pane khi `copilot.enabled = True`.
- Phase non-Copilot: giữ dialog Recommended PivotTables như Spec 40.

## 39.5 Settings Pane

- API provider: Anthropic / OpenAI / Local LLM (Ollama).
- API key (encrypted in QSettings).
- Model preferences for each function.
- Usage budget alarm.
- Privacy: "Allow Copilot to read other sheets" toggle.

## 39.6 Economics & licensing (Excel-side)

### Microsoft 365 Copilot Credits model (Nov 2026 unified)
- **Trước Nov 2026**: AI Builder credits (seeded với enterprise plans) + Copilot Credits cùng tồn tại.
- **Sau Nov 2026**: AI Builder seeded credits **removed**; tất cả AI billing qua **Copilot Credits** unified pricing model (Microsoft consolidate).
- Excel `=COPILOT()` formula + Copilot pane + Agent Mode đều consume Copilot Credits.
- Free quota M365 Copilot license: số call có hạn theo plan (Personal: nhỏ; Business Standard: ~600/month soft; Enterprise: tier-based).
- Beyond quota → pay-as-you-go theo Copilot Credit (≈ 0.5 credit per 4,200 tokens ~ 16k chars cho text generation tool).

### `=COPILOT()` function limits
- Rate limit Frontier/Insiders preview: **~100 calls / 10 minutes per user** (xem §39.2).
- Token: per call ~ 8k input + 4k output max (typical M365 deployment).
- GA pricing chi tiết Microsoft chưa public commit — sẽ enforce per Copilot Credits sau Nov 2026.

### Ezcel approach
- Bring-your-own-key model: user paste Anthropic/OpenAI API key → Ezcel call direct → user pay provider.
- Tránh subscription overhead. Hiển thị token usage real-time ở Status Bar ([Spec 11](11-status-bar.md)).
- Optional: local LLM via Ollama (no cost, slower).

## Implementation note

### Architecture
- LLM client class wraps Anthropic SDK (đã có file system, dùng `anthropic` library — không có nhưng prep cho add).
- Tool calling: define internal tools (`apply_format`, `create_pivot`, `create_chart`, `add_formula`, `filter_rows`, ...).
- Agent mode: agentic loop với multi-tool calling pattern + plan display.
- Streaming: stream tokens vào pane cho UX nhanh.

### Anthropic SDK pattern
- Prompt caching (cache workbook context — system prompt).
- Tool use: tool schemas cho từng action type.
- Sonnet 4.6 mặc định (fast + smart).
- Token usage tracking.

### Workbook context cho Copilot
- Active sheet headers + first 20 rows (preview).
- Named ranges list.
- Charts/Tables list.
- Cell type hints (number / date / text per column).
- Compress để fit context limit.

### Tool implementation
```python
@tool("apply_conditional_formatting")
def apply_cf(range_addr: str, rule_type: str, condition: dict, format: dict):
    # Add CF rule via internal API
    sheet.add_cf_rule(range_addr, rule_type, condition, format)
    return {"success": True, "applied_to": range_addr}
```

### Async + UI
- LLM call trong thread (không block UI).
- Cell value `#BUSY!` while waiting (lỗi async thật của Excel; không có `#LOADING`).
- Stream response → update pane progressively.

## Acceptance criteria
1. Home → Copilot → pane mở; suggested prompts hiện.
2. Prompt "Highlight Sales > 1000 in red" → propose action → click Apply → CF rule applied, undo-able.
3. `=COPILOT("Translate to English: " & A1)` với A1="Xin chào" → cell hiển thị "Hello" (sau loading).
4. Agent task "Create dashboard" → list 5 step → user approve → 5 actions execute với animation.
5. Gõ `=` vào cell trong table → ghost text suggest `=SUM(...)` → Tab accept.
6. Settings → switch model → Anthropic Claude Sonnet → COPILOT function dùng Claude.
7. Status Bar hiện token usage hôm nay.

## Phụ thuộc
- Anthropic SDK (cần add) hoặc OpenAI SDK.
- Tất cả command modules (CF, PivotTable, Chart, Formula, ...) cần expose **command API** để Copilot gọi.
- [21 VBA / Macro](21-vba-macro.md) Python API → một phần overlap.

## Risk
**Cao.**
- LLM costs $.
- Network reliability.
- Tool use correctness (LLM có thể gọi tool sai args → confirm preview trước apply là bắt buộc).
- Privacy: workbook data sent to cloud — clear consent UI.
- Implement Phase rất muộn (sau MVP).
