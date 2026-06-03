// Flash Fill (Spec 05).
#include "model/FlashFill.h"
#include <QStringList>

namespace flashfill {

// Quy luật 1: tách `src` theo một dấu phân cách, lấy token thứ idx (giữ token rỗng).
static std::optional<std::vector<QString>> tokenRule(const std::vector<QString> &sources,
                                                     const std::vector<QString> &examples)
{
    static const QString delims[] = {" ", ",", "@", "\t", ";", "-", ".", "/", "|"};
    const int m = int(examples.size());
    for (const QString &d : delims) {
        int idx = -1;
        bool ok = true;
        for (int i = 0; i < m && ok; ++i) {
            const QStringList parts = sources[i].split(d);
            int j = parts.indexOf(examples[i]);
            if (j < 0) { ok = false; break; }
            if (idx == -1) idx = j;
            else if (idx != j) { ok = false; break; }
        }
        if (ok && idx >= 0) {
            std::vector<QString> out;
            out.reserve(sources.size());
            for (const QString &s : sources) {
                const QStringList parts = s.split(d);
                out.push_back(idx < parts.size() ? parts[idx] : QString());
            }
            return out;
        }
    }
    return std::nullopt;
}

// Quy luật 2: cắt chuỗi con theo cùng vị trí bắt đầu (và cùng độ dài, hoặc tới hết).
static std::optional<std::vector<QString>> substrRule(const std::vector<QString> &sources,
                                                      const std::vector<QString> &examples)
{
    const int m = int(examples.size());
    if (examples[0].isEmpty()) return std::nullopt;
    int start = sources[0].indexOf(examples[0]);
    if (start < 0) return std::nullopt;
    int len = examples[0].size();
    bool toEnd = (start + len == sources[0].size());
    for (int i = 1; i < m; ++i) {
        if (sources[i].indexOf(examples[i]) != start) return std::nullopt;
        bool iToEnd = (start + examples[i].size() == sources[i].size());
        if (toEnd != iToEnd) return std::nullopt;
        if (!toEnd && examples[i].size() != len) return std::nullopt;
    }
    std::vector<QString> out;
    out.reserve(sources.size());
    for (const QString &s : sources) {
        if (start > s.size()) { out.push_back(QString()); continue; }
        out.push_back(toEnd ? s.mid(start) : s.mid(start, len));
    }
    return out;
}

std::optional<std::vector<QString>> infer(const std::vector<QString> &sources,
                                          const std::vector<QString> &examples)
{
    if (sources.empty() || examples.empty() || examples.size() > sources.size())
        return std::nullopt;
    if (auto r = tokenRule(sources, examples)) return r;
    if (auto r = substrRule(sources, examples)) return r;
    return std::nullopt;
}

} // namespace flashfill
