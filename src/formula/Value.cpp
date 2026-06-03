#include "formula/Value.h"

namespace formula {

int Range::height() const { return rows ? int(rows->size()) : 0; }
int Range::width() const { return (rows && !rows->empty()) ? int((*rows)[0].size()) : 0; }

std::vector<Value> Range::flat() const {
    std::vector<Value> out;
    if (!rows) return out;
    for (const auto &r : *rows)
        for (const auto &v : r)
            out.push_back(v);
    return out;
}

QVariant Value::toVariant() const {
    switch (type) {
    case Type::Number:  return num;
    case Type::Bool:    return boolean;
    case Type::Text:    return text;
    case Type::Empty:   return QString();
    case Type::Range: {
        // Vùng trả về biên giới: 1 ô -> ô đó; nhiều ô do _finalize chặn trước.
        auto f = range.flat();
        return f.empty() ? QVariant(QString()) : f.front().toVariant();
    }
    }
    return {};
}

Value Value::fromCell(const QVariant &cell) {
    if (!cell.isValid() || cell.isNull()) return Value::empty();
    // Số (đã tính) giữ nguyên kiểu; chuỗi rỗng -> Empty.
    if (cell.typeId() == QMetaType::Double || cell.typeId() == QMetaType::Int
        || cell.typeId() == QMetaType::LongLong) {
        return Value::number(cell.toDouble());
    }
    if (cell.typeId() == QMetaType::Bool) return Value::boolv(cell.toBool());
    const QString s = cell.toString();
    if (s.isEmpty()) return Value::empty();
    return Value::str(s);
}

} // namespace formula
