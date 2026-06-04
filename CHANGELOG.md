# Lịch sử thay đổi

## v1.83.0 — Sửa ô mượt hơn trên bảng lớn (Spec 12)

- Tối ưu vẽ lại: khi bảng **không dùng mảng động**, mỗi lần sửa ô chỉ vẽ lại vùng thay đổi thay vì cả trang — bảng nhiều dữ liệu thao tác nhẹ và mượt hơn.
- Khi có công thức mảng động (spill) vẫn vẽ lại đầy đủ để các ô tràn luôn đúng.

## v1.82.0 — Ghép mảng VSTACK / HSTACK (Spec 12)

- Thêm **`VSTACK(vùng1, vùng2, ...)`** — xếp chồng nhiều vùng theo **chiều dọc** thành một bảng và tự tràn ra ô.
- Thêm **`HSTACK(vùng1, vùng2, ...)`** — ghép nhiều vùng theo **chiều ngang**.
- Các vùng lệch kích thước được chèn `#N/A` vào ô thiếu; tiện gộp nhiều danh sách/bảng rời thành một.

## v1.81.0 — Mảng động không đè lên ô đã gộp (Spec 12)

- **Sửa lỗi:** công thức mảng động (ví dụ `=SEQUENCE(...)`) khi tràn trúng vùng có **ô đã gộp** thì trước đây làm hỏng ô gộp; nay báo `#SPILL!` đúng như mong đợi.
- Bỏ gộp ô ở chỗ vướng là mảng tự tràn ra lại bình thường.

## v1.80.0 — Dự báo xu hướng tương lai (Spec 27)

- Thêm **Dữ liệu → "Dự báo xu hướng..."**: chọn cột thời gian + cột giá trị + số kỳ muốn dự báo, kết quả gồm dữ liệu thực tế và phần **dự báo** theo đường xu hướng tuyến tính được tạo ở trang mới.
- Tùy chọn thêm cột **trung bình trượt** để làm mượt dữ liệu (đặt cửa sổ = 0 nếu không cần).
- Bảng kết quả đặt thực tế và dự báo ở hai cột riêng, tiện vẽ biểu đồ so sánh.

## v1.79.0 — Gộp dữ liệu từ nhiều trang tính (Spec 27)

- "Gộp dữ liệu nhiều vùng" giờ lấy được vùng **từ trang tính khác** bằng cú pháp `Tên!Vùng` — ví dụ `Tháng 2!A1:C4 ; Tháng 3!A1:C4`.
- Tên trang có dấu cách đặt trong nháy đơn: `'Báo cáo Q1'!A1:C10`.
- Rất tiện để cộng dồn các trang cùng cấu trúc (mỗi tháng/chi nhánh một trang) thành một bảng tổng.

## v1.78.0 — Gộp dữ liệu nhiều vùng theo nhãn (Spec 27)

- Thêm **Dữ liệu → "Gộp dữ liệu nhiều vùng..."**: nhập nhiều vùng (mỗi vùng có nhãn cột ở hàng đầu, nhãn hàng ở cột đầu), chọn hàm tổng hợp, kết quả gộp theo nhãn được tạo ở trang mới.
- Nhãn hàng và nhãn cột được lấy hợp từ tất cả các vùng; ô giao nhau là kết quả tổng hợp (Tổng/Đếm/Trung bình/Lớn nhất/Nhỏ nhất) của mọi giá trị cùng nhãn.
- Hữu ích khi cộng dồn các bảng cùng cấu trúc (ví dụ doanh thu nhiều tháng/khu vực) về một bảng tổng.

## v1.77.0 — Hàm FILTER: lọc dữ liệu bằng công thức (Spec 12)

- Thêm hàm **`FILTER(vùng, điều_kiện, [nếu_rỗng])`** — giữ lại các dòng (hoặc cột) thỏa điều kiện rồi tự tràn kết quả ra ô.
- `=FILTER(A1:B10, C1:C10)` giữ những dòng có ô tương ứng ở cột điều kiện khác 0/TRUE.
- Nếu không dòng nào khớp: trả giá trị `nếu_rỗng` (nếu có truyền), ngược lại báo `#CALC!`.

## v1.76.0 — Hàm UNIQUE: lọc giá trị trùng bằng công thức (Spec 12)

- Thêm hàm **`UNIQUE(vùng, [theo_cột], [chỉ_một_lần])`** — lấy ra các dòng (hoặc cột) khác nhau từ một vùng rồi tự tràn kết quả ra ô.
- `=UNIQUE(A1:A20)` trả danh sách giá trị không trùng; đặt `chỉ_một_lần` = TRUE để chỉ lấy những giá trị xuất hiện đúng một lần.
- So khớp không phân biệt hoa/thường; vùng nhiều cột so cả dòng để xác định trùng.

## v1.75.0 — Hàm SORT: sắp xếp cả vùng bằng công thức (Spec 12)

- Thêm hàm **`SORT(vùng, [cột_khóa], [thứ_tự], [theo_cột])`** — sắp xếp một vùng dữ liệu rồi tự tràn kết quả ra ô bên cạnh (mảng động).
- `=SORT(A1:A10)` sắp xếp tăng dần; `=SORT(A1:B10, 2, -1)` sắp theo cột thứ 2 giảm dần; đặt `theo_cột` = TRUE để sắp các cột thay vì hàng.
- Sắp xếp số theo giá trị số, chữ theo bảng chữ cái; vùng nhiều cột được kéo theo nguyên hàng.

## v1.74.0 — Mảng động: công thức tự tràn ra nhiều ô (Spec 12)

- **Công thức trả về một dãy/khối số giờ tự "tràn" sang các ô bên cạnh** thay vì báo lỗi — gọi là mảng động (spill), giống bảng tính hiện đại.
- Thêm hàm **`SEQUENCE(số_hàng, [số_cột], [bắt_đầu], [bước_nhảy])`** tạo dãy số đếm dần — ví dụ `=SEQUENCE(5)` đổ ra 1,2,3,4,5 theo chiều dọc.
- Vùng tràn có **viền nét đứt xanh**; các ô trong vùng chỉ xem (sửa ở ô gốc). Nếu chỗ tràn bị vướng ô có sẵn dữ liệu thì báo `#SPILL!`; dọn chỗ vướng là tự tràn lại.

## v1.73.0 — Chèn / xóa ô có dịch chuyển (Spec 09)

- **Chèn ô** (menu Cấu trúc → "Chèn ô..."): chọn vùng rồi đẩy ô sang phải hoặc xuống dưới để chừa chỗ trống — không phải chèn nguyên hàng/cột nữa.
- **Xóa ô** (menu Cấu trúc → "Xóa ô..."): xóa vùng rồi dồn ô từ phải sang trái hoặc từ dưới lên trên.
- Vẫn có sẵn lựa chọn chèn/xóa nguyên hàng hoặc nguyên cột trong cùng hộp thoại.
- Dời cả nội dung, định dạng và ghi chú theo ô; hoàn tác (Ctrl+Z) được. Nếu vùng ảnh hưởng chạm ô đã gộp thì báo và không làm để khỏi vỡ ô gộp.

## v1.72.0 — Bảng tổng hợp chéo 2 chiều (Spec 18)

- **Bảng tổng hợp giờ làm được dạng chéo 2 chiều**: chọn một cột làm hàng và một cột làm tiêu đề cột, mỗi ô là kết quả tổng hợp giao nhau (như PivotTable).
- Tự thêm **cột "Tổng" và hàng "Tổng"** ở mép để xem nhanh tổng theo từng hàng, từng cột và toàn bộ.
- Vẫn giữ kiểu bảng 1 chiều như cũ: chỉ cần để ô "Cột làm tiêu đề cột" = 0.

## v1.71.0 — Sửa lỗi click ô khi đang gõ công thức báo #VALUE! (Spec 12)

- **Sửa lỗi:** đang gõ công thức dở (ví dụ `=SUM(`) mà bấm sang ô khác thì ô đang sửa bị "chốt" công thức chưa hoàn chỉnh và báo `#VALUE!`.
- Nay khi bấm sang ô khác lúc đang gõ công thức, ô vẫn mở và địa chỉ ô vừa bấm được chèn vào — đúng như "point mode" mong đợi.

## v1.70.0 — Click ô để chèn địa chỉ vào công thức đang gõ (Spec 12)

- **Đang gõ công thức trong ô (bắt đầu bằng `=`) mà bấm vào một ô khác, địa chỉ ô đó tự được chèn vào công thức** tại vị trí con trỏ — như "point mode" của bảng tính chuyên nghiệp.
- Ô đang sửa vẫn mở để tiếp tục gõ; tiện viết `=SUM(`, bấm chọn vùng/ô, rồi gõ tiếp.

## v1.69.0 — Gợi ý công thức kèm mô tả & ví dụ (Spec 12)

