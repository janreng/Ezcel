// Tự điền chuỗi lịch/quý (Spec 05).
#include "model/SeriesFill.h"
#include <QStringList>

namespace seriesfill {

// Áp kiểu hoa/thường của seed lên kết quả: TẤT CẢ HOA / tất cả thường / Viết Hoa Đầu.
static QString matchCase(const QString &seed, const QString &canonical)
{
    if (!seed.isEmpty() && seed == seed.toUpper() && seed != seed.toLower())
        return canonical.toUpper();
    if (!seed.isEmpty() && seed == seed.toLower())
        return canonical.toLower();
    return canonical; // canonical đã ở dạng Viết Hoa Đầu
}

std::optional<QString> next(const QString &seedRaw, int pos)
{
    const QString seed = seedRaw.trimmed();
    if (seed.isEmpty()) return std::nullopt;

    static const QStringList lists[] = {
        {"Mon","Tue","Wed","Thu","Fri","Sat","Sun"},
        {"Monday","Tuesday","Wednesday","Thursday","Friday","Saturday","Sunday"},
        {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"},
        {"January","February","March","April","May","June","July","August","September","October","November","December"},
        {"Q1","Q2","Q3","Q4"},
    };
    const QString low = seed.toLower();
    for (const QStringList &list : lists) {
        for (int i = 0; i < list.size(); ++i) {
            if (list[i].toLower() == low) {
                int n = list.size();
                int j = ((i + pos) % n + n) % n; // xoay vòng, hỗ trợ pos âm
                return matchCase(seed, list[j]);
            }
        }
    }
    return std::nullopt;
}

} // namespace seriesfill
