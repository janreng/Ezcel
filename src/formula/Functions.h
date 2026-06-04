#pragma once
#include "formula/Value.h"
#include <QString>
#include <QStringList>
#include <QHash>
#include <functional>
#include <vector>

// Registry hàm dựng sẵn (port _FUNCTIONS / _LAZY_FUNCTIONS của formula.py).
namespace formula {

using Args   = std::vector<Value>;
using Fn     = std::function<Value(const Args &)>;
using Thunk  = std::function<Value()>;          // đối số chưa tính (lazy)
using LazyFn = std::function<Value(const std::vector<Thunk> &)>;

// Tra cứu (key viết HOA). Trả nullptr nếu không có.
const Fn     *lookupFunction(const QString &upperName);
const LazyFn *lookupLazy(const QString &upperName);

// Danh sách TÊN tất cả hàm dựng sẵn (đã sắp xếp, viết HOA) — cho popup gợi ý công thức.
QStringList functionNames();

} // namespace formula
