# Ezcel

Bảng tính gọn nhẹ viết bằng **C++/Qt6** — bản viết lại của Ezcel (gốc Python/PySide6) cho nhẹ và mượt hơn.

[![Release](https://img.shields.io/github/v/release/janreng/Ezcel)](https://github.com/janreng/Ezcel/releases/latest)

## Tải về

Tải bộ cài mới nhất ở trang [Releases](https://github.com/janreng/Ezcel/releases/latest): `Ezcel-Setup-x.y.z.exe` (cài per-user, không cần quyền admin). Trong app có sẵn **Trợ giúp → Kiểm tra cập nhật** để tự cập nhật.

## Tính năng

- **Tệp**: mở/lưu CSV và XLSX (giữ công thức, số, **ô gộp** và **định dạng**); nhiều trang tính trong một file.
- **Công thức**: ~146 hàm (toán, thống kê, ngày giờ, dò tìm, văn bản, logic, đổi hệ cơ số…), tính lại tức thì, có chống vòng tham chiếu.
- **Định dạng**: phông chữ, cỡ chữ, đậm/nghiêng/gạch, màu chữ–nền, căn lề, định dạng số, **định dạng có điều kiện**.
- **Bảng tính**: nhiều trang (tab), gộp ô, chèn/xóa/ẩn/hiện hàng-cột, tự vừa khít, thu phóng, tự xuống dòng, cố định đường lưới.
- **Dữ liệu**: sắp xếp, lọc, AutoSum, thống kê vùng chọn, chèn ngày/giờ, dán đặc biệt (chuyển vị).
- **Tìm & Thay thế**, **Ô địa chỉ (Name Box)**, hoàn tác/làm lại, sao chép–dán (tương thích clipboard ngoài).
- **Giao diện**: thanh công cụ biểu tượng, song ngữ **Tiếng Việt / English**.

## Build từ mã nguồn

Yêu cầu: **Qt 6.8.x (MinGW)**, CMake, Ninja (cài qua aqtinstall, đặt ở `C:\Qt`).

```bat
build.bat            REM cấu hình + biên dịch -> build\Ezcel.exe
```

QXlsx được kéo tự động qua CMake FetchContent (cần mạng lần đầu).

### Đóng gói

```bat
package.bat          REM windeployqt -> dist\Ezcel + Inno Setup -> installer\Ezcel-Setup-x.y.z.exe
```

## Kiến trúc

Module hoá theo lib lá độc lập, dễ phát triển song song:

```
ezcel_formula (engine công thức)  <- không phụ thuộc GUI
ezcel_model   (SpreadsheetModel + sort/filter/cond-format/stats…)
ezcel_io      (CSV + XLSX qua QXlsx)
ezcel_view    (delegate viền ô, span ô gộp, ẩn/hiện…)
ezcel_update  (tự cập nhật từ GitHub Releases)
Ezcel (app)   -> menu/toolbar/formula-bar/tab trang tính (glue mỏng)
```

Mỗi module có bộ test headless riêng (`tests/test_*.cpp`).

## Giấy phép

Mã nguồn của dự án. Icon dùng [Lucide](https://lucide.dev/) (ISC), thư viện [QXlsx](https://github.com/QtExcel/QXlsx) (MIT).
