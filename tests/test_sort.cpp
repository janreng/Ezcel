// Test headless cho module Sort (sắp xếp vùng dữ liệu). Chỉ in ASCII.
// Sort dùng thuần QString/QVector nên KHÔNG cần QApplication.
#include "model/Sort.h"
#include <QString>
#include <QVector>
#include <cstdio>

static int g_pass = 0, g_fail = 0;
static void ok(bool cond, const char *name) {
    if (cond) ++g_pass; else { ++g_fail; std::printf("FAIL %s\n", name); }
}

using Row   = QVector<QString>;
using Block = QVector<Row>;
using sort::Order;

// Ghép cột 0 của block thành "a,b,c" để so khớp nhanh.
static QString col0(const Block &b) {
    QString s;
    for (int i = 0; i < b.size(); ++i) { if (i) s += ','; s += b.at(i).value(0); }
    return s;
}

int main() {
    // 1) Số sắp theo GIÁ TRỊ, không phải theo chữ ("10" sau "9").
    {
        Block b{ Row{"10"}, Row{"9"}, Row{"100"}, Row{"2"} };
        Block s = sort::sortRows(b, { {0, Order::Ascending} });
        ok(col0(s) == "2,9,10,100", "so tang dan theo gia tri");
    }
    // 2) Giảm dần.
    {
        Block b{ Row{"2"}, Row{"9"}, Row{"10"} };
        Block s = sort::sortRows(b, { {0, Order::Descending} });
        ok(col0(s) == "10,9,2", "so giam dan");
    }
    // 3) Số đứng trước chữ.
    {
        Block b{ Row{"banana"}, Row{"5"}, Row{"apple"}, Row{"1"} };
        Block s = sort::sortRows(b, { {0, Order::Ascending} });
        ok(col0(s) == "1,5,apple,banana", "so dung truoc chu");
    }
    // 4) Ô rỗng luôn xuống cuối — chiều TĂNG.
    {
        Block b{ Row{"b"}, Row{""}, Row{"a"} };
        Block s = sort::sortRows(b, { {0, Order::Ascending} });
        ok(col0(s) == "a,b,", "o rong cuoi (tang)");
    }
    // 5) Ô rỗng VẪN xuống cuối — chiều GIẢM.
    {
        Block b{ Row{"b"}, Row{""}, Row{"a"} };
        Block s = sort::sortRows(b, { {0, Order::Descending} });
        ok(col0(s) == "b,a,", "o rong cuoi (giam)");
    }
    // 6) Chữ không phân biệt hoa-thường.
    {
        Block b{ Row{"Banana"}, Row{"apple"}, Row{"Cherry"} };
        Block s = sort::sortRows(b, { {0, Order::Ascending} });
        ok(col0(s) == "apple,Banana,Cherry", "chu khong phan biet hoa thuong");
    }
    // 7) Đa khóa: cột 0 tăng rồi cột 1 tăng -> A1,A3,B1,B2.
    {
        Block b{ Row{"A","3"}, Row{"B","1"}, Row{"A","1"}, Row{"B","2"} };
        Block s = sort::sortRows(b, { {0, Order::Ascending}, {1, Order::Ascending} });
        QString joined;
        for (int i = 0; i < s.size(); ++i) {
            if (i) joined += ',';
            joined += s.at(i).value(0) + s.at(i).value(1);
        }
        ok(joined == "A1,A3,B1,B2", "da khoa cot0 roi cot1");
    }
    // 8) Ổn định: khóa bằng nhau giữ nguyên thứ tự gốc (cột phụ làm dấu).
    {
        Block b{ Row{"x","1"}, Row{"x","2"}, Row{"x","3"} };
        Block s = sort::sortRows(b, { {0, Order::Ascending} });
        QString tag;
        for (const Row &r : s) tag += r.value(1);
        ok(tag == "123", "on dinh giu thu tu goc");
    }
    // 9) Không có khóa -> giữ nguyên block.
    {
        Block b{ Row{"3"}, Row{"1"}, Row{"2"} };
        Block s = sort::sortRows(b, {});
        ok(col0(s) == "3,1,2", "khong co khoa -> giu nguyen");
    }
    // 10) Số âm và thập phân so đúng theo giá trị.
    {
        Block b{ Row{"-3"}, Row{"2.5"}, Row{"-10"}, Row{"0"} };
        Block s = sort::sortRows(b, { {0, Order::Ascending} });
        ok(col0(s) == "-10,-3,0,2.5", "so am va thap phan");
    }

    std::printf("test_sort: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
