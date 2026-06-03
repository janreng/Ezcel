#pragma once
#include <QString>
#include <QVariant>
#include <memory>
#include <vector>

// Kiểu giá trị nội bộ của engine công thức (port từ formula.py).
// Dùng struct gắn thẻ (tagged) cho rõ ràng + dễ build; perf vẫn tốt.
namespace formula {

struct Value;
struct Range;

enum class Type { Empty, Number, Bool, Text, Range };

// Vùng 2 chiều các giá trị ô (ma trận row×col) — như _Range bản Python.
struct Range {
    std::shared_ptr<std::vector<std::vector<Value>>> rows;
    int height() const;
    int width() const;
    std::vector<Value> flat() const;
};

struct Value {
    Type type = Type::Empty;
    double num = 0.0;
    bool boolean = false;
    QString text;
    Range range;

    Value() = default;
    static Value number(double n) { Value v; v.type = Type::Number; v.num = n; return v; }
    static Value boolv(bool b) { Value v; v.type = Type::Bool; v.boolean = b; return v; }
    static Value str(QString s) { Value v; v.type = Type::Text; v.text = std::move(s); return v; }
    static Value empty() { return Value{}; }
    static Value rangev(Range r) { Value v; v.type = Type::Range; v.range = std::move(r); return v; }

    bool isNumber() const { return type == Type::Number; }
    bool isRange() const { return type == Type::Range; }

    // Chuyển sang QVariant ở biên giới (trả kết quả cho model).
    QVariant toVariant() const;
    // Dựng Value từ nội dung ô thô (chuỗi/số) khi resolver trả về.
    static Value fromCell(const QVariant &cell);
};

} // namespace formula
