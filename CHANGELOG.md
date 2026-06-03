# Lịch sử thay đổi

## v0.22.0 — Sắp xếp dữ liệu

- **Sắp xếp vùng dữ liệu** (menu **Dữ liệu → Sắp xếp tăng dần / giảm dần**): sắp các hàng của vùng đang chọn theo cột đang đứng, kiểu Excel — số đứng trước chữ, ô trống dồn xuống cuối, các cột còn lại đi theo hàng. Hoàn tác được.

## v0.21.0 — Tự động cập nhật

- **Kiểm tra cập nhật trong ứng dụng**: menu **Trợ giúp → Kiểm tra cập nhật** sẽ dò bản mới nhất trên GitHub, nếu có bản mới thì tải file cài và chạy luôn cho bạn.
- Thêm hộp thoại **Giới thiệu Ezcel** hiển thị phiên bản đang dùng.

## v0.20.0 — Bản C++ đầu tiên

Ezcel viết lại hoàn toàn bằng C++/Qt6 cho nhẹ và mượt hơn. Bản đầu này đã có đủ tính năng dùng hằng ngày:

- **Mở & lưu file**: CSV và XLSX (giữ công thức, số và ô gộp)
- **Công thức**: hơn 120 hàm (toán, thống kê, ngày giờ, dò tìm, văn bản, logic…) tính lại tức thì
- **Định dạng**: phông chữ, cỡ chữ, in đậm/nghiêng/gạch, màu chữ–nền, căn lề, định dạng số (số nghìn, phần trăm, ngày)
- **Ô gộp**: gộp/bỏ gộp, hiển thị span trực quan
- **Chỉnh sửa**: hoàn tác/làm lại, sao chép–cắt–dán (dán được từ Excel), điền chuỗi xuống/sang phải
- **Cấu trúc**: chèn/xóa hàng và cột (tự dời dữ liệu, định dạng, ô gộp theo)
- **Tìm & Thay thế** (Ctrl+F / Ctrl+H), có phân biệt hoa/thường
- **Thanh công thức** sửa nhanh nội dung ô, **Hiện công thức** (Ctrl+`)