- **Tooltip công thức nay đầy đủ hơn**: ngoài cú pháp, còn hiển thị **mô tả hàm**, **ví dụ minh họa** và **giải thích từng tham số** — như ở bảng tính chuyên nghiệp.
- Phủ các hàm thường dùng (SUM, IF, VLOOKUP, INDEX, MATCH, SUMIF/SUMIFS, TEXT, DATE, DATEDIF...).

## v1.68.0 — Thêm chèn hàng bên dưới & cột bên phải (Spec 09)

- **Thêm "Chèn hàng bên dưới"** và **"Chèn cột bên phải"** trong menu Cấu trúc và menu chuột phải.
- Đổi tên cho rõ nghĩa: chèn cột nay ghi **"bên trái" / "bên phải"** (thay cho "phía trước / phía sau"), chèn hàng ghi **"bên trên" / "bên dưới"**.

## v1.67.0 — Gợi ý tham số của hàm khi nhập công thức (Spec 12)

- **Khi gõ tên hàm và mở ngoặc** (ví dụ `=VLOOKUP(`), ứng dụng hiện **gợi ý các tham số cần điền** ngay bên dưới, như ở bảng tính chuyên nghiệp.
- Phủ các hàm thường dùng (SUM, IF, VLOOKUP, INDEX, MATCH, SUMIFS, TEXT, DATE...) với mô tả tham số bằng tiếng Việt.

## v1.66.0 — Gợi ý hàm khi gõ công thức (Spec 12)

- **Gõ `=` rồi nhập chữ trong ô (hoặc thanh công thức) sẽ xổ popup gợi ý các hàm phù hợp** (SUM, IF, VLOOKUP...).
- Chọn một hàm là tự chèn tên hàm kèm dấu mở ngoặc, giúp viết công thức nhanh và đỡ gõ sai tên.

## v1.65.0 — Nhấn Backspace/Delete để xóa ô ngay (Spec 03)

- **Chọn ô rồi nhấn Backspace (hoặc Delete) là xóa nội dung ngay**, không cần bấm đúp vào ô nữa.
- Xóa được cả một vùng nhiều ô đang chọn cùng lúc.

## v1.64.0 — Bảng tổng hợp nhanh (Spec 18)

- **Thêm "Bảng tổng hợp nhanh..."** trong menu Dữ liệu: gom nhóm theo một cột và tổng hợp một cột giá trị (Tổng / Đếm / Trung bình / Lớn nhất / Nhỏ nhất).
- Kết quả được xuất ra một **trang tính mới** với mỗi nhóm một dòng, sắp xếp theo tên nhóm — như một bảng tổng hợp (pivot) thu gọn.

## v1.63.0 — Sửa trong ô nhấn Enter tự xuống dòng (Spec 03)

- **Sửa lỗi**: khi gõ nội dung NGAY TRONG Ô rồi nhấn Enter, con trỏ nay tự chuyển xuống ô bên dưới (trước đây vẫn đứng yên).
- Hoàn thiện cùng với bản trước (Enter ở thanh công thức) để thao tác nhập liệu đúng như mong đợi.

## v1.62.0 — Cố định dòng/cột (Spec 14)

- **Cố định dòng/cột** trong menu Xem: giữ các hàng trên và cột trái luôn hiển thị khi cuộn bảng.
- **Cố định tại ô hiện hành** (giữ mọi hàng/cột phía trên-trái ô đang chọn), **Cố định hàng đầu**, **Cố định cột đầu**, và **Bỏ cố định**.
- Vùng cố định và vùng cuộn được đồng bộ với nhau khi cuộn ngang/dọc.

## v1.61.0 — Tham chiếu chéo trang tính (Spec 10)

- **Công thức nay đọc được ô của trang tính khác**: dùng cú pháp `Tên!Ô`, ví dụ `=Sheet2!A1`, `=SUM(Báocáo!A1:A10)`.
- Hỗ trợ tên trang có dấu cách (bọc trong nháy đơn, ví dụ `='Bảng giá'!B2`), kết hợp tự do với ô của trang hiện tại.
- Sửa một trang sẽ tự tính lại các công thức ở trang khác đang tham chiếu tới nó.

## v1.60.0 — Hiện biểu tượng ứng dụng (Spec 50)

- **Sửa lỗi thiếu biểu tượng**: tệp chương trình nay nhúng sẵn biểu tượng, hiển thị đúng trên thanh tác vụ, cửa sổ và trong trình quản lý tệp.

## v1.59.0 — Kéo nút điền để tự tăng dần (Spec 05)

- **Nút kéo điền (fill handle)**: ô vuông nhỏ ở góc dưới-phải vùng chọn. Kéo xuống hoặc sang phải để tự điền tiếp chuỗi (số tăng dần, ngày, thứ, mẫu lặp...).
- Tận dụng cùng cơ chế tự điền như khi dùng lệnh Điền; chọn hướng theo chiều kéo dài hơn.

## v1.58.0 — Gõ xong nhấn Enter tự nhảy xuống ô dưới (Spec 03)

- **Sửa lỗi**: nhập nội dung rồi nhấn Enter nay tự chuyển xuống ô ngay bên dưới như mong đợi.
- **Gõ phím là vào chế độ sửa ngay** trên ô đang chọn (không cần bấm đúp hay F2 trước); Tab chuyển sang phải, Enter xuống dưới.

## v1.57.0 — Hiện độ lệch chuẩn ở thanh trạng thái (Spec 11)

- **Thanh trạng thái** nay hiển thị thêm **Độ lệch chuẩn** (mẫu) của vùng chọn khi có từ 2 ô số trở lên.
- Bổ sung cho Đếm, Tổng, Trung bình, Trung vị, Nhỏ nhất và Lớn nhất — xem nhanh mức phân tán dữ liệu mà không cần công thức.

## v1.56.0 — Hiện trung vị ở thanh trạng thái (Spec 11)

- **Thanh trạng thái** nay hiển thị thêm **Trung vị** của vùng chọn, bên cạnh Đếm, Tổng, Trung bình, Nhỏ nhất và Lớn nhất.
- Tự sắp xếp các ô số để tính; rất tiện để xem giá trị giữa mà không cần công thức.

## v1.55.0 — Áp phép tính với hằng số lên vùng chọn (Spec 13)

- **Thêm "Áp phép tính với hằng số..."** trong menu Dữ liệu: cộng, trừ, nhân hoặc chia tất cả các ô số trong vùng chọn với một số nhất định.
- Chỉ tác động ô số; bỏ qua ô công thức và ô văn bản. Chia cho 0 giữ nguyên giá trị.

## v1.54.0 — Đảo ngược thứ tự cột (Spec 15)

- **Thêm "Đảo ngược thứ tự cột"** trong menu Dữ liệu: lật ngược thứ tự các cột trong vùng chọn (cột đầu thành cột cuối).
- Bổ trợ cho "Đảo ngược thứ tự hàng" để sắp xếp lại bố cục bảng nhanh.

## v1.53.0 — Chọn ô trùng giá trị (Spec 15)

- **Thêm "Chọn ô trùng giá trị"** trong menu Dữ liệu: tự đánh dấu tất cả các ô có giá trị xuất hiện nhiều hơn một lần trong cột đang chọn.
- So sánh không phân biệt hoa/thường; bỏ qua ô trống. Giúp rà soát trùng lặp nhanh trước khi xử lý.

## v1.52.0 — Gộp nhiều cột thành một (Spec 27)

- **Thêm "Gộp cột thành một..."** trong menu Dữ liệu: nối nội dung nhiều cột của mỗi hàng thành một chuỗi, ngăn cách bằng dấu tùy chọn.
- Là thao tác ngược của "Tách cột"; tự bỏ qua các ô trống khi ghép. Kết quả ghi vào cột đầu tiên.

## v1.51.0 — Điền ô trống bằng giá trị phía trên (Spec 27)

- **Thêm "Điền ô trống bằng giá trị trên"** trong menu Dữ liệu: tự điền các ô trống bằng giá trị không rỗng gần nhất phía trên (theo từng cột).
- Rất hữu ích khi dữ liệu xuất ra chỉ ghi nhãn nhóm ở hàng đầu mỗi nhóm, để lại các ô bên dưới trống.

## v1.50.0 — Đảo ngược thứ tự hàng (Spec 15)

- **Thêm "Đảo ngược thứ tự hàng"** trong menu Dữ liệu: lật ngược thứ tự các hàng trong vùng chọn (hàng đầu thành hàng cuối).
- Hữu ích khi cần xem danh sách theo chiều ngược mà không phải sắp xếp lại.

## v1.49.0 — Hàm ngày làm việc tùy chỉnh cuối tuần (Spec 12)

