# Hàm công thức còn thiếu (note để quay lại sau)

Tính tới v1.15.0 đã có ~288 hàm. Theo chỉ đạo user (2026-06-03): tạm dừng thêm hàm, chuyển sang làm spec khác. Danh sách hàm Excel **chưa làm**, ưu tiên thấp, quay lại sau:

## Phân phối thống kê (cần hàm ngược / tích phân)
- `NORMDIST`, `NORMINV` (chuẩn tổng quát — mean/sd)
- `BETADIST`, `BETAINV`, `GAMMADIST`, `GAMMAINV`
- `CHIDIST`, `CHIINV`, `CHITEST`
- `TDIST`, `TINV`, `TTEST`, `FDIST`, `FINV`, `FTEST`
- `LOGNORMDIST`, `LOGINV`, `CONFIDENCE.T`
- `BINOM.INV` / `CRITBINOM`, `PERCENTILE.EXC`, `QUARTILE.EXC`, `RANK.AVG`

## Mảng động (cần hạ tầng spill — KHÓ)
- `LINEST`, `LOGEST`, `TREND`/`GROWTH` dạng mảng nhiều điểm
- `FREQUENCY`, `TRANSPOSE`, `MMULT`, `MINVERSE`, `MDETERM`, `MUNIT`
- `TEXTSPLIT` (2D), `FILTER`, `SORT`, `UNIQUE`, `SEQUENCE`, `SORTBY`

## Số phức còn lại
- `IMTAN`-có, `IMASIN`/`IMACOS`/`IMATAN`, `IMASINH`/`IMACOSH`/`IMATANH`, `IMSECH`/`IMCSCH`/`IMCOTH`

## Tra cứu / tham chiếu (cần engine nâng cao)
- `OFFSET`, `INDIRECT`, `ADDRESS`, `AREAS`, `GETPIVOTDATA`
- `FORMULATEXT`, `ISFORMULA`, `SHEET`, `SHEETS`, `CELL`, `INFO`, `ERROR.TYPE`

## Tài chính nâng cao (cần tham số ngày)
- `XNPV`, `XIRR`, `YIELD`, `PRICE`, `DURATION`, `ACCRINT`, `DB`, `VDB`, `AMORDEGRC`

## Cơ sở dữ liệu (D-functions)
- `DSUM`, `DCOUNT`, `DAVERAGE`, `DMAX`, `DMIN`, `DGET`, `DPRODUCT`...

## Hàm ngày/giờ còn thiếu
- `DATEVALUE`, `TIMEVALUE`, `WORKDAY.INTL`, `NETWORKDAYS.INTL`, `ISOWEEKNUM`-có, `DAYS360`

> Tên có dấu chấm (`.`) cần nâng lexer để hỗ trợ (hiện đang né).
