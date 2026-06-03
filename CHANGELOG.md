# Lịch sử thay đổi

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