- **NETWORKDAYSINTL(ngày đầu, ngày cuối, [mã cuối tuần], [ngày nghỉ])** — đếm số ngày làm việc, cho phép chọn ngày nào trong tuần là cuối tuần.
- **WORKDAYINTL(ngày đầu, số ngày, [mã cuối tuần], [ngày nghỉ])** — tìm ngày làm việc thứ N với quy ước cuối tuần tùy chỉnh.
- Mã cuối tuần theo số (1–7, 11–17) hoặc chuỗi 7 ký tự (T2…CN, 1 = nghỉ). Hữu ích cho lịch làm việc không theo thứ Bảy–Chủ Nhật.

## v1.48.0 — DATEDIF hỗ trợ thêm đơn vị MD, YM, YD (Spec 12)

- **DATEDIF** nay nhận thêm các đơn vị: **YM** (số tháng lẻ sau khi trừ năm), **MD** (số ngày lẻ sau khi trừ tháng), **YD** (số ngày lẻ sau khi trừ năm).
- Tiện cho việc hiển thị khoảng cách giữa hai ngày theo dạng "X năm Y tháng Z ngày".

## v1.47.0 — Bỏ ký tự không in được cho vùng chọn (Spec 27)

- **Thêm "Bỏ ký tự không in được"** trong menu Sửa: loại bỏ các ký tự điều khiển ẩn thường lẫn vào khi sao chép từ web hoặc tệp khác.
- Đổi khoảng trắng đặc biệt (non-breaking space) thành dấu cách thường; bỏ ký tự độ rộng bằng 0 và BOM. Giữ nguyên ô công thức.

## v1.46.0 — Cắt gọn khoảng trắng cho vùng chọn (Spec 27)

- **Thêm "Cắt gọn khoảng trắng"** trong menu Sửa: bỏ khoảng trắng thừa ở đầu/cuối và rút nhiều dấu cách liên tiếp thành một.
- Cũng xử lý tab và xuống dòng thành dấu cách. Chỉ tác động ô văn bản; giữ nguyên ô công thức.

## v1.45.0 — Đổi chữ hoa/thường cho vùng chọn (Spec 05)

- **Thêm menu "Đổi chữ hoa/thường"** trong menu Sửa: đổi nhanh nội dung vùng chọn sang **CHỮ HOA**, **chữ thường**, hoặc **Viết Hoa Đầu Từ**.
- Chỉ tác động lên ô văn bản; ô công thức được giữ nguyên.

## v1.44.0 — Hàm cơ sở dữ liệu nâng cao: DGET, DVAR, DSTDEV... (Spec 27)

- **DGET** — lấy đúng một giá trị duy nhất khớp tiêu chí (báo lỗi nếu không có hoặc có nhiều hơn một).
- **DVAR, DVARP, DSTDEV, DSTDEVP** — tính phương sai và độ lệch chuẩn (mẫu hoặc tổng thể) trên bản ghi khớp tiêu chí.
- Hoàn thiện bộ hàm cơ sở dữ liệu.

## v1.43.0 — Hàm cơ sở dữ liệu: DSUM, DCOUNT, DAVERAGE... (Spec 27)

- Thêm nhóm hàm **CSDL**: **DSUM, DCOUNT, DCOUNTA, DAVERAGE, DMAX, DMIN, DPRODUCT**.
- Tính toán trên một bảng có tiêu đề, lọc theo **vùng tiêu chí** (hỗ trợ điều kiện như `>15`, `A`, kết hợp nhiều cột/nhiều dòng).
- Chọn cột cần tính theo **tên tiêu đề** hoặc số thứ tự cột.

## v1.42.0 — Điền chuỗi số có bước nhảy (Spec 05)

- **Thêm "Điền chuỗi..."** trong menu Sửa: chọn một vùng rồi điền dãy số tự động.
- Chọn **cấp số cộng** (cộng dồn bước nhảy) hoặc **cấp số nhân** (nhân bước nhảy); lấy ô đầu làm số bắt đầu.
- Điền theo cột hoặc theo hàng tùy hướng vùng chọn dài hơn.

## v1.41.0 — Hàm ngày/giờ: DATEVALUE, TIMEVALUE, DAYS360 (Spec 12)

- **DATEVALUE(văn bản)** — đổi chuỗi ngày (ví dụ "2026-06-03") thành giá trị ngày để tính toán.
- **TIMEVALUE(văn bản)** — đổi chuỗi giờ (ví dụ "12:00:00") thành phần thập phân của ngày.
- **DAYS360(ngày đầu, ngày cuối, [kiểu])** — đếm số ngày theo quy ước năm 360 ngày (kiểu Mỹ hoặc châu Âu), thường dùng trong tài chính.

## v1.40.0 — Hàm biểu thức chính quy: REGEXTEST, REGEXEXTRACT, REGEXREPLACE (Spec 22)

- **REGEXTEST(văn bản, mẫu, [phân biệt hoa/thường])** — trả về ĐÚNG/SAI tùy chuỗi có khớp mẫu hay không.
- **REGEXEXTRACT(văn bản, mẫu, [chế độ], [phân biệt])** — trích phần khớp đầu tiên (hoặc nhóm bắt).
- **REGEXREPLACE(văn bản, mẫu, thay thế, [lần khớp], [phân biệt])** — thay phần khớp; có thể thay tất cả hoặc chỉ lần thứ n.
- Hữu ích để kiểm tra định dạng (email, mã số), trích số, hay che bớt dữ liệu.

## v1.39.0 — Chọn nhiều vùng & cả cột/hàng qua ô địa chỉ (Spec 02)

- Gõ vào **ô địa chỉ** nay nhận nhiều vùng cách nhau bằng dấu phẩy: `A1:B3,D5,F1:F10` để chọn cùng lúc.
- Hỗ trợ chọn **cả cột** (`A:A`, `A:C`) và **cả hàng** (`1:1`, `2:4`).
- Tự chuẩn hóa vùng đảo ngược và giới hạn theo kích thước bảng.

## v1.38.0 — Tìm tất cả (Spec 32)

- **Thêm nút "Tìm tất cả"** trong hộp thoại Tìm & Thay thế: chọn cùng lúc mọi ô có chứa từ khóa.
- Báo số lượng ô tìm thấy ở thanh trạng thái; tôn trọng tùy chọn phân biệt hoa/thường.

## v1.37.0 — Quản lý vùng đặt tên (Spec 31)

- **Thêm "Quản lý tên vùng..."** trong menu Sửa: liệt kê tất cả vùng đã đặt tên kèm địa chỉ (ví dụ `DoanhThu → A1:C5`).
- **Đi tới**: chọn nhanh vùng tương ứng với một tên.
- **Xóa**: gỡ bỏ vùng đặt tên không còn dùng.

## v1.36.0 — Kiểm tra tên vùng hợp lệ (Spec 04)

- Khi đặt **tên cho vùng** qua ô địa chỉ, ứng dụng kiểm tra tên theo quy tắc chuẩn và báo lý do nếu không hợp lệ.
- Chặn các tên sai: bắt đầu bằng số, có khoảng trắng, trùng dạng địa chỉ ô (A1, R1C1), ký tự cấm, hay tên dành riêng "C"/"R".

## v1.35.0 — Phím F4 khóa tham chiếu tuyệt đối (Spec 04)

- **Nhấn F4 trên thanh công thức** để đảo nhanh kiểu khóa của tham chiếu ô tại con trỏ: A1 → $A$1 → A$1 → $A1 → A1.
- Tiện khi viết công thức cần cố định cột/hàng trước khi kéo sao chép.

## v1.34.0 — Truy vết ô tham chiếu & phụ thuộc (Spec 32)

- **Truy vết ô tham chiếu (precedents)**: chọn nhanh tất cả ô mà công thức của ô đang chọn đang dùng tới (lan qua nhiều cấp).
- **Truy vết ô phụ thuộc (dependents)**: chọn tất cả ô có công thức phụ thuộc vào ô đang chọn.
- Giúp kiểm tra mạch tính toán: đổi một ô sẽ ảnh hưởng tới đâu, hoặc một kết quả đến từ những ô nào.

## v1.33.0 — Sao chép chỉ ô đang hiển thị (Spec 32)

- **Sao chép chỉ ô hiện (Alt+;)**: khi dữ liệu đã lọc/ẩn hàng, chỉ chép các hàng đang hiển thị, bỏ qua hàng ẩn.
- Tránh tình trạng dán nhầm cả những hàng đã bị lọc đi như khi sao chép thường.

## v1.32.0 — Tìm ô khác biệt theo hàng/cột (Spec 32)

- **Khác biệt theo hàng**: trong vùng chọn, đánh dấu các ô có giá trị khác với ô mốc trên cùng hàng (cột của ô đang chọn).
- **Khác biệt theo cột**: tương tự nhưng so theo cùng cột với ô mốc trên hàng đang chọn.
- Hữu ích để soi nhanh các ô lệch dữ liệu trong một bảng.

## v1.31.0 — Chọn vùng dữ liệu & đi tới ô cuối (Spec 32)

