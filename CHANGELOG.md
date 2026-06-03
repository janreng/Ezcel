# Lịch sử thay đổi

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