- **Chọn vùng dữ liệu (Ctrl+Shift+*)**: tự khoanh khối dữ liệu liên tục bao quanh ô đang chọn.
- **Đi tới ô cuối**: nhảy nhanh tới ô cuối cùng còn dữ liệu trong bảng.
- Bổ sung cho nhóm thao tác "Đi tới đặc biệt" đã có (ô trống, công thức, số, văn bản, hằng).

## v1.30.0 — Sắp xếp nhiều cấp (Spec 15)

- **Thêm "Sắp xếp nhiều cấp..."** trong menu Dữ liệu: sắp theo tối đa 3 cột liên tiếp ("sắp theo... rồi theo...").
- Mỗi cấp chọn cột và chiều **Tăng dần / Giảm dần** riêng.
- Hữu ích khi cần gom theo nhóm rồi xếp giá trị trong từng nhóm.

## v1.29.0 — Lọc tùy chỉnh 2 điều kiện (Spec 15)

- **Thêm "Lọc tùy chỉnh (2 điều kiện)..."** trong menu Dữ liệu: kết hợp hai điều kiện bằng **VÀ / HOẶC**.
- Mỗi điều kiện chọn phép: bằng, khác, lớn hơn, nhỏ hơn (và ≥ ≤), chứa, không chứa, bắt đầu bằng, kết thúc bằng.
- So sánh thông minh: tự nhận biết số để so theo số, còn lại so theo chữ. Để trống điều kiện thứ hai nếu chỉ cần một.

## v1.28.0 — Lọc theo điều kiện số (Spec 15)

- **Thêm "Lọc theo số..."** trong menu Dữ liệu: ẩn/hiện hàng theo so sánh số trên cột đang chọn.
- Hỗ trợ: Bằng, Khác, Lớn hơn, Lớn hơn hoặc bằng, Nhỏ hơn, Nhỏ hơn hoặc bằng, Trong khoảng [a, b], Ngoài khoảng.
- Thêm **Trên/Dưới trung bình** — tự so với giá trị trung bình của cột.

## v1.27.0 — Xóa định dạng & xóa tất cả (Spec 09)

- **Menu "Xóa..."** trong Sửa: tách 3 lựa chọn rõ ràng — Tất cả (nội dung + định dạng), Chỉ định dạng, Chỉ nội dung.
- **Chỉ định dạng**: gỡ hết tô màu, in đậm, căn lề... mà vẫn giữ nguyên dữ liệu.
- **Tất cả**: xóa sạch cả nội dung lẫn định dạng trong một bước, hoàn tác lại được bằng một lần Ctrl+Z.

## v1.26.0 — Gom nhóm & thu gọn cột (Spec 09)

- **Gom nhóm cột**: chọn một dải cột rồi gom thành nhóm để gấp gọn theo chiều ngang.
- **Thu gọn/Mở rộng nhóm cột**: ẩn nhanh các cột chi tiết trong nhóm rồi bung lại khi cần.
- **Bỏ nhóm cột**: gỡ nhóm tại ô đang chọn. Hoàn thiện gom nhóm cho cả hàng lẫn cột.

## v1.25.0 — Gom nhóm & thu gọn hàng (Spec 09)

- **Gom nhóm hàng** (Alt+Shift+→): chọn một dải hàng rồi gom thành nhóm để gấp gọn.
- **Thu gọn/Mở rộng nhóm**: ẩn nhanh các hàng chi tiết trong nhóm rồi bung lại khi cần.
- **Bỏ nhóm hàng** (Alt+Shift+←): gỡ nhóm tại ô đang chọn. Các nhóm có thể lồng nhau.

## v1.24.0 — Tổng phụ theo nhóm (Spec 27)

- **Thêm "Tổng phụ theo nhóm..."** trong menu Dữ liệu: tự chèn dòng tổng sau mỗi nhóm dữ liệu liên tiếp và một dòng tổng cộng ở cuối.
- Chọn **hàm tổng hợp**: Tổng, Đếm, Trung bình, Lớn nhất, Nhỏ nhất; chọn cột cần tính.
- Nhóm theo cột của ô đang chọn (dữ liệu nên được sắp xếp theo cột đó trước).

## v1.23.0 — Dán đặc biệt: phép tính & bỏ qua ô trống (Spec 13)

- **Hộp thoại "Dán đặc biệt" mở rộng**: chọn phép tính khi dán — Cộng, Trừ, Nhân, Chia — để gộp giá trị vùng dán vào giá trị đang có ở vùng đích.
- **Bỏ qua ô trống**: ô trống trong vùng nguồn sẽ không ghi đè lên ô đích đang có dữ liệu.
- Vẫn giữ tùy chọn **Chuyển vị** (hàng ↔ cột); các tùy chọn có thể kết hợp với nhau.

## v1.22.0 — Menu chuột phải trên ô và đầu hàng/cột (Spec 06)

- **Bấm chuột phải vào ô** mở menu nhanh: Cắt, Sao chép, Dán, Dán đặc biệt, Chèn/Xóa hàng-cột, Xóa nội dung, Lọc theo giá trị, Sắp xếp tăng/giảm, Chọn từ danh sách.
- **Bấm chuột phải vào ô đầu hàng** mở menu: Chèn hàng phía trên, Xóa hàng, Xóa nội dung, Chiều cao hàng... (nhập số), Tự khít chiều cao, Ẩn/Hiện lại hàng.
- **Bấm chuột phải vào ô đầu cột** mở menu: Chèn cột phía trước, Xóa cột, Độ rộng cột... (nhập số), Tự khít độ rộng, Ẩn/Hiện lại cột.
- Mục **"Hiện lại"** chỉ bật khi trong vùng chọn thật sự có hàng/cột đang ẩn.

## v1.21.0 — Công cụ dữ liệu: xóa hàng trùng & tách cột (Spec 27)

- **Thêm "Xóa hàng trùng"** trong menu Dữ liệu: tự tìm và xóa các hàng có nội dung trùng nhau (giữ lần xuất hiện đầu, giữ hàng tiêu đề).
- **Thêm "Tách cột theo dấu phân cách..."**: tách nội dung một cột thành nhiều cột dựa trên dấu phân cách (phẩy, chấm phẩy, dấu cách…).

## v1.20.0 — Lọc theo giá trị (Spec 15)

- **Thêm "Lọc theo giá trị..."** trong menu Dữ liệu: mở hộp thoại liệt kê các giá trị duy nhất của cột với ô tích chọn — bỏ tích để ẩn các hàng có giá trị đó, giống bộ lọc cột quen thuộc. Bổ sung cho bộ lọc theo từ khóa đã có.

## v1.19.0 — Chọn từ danh sách & gợi ý tự động (Spec 05)

- **Thêm "Chọn từ danh sách" (Alt + Mũi tên xuống)**: hiện menu liệt kê các giá trị văn bản đã có trong cột, chọn để điền nhanh vào ô — đỡ phải gõ lại.
- Bổ sung lõi **tự động hoàn thành** theo tiền tố (gợi ý khi chỉ có một giá trị trong cột khớp).

## v1.18.0 — Tự điền theo mẫu Flash Fill (Spec 05)

- **Thêm Flash Fill (Ctrl + E)**: gõ 1–2 ví dụ ở đầu một cột, ứng dụng tự nhận ra quy luật tách/ghép từ cột dữ liệu bên trái và điền nốt cả cột — ví dụ tách họ ("Nguyễn Văn A" → "Nguyễn"), tách tên miền email ("user@gmail.com" → "gmail.com") hay cắt tiền tố.

## v1.17.0 — Tự điền chuỗi lịch & quý (Spec 05)

- **Kéo để tự điền chuỗi thứ, tháng và quý**: từ `Mon` ra `Tue, Wed…`, `Jan` ra `Feb, Mar…`, `Q1` ra `Q2, Q3, Q4` (tự xoay vòng), giữ nguyên kiểu chữ hoa/thường — bổ sung cho tự điền dãy số và chuỗi có số sẵn có.

## v1.16.0 — Thống kê bảng tính (Spec 57)

- **Thêm hộp thoại "Thống kê bảng tính"** trong menu Trợ giúp: hiển thị số ô có dữ liệu, số công thức, số ô là số và số từ — cho cả trang hiện tại và toàn bộ bảng tính (tất cả các trang). Tiện kiểm tra nhanh độ phức tạp của tệp.

## v1.15.0 — Bộ hàm lớn: phân phối xác suất, đổi đơn vị & thống kê (Spec 12)

- **Thêm nhóm hàm phân phối xác suất**: `BINOMDIST` (nhị thức), `POISSON`, `EXPONDIST` (mũ), `WEIBULL`, `NEGBINOMDIST` (nhị thức âm) và `HYPGEOMDIST` (siêu bội), hỗ trợ cả dạng mật độ và tích lũy.
- **Mở rộng hàm `CONVERT`** với bốn nhóm đơn vị mới: **áp suất** (Pa, kPa, bar, atm, mmHg, psi), **năng lượng** (J, kJ, cal, kcal, Wh, kWh, eV), **công suất** (W, kW, mã lực HP/PS) và **lực** (N, kN, dyn, lbf).
- **Thêm hàm thống kê**: `CONFIDENCE` (khoảng tin cậy), `TRIMMEAN` (trung bình cắt), `MODE` (giá trị xuất hiện nhiều nhất) và `SUMX2MY2`, `SUMX2PY2`, `SUMXMY2`.

## v1.14.0 — Hàm gamma & phân phối chuẩn (Spec 12)

- **Thêm hàm `GAMMA` và `GAMMALN`** — tính hàm gamma và logarit của hàm gamma.
- **Thêm hàm `NORMSDIST` và `NORMSINV`** — tính hàm phân phối tích lũy chuẩn tắc và hàm nghịch đảo (phân vị), rất hữu ích trong thống kê và kiểm định.

## v1.13.0 — Hàm sai số ERF & ERFC (Spec 12)

- **Thêm hàm `ERF`** — tính hàm sai số (error function), hỗ trợ cả dạng tích phân giữa hai cận.
- **Thêm hàm `ERFC`** — tính hàm sai số bù (1 trừ ERF), thường dùng trong xác suất và kỹ thuật.

## v1.12.0 — Hàm mã hóa URL ENCODEURL (Spec 12)

- **Thêm hàm `ENCODEURL`** — mã hóa một chuỗi theo kiểu URL (dấu cách thành `%20`, ký tự đặc biệt thành mã phần trăm), tiện khi ghép địa chỉ web.

## v1.11.0 — Hàm VARPA & STDEVPA (Spec 12)

- **Thêm hàm `VARPA` và `STDEVPA`** — tính phương sai và độ lệch chuẩn của toàn bộ tổng thể, có tính cả ô chứa chữ và giá trị logic.

## v1.10.0 — Hàm VARA & STDEVA (Spec 12)

- **Thêm hàm `VARA` và `STDEVA`** — tính phương sai và độ lệch chuẩn mẫu, có tính cả ô chứa chữ (coi là 0) và giá trị logic, phù hợp khi vùng dữ liệu lẫn nhiều kiểu.

## v1.9.0 — Hàm dự báo TREND & GROWTH (Spec 12)

- **Thêm hàm `TREND`** — dự báo giá trị theo xu hướng tuyến tính từ dữ liệu đã biết.
- **Thêm hàm `GROWTH`** — dự báo giá trị theo xu hướng tăng trưởng hàm mũ.

## v1.8.0 — Hàm lượng giác nghịch đảo số phức (Spec 12)

- **Thêm hàm `IMCSC`, `IMSEC` và `IMCOT`** — tính cosec, sec và cotang của một số phức (nghịch đảo của sin, cos và tang).

## v1.7.0 — Sinh & cosh số phức (Spec 12)

- **Thêm hàm `IMSINH`** — tính sin hyperbolic của một số phức.
- **Thêm hàm `IMCOSH`** — tính cos hyperbolic của một số phức.

## v1.6.0 — Tang & logarit cơ số số phức (Spec 12)

- **Thêm hàm `IMTAN`** — tính tang của một số phức.
- **Thêm hàm `IMLOG10` và `IMLOG2`** — tính logarit cơ số 10 và cơ số 2 của một số phức.

## v1.5.0 — Sin & cos số phức (Spec 12)

- **Thêm hàm `IMSIN`** — tính sin của một số phức.
- **Thêm hàm `IMCOS`** — tính cos của một số phức.

## v1.4.0 — Căn & lũy thừa số phức (Spec 12)

- **Thêm hàm `IMSQRT`** — tính căn bậc hai của một số phức.
- **Thêm hàm `IMPOWER`** — tính lũy thừa của một số phức với số mũ cho trước.

## v1.3.0 — Hàm mũ & logarit số phức (Spec 12)

- **Thêm hàm `IMEXP`** — tính lũy thừa cơ số e của một số phức.
- **Thêm hàm `IMLN`** — tính logarit tự nhiên của một số phức.

## v1.2.0 — Nhân & chia số phức (Spec 12)

- **Thêm hàm `IMPRODUCT`** — nhân nhiều số phức với nhau.
- **Thêm hàm `IMDIV`** — chia hai số phức.

## v1.1.0 — Phép toán số phức (Spec 12)

- **Thêm hàm `IMSUM` và `IMSUB`** — cộng và trừ các số phức.
- **Thêm hàm `IMCONJUGATE`** — tính số phức liên hợp.
- **Thêm hàm `IMARGUMENT`** — tính góc (argument) của số phức theo radian.

## v1.0.0 — Phiên bản 1.0 🎉

**Cột mốc lớn:** đây là bản viết lại hoàn chỉnh bằng C++/Qt6 — khởi động nhanh hơn, thao tác mượt hơn và đầy đủ tính năng so với bản cũ. Ứng dụng đã có:

- **Bảng tính đa trang** với thanh tab, đổi tên, màu tab và phím tắt chuyển trang.
- **Định dạng phong phú**: phông chữ, màu sắc, viền ô, gộp ô, kiểu ô dựng sẵn, nhiều kiểu định dạng số.
- **Công cụ dữ liệu**: sắp xếp, lọc, tìm & thay thế, định dạng có điều kiện, kiểm tra dữ liệu, đi tới đặc biệt.
- **Hơn 250 hàm tính toán**: toán học, thống kê & hồi quy, ngày tháng, văn bản, tra cứu, tài chính (vay/đầu tư/khấu hao) và kỹ thuật (đổi hệ, số phức).
- **Tiện ích giao diện**: thu phóng, chỉ báo chế độ ô, thanh trạng thái thống kê, bảng tra cứu phím tắt, hai ngôn ngữ Việt/Anh, tự động cập nhật.

### Mới trong bản này (Spec 12)
- **Thêm nhóm hàm số phức** `COMPLEX`, `IMREAL`, `IMAGINARY`, `IMABS` — dựng và phân tích số phức dạng `a+bi`.

## v0.99.0 — Đổi chéo hệ nhị phân, bát phân & thập lục (Spec 12)

- **Thêm nhóm hàm đổi chéo hệ** `BIN2HEX`, `HEX2BIN`, `BIN2OCT`, `OCT2BIN`, `HEX2OCT`, `OCT2HEX` — chuyển trực tiếp giữa nhị phân, bát phân và thập lục mà không cần qua thập phân.

## v0.98.0 — Hàm đổi hệ bát phân DEC2OCT & OCT2DEC (Spec 12)

- **Thêm hàm `DEC2OCT`** — đổi số thập phân sang chuỗi bát phân (cơ số 8).
- **Thêm hàm `OCT2DEC`** — đổi chuỗi bát phân về số thập phân, bổ sung cho nhóm đổi hệ nhị phân và thập lục đã có.

## v0.97.0 — Hàm sai số chuẩn hồi quy STEYX (Spec 12)

- **Thêm hàm `STEYX`** — tính sai số chuẩn của giá trị y dự báo theo đường hồi quy, cho biết độ chính xác của dự báo.

## v0.96.0 — Hàm COVAR & FORECAST (Spec 12)

- **Thêm hàm `COVAR`** — tính hiệp phương sai tổng thể giữa hai vùng dữ liệu.
- **Thêm hàm `FORECAST`** — dự báo một giá trị theo đường hồi quy tuyến tính từ dữ liệu đã biết.

## v0.95.0 — Hàm tương quan CORREL, PEARSON & RSQ (Spec 12)

- **Thêm hàm `CORREL` và `PEARSON`** — tính hệ số tương quan giữa hai vùng dữ liệu (giá trị từ -1 đến 1).
- **Thêm hàm `RSQ`** — tính hệ số xác định R² của đường hồi quy, cho biết mức độ phù hợp của mô hình.

## v0.94.0 — Hàm hồi quy SLOPE & INTERCEPT (Spec 12)

- **Thêm hàm `SLOPE`** — tính hệ số góc của đường hồi quy tuyến tính qua hai vùng dữ liệu.
- **Thêm hàm `INTERCEPT`** — tính điểm giao của đường hồi quy với trục tung, dùng để dự báo theo xu hướng.

## v0.93.0 — Hàm tổng lãi & gốc CUMIPMT & CUMPRINC (Spec 12)

- **Thêm hàm `CUMIPMT`** — tính tổng tiền lãi đã trả giữa hai kỳ bất kỳ của khoản vay.
- **Thêm hàm `CUMPRINC`** — tính tổng tiền gốc đã trả giữa hai kỳ, tiện theo dõi dư nợ theo từng giai đoạn.

## v0.92.0 — Hàm tài chính MIRR (Spec 12)

- **Thêm hàm `MIRR`** — tính tỉ suất hoàn vốn nội bộ điều chỉnh, tách riêng lãi suất vay vốn và lãi suất tái đầu tư để đánh giá dự án thực tế hơn so với IRR.

## v0.91.0 — Hàm lãi suất EFFECT & NOMINAL (Spec 12)

- **Thêm hàm `EFFECT`** — tính lãi suất hiệu dụng năm từ lãi suất danh nghĩa và số kỳ ghép lãi mỗi năm.
- **Thêm hàm `NOMINAL`** — tính ngược lãi suất danh nghĩa từ lãi suất hiệu dụng.

## v0.90.0 — Hàm giá kiểu phân số DOLLARDE & DOLLARFR (Spec 12)

- **Thêm hàm `DOLLARDE`** — đổi giá ghi theo kiểu phân số (ví dụ 1.02 nghĩa là 1 và 2/16) thành số thập phân.
- **Thêm hàm `DOLLARFR`** — đổi ngược số thập phân về cách ghi kiểu phân số, thường dùng cho giá chứng khoán.

## v0.89.0 — Hàm khấu hao nhanh DDB (Spec 12)

- **Thêm hàm `DDB`** — tính khấu hao theo phương pháp số dư giảm dần kép cho từng kỳ, có hệ số tăng tốc tùy chọn và tự dừng khi chạm giá trị thanh lý.

## v0.88.0 — Hàm khấu hao SLN & SYD (Spec 12)

- **Thêm hàm `SLN`** — tính khấu hao đều mỗi kỳ của tài sản (phương pháp đường thẳng).
- **Thêm hàm `SYD`** — tính khấu hao theo tổng số năm sử dụng (khấu hao nhanh hơn ở những năm đầu).

## v0.87.0 — Hàm tài chính RATE (Spec 12)

- **Thêm hàm `RATE`** — tính lãi suất mỗi kỳ của một khoản vay hoặc khoản đầu tư, hoàn thiện bộ hàm giá trị thời gian của tiền (FV, PV, PMT, NPER, RATE).

## v0.86.0 — Hàm tài chính IPMT & PPMT (Spec 12)

- **Thêm hàm `IPMT`** — tính phần tiền lãi trong khoản trả của một kỳ nhất định.
- **Thêm hàm `PPMT`** — tính phần tiền gốc trong khoản trả của một kỳ nhất định (IPMT cộng PPMT bằng khoản trả mỗi kỳ).

## v0.85.0 — Hàm tài chính NPV & IRR (Spec 12)

- **Thêm hàm `NPV`** — tính giá trị hiện tại ròng của một dãy dòng tiền theo lãi suất chiết khấu.
- **Thêm hàm `IRR`** — tính tỉ suất hoàn vốn nội bộ của một dãy dòng tiền, giúp đánh giá hiệu quả đầu tư.

## v0.84.0 — Hàm tài chính PMT & NPER (Spec 12)

- **Thêm hàm `PMT`** — tính khoản trả đều mỗi kỳ cho một khoản vay (trả góp) với lãi suất và số kỳ cố định.
- **Thêm hàm `NPER`** — tính số kỳ cần thiết để trả hết khoản vay hoặc đạt mục tiêu tích lũy.

## v0.83.0 — Hàm tài chính FV & PV (Spec 12)

- **Thêm hàm `FV`** — tính giá trị tương lai của một khoản đầu tư hoặc khoản vay với lãi suất và kỳ hạn cố định.
- **Thêm hàm `PV`** — tính giá trị hiện tại của một dòng tiền đều trong tương lai.

## v0.82.0 — Hàm đổi chuỗi thành số NUMBERVALUE (Spec 12)

- **Thêm hàm `NUMBERVALUE`** — chuyển một chuỗi thành số với dấu thập phân và dấu phân nhóm tùy chọn (hỗ trợ cả kiểu châu Âu `1.234,56`), tự xử lý dấu phần trăm ở cuối.

## v0.81.0 — Mở rộng đổi đơn vị diện tích & thể tích (Spec 12)

- **`CONVERT` hỗ trợ thêm đơn vị diện tích** (m², km², cm², ft², in², héc-ta) và **thể tích** (lít, ml, m³, gallon, quart, pint, ft³, in³) — bổ sung cho khối lượng, độ dài, thời gian và nhiệt độ đã có.

## v0.80.0 — Hàm đổi đơn vị CONVERT (Spec 12)

- **Thêm hàm `CONVERT`** — đổi đơn vị đo lường: khối lượng (g, kg, mg, lbm, ozm), độ dài (m, km, cm, mm, mi, yd, ft, in), thời gian (giây, phút, giờ, ngày) và nhiệt độ (C, F, K).

## v0.79.0 — Hàm PERCENTRANK & SERIESSUM (Spec 12)

- **Thêm hàm `PERCENTRANK`** — xác định thứ hạng của một giá trị theo tỉ lệ phần trăm trong tập dữ liệu.
- **Thêm hàm `SERIESSUM`** — tính tổng của một chuỗi lũy thừa theo các hệ số cho trước.

## v0.78.0 — Hàm STANDARDIZE & MULTINOMIAL (Spec 12)

- **Thêm hàm `STANDARDIZE`** — chuẩn hóa một giá trị về điểm z dựa trên trung bình và độ lệch chuẩn.
- **Thêm hàm `MULTINOMIAL`** — tính hệ số đa thức (tỉ lệ giai thừa của tổng so với tích các giai thừa).

## v0.77.0 — Hàm tổ hợp có lặp COMBINA & PERMUTATIONA (Spec 12)

- **Thêm hàm `COMBINA`** — số tổ hợp có lặp lại khi chọn k phần tử từ n nhóm.
- **Thêm hàm `PERMUTATIONA`** — số chỉnh hợp có lặp lại (n mũ k).

## v0.76.0 — Hàm phân phối chuẩn PHI & GAUSS (Spec 12)

- **Thêm hàm `PHI`** — mật độ xác suất của phân phối chuẩn tắc tại một giá trị.
- **Thêm hàm `GAUSS`** — xác suất một biến chuẩn tắc rơi vào khoảng từ 0 đến giá trị cho trước.

## v0.75.0 — Hàm biến đổi Fisher FISHER & FISHERINV (Spec 12)

- **Thêm hàm `FISHER`** — biến đổi Fisher một giá trị (trong khoảng -1 đến 1), thường dùng để chuẩn hóa hệ số tương quan.
- **Thêm hàm `FISHERINV`** — biến đổi Fisher nghịch đảo, đưa giá trị về dạng ban đầu.

## v0.74.0 — Hàm hình dạng phân phối SKEW & KURT (Spec 12)

- **Thêm hàm `SKEW`** — tính độ lệch (skewness) của tập dữ liệu mẫu, cho biết phân phối nghiêng về bên nào.
- **Thêm hàm `KURT`** — tính độ nhọn (kurtosis) của tập dữ liệu mẫu, cho biết phân phối nhọn hay tù so với phân phối chuẩn.

## v0.73.0 — Chỉ báo chế độ ô trên thanh trạng thái (Spec 11)

- **Thêm chỉ báo chế độ ô** ở góc trái thanh trạng thái: hiện **Sẵn sàng** khi đang chọn ô, chuyển sang **Nhập** khi bắt đầu gõ vào thanh công thức, trở lại **Sẵn sàng** sau khi xác nhận hoặc đổi ô — giống thói quen quen thuộc của ứng dụng bảng tính.

## v0.72.0 — Hàm độ lệch DEVSQ & AVEDEV (Spec 12)

- **Thêm hàm `DEVSQ`** — tổng bình phương độ lệch của các giá trị so với trung bình.
- **Thêm hàm `AVEDEV`** — độ lệch tuyệt đối trung bình của các giá trị so với trung bình.

## v0.71.0 — Hàm phân vị PERCENTILE & QUARTILE (Spec 12)

- **Thêm hàm `PERCENTILE`** — tính phân vị thứ k (0–1) của một vùng số theo phương pháp nội suy tuyến tính.
- **Thêm hàm `QUARTILE`** — tính tứ phân vị (nhỏ nhất, 25%, trung vị, 75%, lớn nhất) của vùng số.

## v0.70.0 — Hàm tách chuỗi TEXTBEFORE & TEXTAFTER (Spec 12)

- **Thêm hàm `TEXTBEFORE`** — lấy phần văn bản trước dấu phân cách; chọn được lần xuất hiện thứ mấy (đếm từ đầu hoặc từ cuối) và có/không phân biệt hoa thường.
- **Thêm hàm `TEXTAFTER`** — lấy phần văn bản sau dấu phân cách, với cùng các tùy chọn, tiện tách họ tên, email, mã chuỗi…

## v0.69.0 — Hàm thống kê AVERAGEA, MAXA, MINA (Spec 12)

- **Thêm `AVERAGEA`, `MAXA`, `MINA`** — giống AVERAGE/MAX/MIN nhưng tính cả ô chứa chữ (coi là 0) và giá trị logic (ĐÚNG = 1, SAI = 0), hữu ích khi vùng dữ liệu lẫn cả số, chữ và logic.

## v0.68.0 — Hàm phần năm YEARFRAC (Spec 12)

- **Thêm hàm `YEARFRAC`** — tính phần năm giữa hai ngày theo 5 quy ước đếm ngày (US 30/360, thực/thực, thực/360, thực/365, Âu 30/360), tiện cho tính lãi và kỳ hạn tài chính.

## v0.67.0 — Hàm so sánh kỹ thuật DELTA & GESTEP (Spec 12)

- **Thêm hàm `DELTA`** — trả về 1 nếu hai số bằng nhau, ngược lại 0 (delta Kronecker).
- **Thêm hàm `GESTEP`** — trả về 1 nếu số lớn hơn hoặc bằng ngưỡng, ngược lại 0.

## v0.66.0 — Hàm chữ số La Mã ROMAN & ARABIC (Spec 12)

- **Thêm hàm `ROMAN`** — đổi số (0–3999) sang chữ số La Mã, ví dụ `1994` ra `MCMXCIV`.
- **Thêm hàm `ARABIC`** — đổi ngược chữ số La Mã về số nguyên, hiểu cả cú pháp trừ (IV, IX, XC…).

## v0.65.0 — Hàm thao tác bit (Spec 12)

- **Thêm nhóm hàm thao tác bit** `BITAND`, `BITOR`, `BITXOR` (và/hoặc/hoặc-loại-trừ theo bit) cùng `BITLSHIFT`, `BITRSHIFT` (dịch bit trái/phải) — tiện cho tính toán kỹ thuật và xử lý cờ nhị phân.

## v0.64.0 — Hàm lượng giác mở rộng (Spec 12)

- **Thêm hàm lượng giác nghịch đảo** `SEC`, `CSC`, `COT` và phiên bản hyperbolic `SECH`, `CSCH`, `COTH`.
- **Thêm hàm hyperbolic ngược** `ASINH`, `ACOSH`, `ATANH` — tính nghịch đảo của sinh, cosh, tanh.

## v0.63.0 — Hàm số tuần WEEKNUM & ISOWEEKNUM (Spec 12)

- **Thêm hàm `WEEKNUM`** — trả về số thứ tự tuần trong năm của một ngày; chọn được tuần bắt đầu từ Chủ Nhật (mặc định) hoặc Thứ Hai.
- **Thêm hàm `ISOWEEKNUM`** — trả về số tuần theo chuẩn ISO 8601 (tuần bắt đầu Thứ Hai, tuần 1 chứa Thứ Năm đầu tiên của năm).

## v0.62.0 — Hàm ngày làm việc WORKDAY & NETWORKDAYS (Spec 12)

- **Thêm hàm `WORKDAY`** — tính ngày sau (hoặc trước) một số ngày làm việc, tự bỏ qua Thứ Bảy, Chủ Nhật và danh sách ngày nghỉ tùy chọn.
- **Thêm hàm `NETWORKDAYS`** — đếm số ngày làm việc giữa hai mốc ngày (gồm cả hai đầu), trừ ngày nghỉ; đảo thứ tự cho kết quả âm.

## v0.61.0 — Hàm ngày tháng EDATE & EOMONTH (Spec 12)

- **Thêm hàm `EDATE`** — cộng/trừ số tháng vào một ngày, tự dồn về ngày cuối tháng khi cần (ví dụ 31/01 cộng 1 tháng ra 29/02).
- **Thêm hàm `EOMONTH`** — trả về ngày cuối cùng của tháng sau khi cộng/trừ số tháng, xử lý đúng cả năm nhuận.

## v0.60.0 — Điều hướng & quản lý trang tính (Spec 10)

- **Chuyển nhanh giữa các trang tính bằng Ctrl + Page Down / Page Up** (quay vòng khi tới trang cuối/đầu).
- **Bấm chuột phải vào tab trang tính** để Đổi tên, đặt **Màu tab**, hoặc Xóa trang.

## v0.59.0 — Thanh trượt thu phóng (Spec 11)

- **Thêm thanh trượt thu phóng** ở góc phải thanh trạng thái: nút **−** / **+** đổi 10% mỗi nấc, kéo trượt từ 10% đến 400%, bấm vào số **%** để chọn nhanh mức dựng sẵn (25, 50, 75, 100, 150, 200, 400). Cuộn **Ctrl + lăn chuột** vẫn thu phóng như cũ.

## v0.58.0 — Thanh trạng thái hiện thêm thống kê

- **Thanh trạng thái hiện thêm Nhỏ nhất, Lớn nhất và Đếm số** khi chọn nhiều ô: ngoài Đếm, Tổng và Trung bình đã có, nay xem nhanh được giá trị nhỏ nhất, lớn nhất và số lượng ô chứa số trong vùng chọn.

## v0.57.0 — Bảng tra cứu phím tắt

- **Thêm hộp thoại "Phím tắt"** trong menu Trợ giúp (hoặc nhấn **F1**): liệt kê đầy đủ phím tắt theo nhóm — Di chuyển, Chọn, Soạn thảo, Định dạng, Công thức, Tệp & Trang tính — kèm mô tả tiếng Việt cho từng tổ hợp.

## v0.56.0 — Thêm kiểu định dạng số

- **Thêm các kiểu định dạng số** trên thanh định dạng: **tiền tệ** ($1,234.00), **khoa học** (1.23E+04) và **phân số** (1/2, 1 1/2) — ngoài số nghìn, phần trăm, ngày đã có.

## v0.55.0 — Đi tới đặc biệt

- **Chọn nhanh các ô theo loại** (menu **Sửa → Đi tới đặc biệt**): chọn để tô sáng toàn bộ ô **trống / công thức / số / văn bản / có dữ liệu** trong trang — tiện rà soát và thao tác hàng loạt.

## v0.54.0 — Kiểu ô dựng sẵn

- **Áp kiểu ô nhanh** (ô **Kiểu ô** trên thanh định dạng): chọn vùng rồi chọn kiểu **Tốt / Xấu / Trung tính / Tiêu đề / Tựa đề** để tô màu + chữ theo bộ có sẵn; chọn **Bình thường** để xóa định dạng.

## v0.53.0 — Đặt tên vùng (Named Range)

- **Đặt tên cho vùng ô**: chọn vùng → gõ một cái tên (vd `DoanhThu`) vào ô địa chỉ rồi Enter để lưu tên. Sau này gõ lại tên đó vào ô địa chỉ là **nhảy tới và chọn đúng vùng** ngay.

## v0.52.0 — Ghi chú ô

- **Thêm ghi chú cho ô** (menu **Sửa → Ghi chú ô**, hoặc Shift+F2): ô có ghi chú hiện **dấu tam giác đỏ** ở góc và nội dung ghi chú khi rê chuột vào. Có nút xóa ghi chú.

## v0.51.0 — Kiểm tra dữ liệu nhập

- **Đặt quy tắc kiểm tra dữ liệu cho ô** (menu **Dữ liệu → Kiểm tra dữ liệu**): chọn vùng và yêu cầu giá trị là **số nguyên / số thập phân / độ dài văn bản** thỏa điều kiện (nằm giữa, lớn hơn, nhỏ hơn…). Nhập sai sẽ bị từ chối và báo ở thanh trạng thái. Có nút xóa quy tắc.

## v0.50.0 — Thêm hàm đổi hệ cơ số

- **Bổ sung hàm đổi hệ cơ số**: `DECIMAL`, `DEC2BIN`, `DEC2HEX`, `BIN2DEC`, `HEX2DEC` — chuyển qua lại giữa số thập phân, nhị phân và thập lục phân.

## v0.49.0 — Thêm hàm định dạng số

- **Bổ sung hàm** `FIXED` (định dạng số có dấu phẩy hàng nghìn), `DOLLAR` (định dạng tiền tệ) và `BASE` (đổi số sang hệ cơ số khác, ví dụ nhị phân/thập lục).

## v0.48.0 — Thêm hàm công thức

- **Bổ sung hàm** `TIME` (tạo giờ từ giờ/phút/giây), `T` (lấy phần văn bản) và `N` (đổi sang số) — nâng tổng số hàm tính toán hỗ trợ.

## v0.47.0 — Tự xuống dòng trong ô

- **Cho chữ tự xuống dòng trong ô** (menu **Xem → Tự xuống dòng trong ô**): bật để nội dung dài hiển thị nhiều dòng vừa bề rộng cột, không bị cắt.

## v0.46.0 — Bật/tắt đường lưới

- **Ẩn hoặc hiện đường kẻ ô** (menu **Xem → Hiện đường lưới**): tắt đi để xem dữ liệu sạch hơn, bật lại khi cần.

## v0.45.0 — Thanh công cụ chính bằng biểu tượng

- **Thanh công cụ chính nhiều biểu tượng hơn**: Mới / Mở / Lưu, Hoàn tác / Làm lại, Cắt / Sao chép / Dán, Sắp xếp tăng-giảm, Tìm, Gộp ô — tất cả nay là biểu tượng trực quan, thao tác nhanh hơn.

## v0.44.0 — Biểu tượng cho thanh định dạng

- **Nút định dạng có biểu tượng**: in đậm/nghiêng/gạch chân/gạch ngang, màu chữ, màu nền, căn lề trên thanh định dạng nay hiển thị bằng biểu tượng trực quan thay cho chữ — gọn và dễ nhận biết hơn.

## v0.43.0 — Xóa trang tính

- **Đóng/xóa trang tính**: mỗi tab có nút ✕ để xóa trang không cần nữa (luôn giữ lại ít nhất một trang).

## v0.42.0 — Lưu nhiều trang vào một file

- **Mở và lưu nhiều trang tính trong cùng một file `.xlsx`**: khi lưu, tất cả các trang đều được ghi vào file; khi mở file nhiều trang, các tab sẽ hiện đầy đủ theo đúng tên trang.

## v0.41.0 — Nhiều trang tính

- **Làm việc với nhiều trang tính trong cùng cửa sổ**: thanh tab ở dưới cùng, bấm **+** để thêm trang, nhấp đúp vào tab để đổi tên, chuyển qua lại giữa các trang. Mỗi trang có dữ liệu riêng.

## v0.40.0 — Chèn ngày / giờ

- **Chèn nhanh ngày và giờ** (menu **Dữ liệu**): Ctrl+`;` chèn ngày hôm nay, Ctrl+Shift+`;` chèn giờ hiện tại vào ô đang chọn.

## v0.39.0 — Tính tổng nhanh (AutoSum)

- **Tính tổng tự động** (menu **Dữ liệu → AutoSum**, hoặc Alt+`=`): tự nhận dãy số ngay phía trên (hoặc bên trái) ô đang chọn và chèn công thức `=SUM(...)` cho bạn.

## v0.38.0 — Thống kê vùng chọn

- **Xem nhanh tổng / trung bình / số ô**: khi bôi đen nhiều ô, góc phải thanh trạng thái hiện ngay **Đếm**, **Tổng** và **Trung bình** của vùng chọn — không cần gõ công thức.

## v0.37.0 — Thanh công cụ kiểu dải lệnh

- **Thanh công cụ gọn và chuyên nghiệp hơn**: nền xám nhạt, nút bo góc và sáng lên khi rê chuột, có vạch ngăn nhóm — nhìn giống dải lệnh (ribbon) quen thuộc.

## v0.36.0 — Dịch trọn thanh menu

- **Toàn bộ thanh menu đã song ngữ**: ngoài các menu trước đó, nay cả menu **Sửa, Cấu trúc, Dữ liệu** cũng đổi theo ngôn ngữ đã chọn (Tiếng Việt / English).

## v0.35.0 — Song ngữ Việt / Anh

- **Đổi ngôn ngữ giao diện** (menu **Cài đặt → Ngôn ngữ → Tiếng Việt / English**): thanh menu chuyển sang tiếng Anh hoặc tiếng Việt. Các phần còn lại của giao diện sẽ được dịch dần ở các bản sau.

## v0.34.0 — Dán đặc biệt

- **Dán đặc biệt** (menu **Sửa → Dán đặc biệt**, hoặc Ctrl+Alt+V): chọn kiểu dán, hỗ trợ **chuyển vị** — đổi hàng thành cột và ngược lại khi dán.

## v0.33.0 — Lọc dữ liệu

- **Lọc nhanh theo cột** (menu **Dữ liệu → Lọc theo cột hiện tại**): nhập một từ khóa, chỉ giữ lại các hàng có cột đó chứa từ khóa (giữ nguyên hàng tiêu đề). Bấm "Bỏ lọc" để hiện lại tất cả.

## v0.32.0 — Định dạng có điều kiện

- **Tô màu ô theo điều kiện** (menu **Dữ liệu → Định dạng có điều kiện**): chọn vùng, đặt điều kiện (lớn hơn / nhỏ hơn / bằng / khác / nằm giữa / chứa chữ) và màu nền — những ô thỏa điều kiện sẽ tự tô màu. Có nút xóa toàn bộ quy tắc.

## v0.31.0 — Thu phóng (Zoom)

- **Phóng to / thu nhỏ bảng tính**: giữ **Ctrl + lăn chuột**, hoặc menu **Xem → Phóng to / Thu nhỏ / Thu phóng 100%** (Ctrl+`=`, Ctrl+`-`, Ctrl+`0`). Phóng từ 50% đến 400%.

## v0.30.0 — Tự vừa khít cột/dòng

- **Tự điều chỉnh độ rộng cột và chiều cao dòng cho vừa nội dung** (menu **Cấu trúc**): chọn vùng rồi chọn "Vừa khít độ rộng cột" / "Vừa khít chiều cao dòng" — không còn cảnh chữ bị che hay cột quá rộng.

## v0.29.0 — Ẩn / hiện hàng và cột

- **Ẩn và hiện lại hàng/cột** (menu **Cấu trúc**): chọn vùng rồi Ẩn hàng / Ẩn cột để giấu tạm; chọn quanh chỗ bị ẩn rồi "Hiện lại (bỏ ẩn)" để cho hiện ra.

## v0.28.0 — Ô địa chỉ (Name Box)

- **Ô địa chỉ bên trái thanh công thức**: luôn hiện vị trí ô đang chọn (vd `A1`). Gõ một địa chỉ (`B10`) hoặc một vùng (`A1:C5`) rồi Enter để **nhảy/chọn nhanh** tới đó.

## v0.27.0 — Viền ô đang chọn nổi bật

- **Ô đang chọn có viền xanh đậm bao quanh** — con trỏ ô rõ ràng, dễ thấy mình đang ở ô nào.

## v0.26.0 — Phông chữ chuyên nghiệp

- **Phông chữ chuẩn**: giao diện dùng Segoe UI, nội dung ô dùng Aptos Narrow cỡ 11 (tự lùi về Calibri nếu máy chưa có) — chữ nhìn quen và gọn.

## v0.25.0 — Bảng màu mới

- **Giao diện lưới chuyên nghiệp hơn**: đường kẻ ô màu xám nhạt, vùng chọn tô xanh nhạt với chữ xanh, tiêu đề cột/dòng nền xám và sáng lên màu xanh khi đang chọn — nhìn dễ chịu hơn.

## v0.24.0 — Biểu tượng ứng dụng

- **Ezcel đã có icon riêng**: hiển thị trên thanh tiêu đề cửa sổ, thanh tác vụ và bộ cài đặt (giống biểu tượng quen thuộc của bản trước).

## v0.23.0 — Lưu định dạng vào file XLSX

- **Định dạng giờ được lưu vào file `.xlsx`**: phông chữ, cỡ chữ, in đậm/nghiêng/gạch, màu chữ, màu nền, căn lề, xuống dòng và định dạng số (số nghìn, phần trăm…) đều được giữ khi lưu và mở lại — không còn mất định dạng sau khi đóng file.

## v0.22.0 — Sắp xếp dữ liệu

- **Sắp xếp vùng dữ liệu** (menu **Dữ liệu → Sắp xếp tăng dần / giảm dần**): sắp các hàng của vùng đang chọn theo cột đang đứng — số đứng trước chữ, ô trống dồn xuống cuối, các cột còn lại đi theo hàng. Hoàn tác được.

## v0.21.0 — Tự động cập nhật

- **Kiểm tra cập nhật trong ứng dụng**: menu **Trợ giúp → Kiểm tra cập nhật** sẽ dò bản mới nhất trên GitHub, nếu có bản mới thì tải file cài và chạy luôn cho bạn.
- Thêm hộp thoại **Giới thiệu Ezcel** hiển thị phiên bản đang dùng.

## v0.20.0 — Bản C++ đầu tiên

Ezcel viết lại hoàn toàn bằng C++/Qt6 cho nhẹ và mượt hơn. Bản đầu này đã có đủ tính năng dùng hằng ngày:

- **Mở & lưu file**: CSV và XLSX (giữ công thức, số và ô gộp)
- **Công thức**: hơn 120 hàm (toán, thống kê, ngày giờ, dò tìm, văn bản, logic…) tính lại tức thì
- **Định dạng**: phông chữ, cỡ chữ, in đậm/nghiêng/gạch, màu chữ–nền, căn lề, định dạng số (số nghìn, phần trăm, ngày)
- **Ô gộp**: gộp/bỏ gộp, hiển thị span trực quan
- **Chỉnh sửa**: hoàn tác/làm lại, sao chép–cắt–dán (dán được từ ứng dụng bảng tính khác), điền chuỗi xuống/sang phải
- **Cấu trúc**: chèn/xóa hàng và cột (tự dời dữ liệu, định dạng, ô gộp theo)
- **Tìm & Thay thế** (Ctrl+F / Ctrl+H), có phân biệt hoa/thường
- **Thanh công thức** sửa nhanh nội dung ô, **Hiện công thức** (Ctrl+`)
