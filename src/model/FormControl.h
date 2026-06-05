#pragma once
#include <QString>

// Hộp kiểm trong ô (Form control, Spec 37): ô có giá trị TRUE/FALSE hiển thị
// dưới dạng ô vuông tích ☑/☐; bấm để đảo. Logic thuần để kiểm thử.
namespace formctl {

// Ô có phải giá trị luận lý (TRUE/FALSE, không phân biệt hoa thường)?
inline bool isBool(const QString &s)
{
    const QString t = s.trimmed();
    return t.compare(QLatin1String("TRUE"), Qt::CaseInsensitive) == 0
        || t.compare(QLatin1String("FALSE"), Qt::CaseInsensitive) == 0;
}

inline bool isTrue(const QString &s)
{
    return s.trimmed().compare(QLatin1String("TRUE"), Qt::CaseInsensitive) == 0;
}

// Đảo TRUE <-> FALSE (giữ chuẩn chữ HOA). Không phải bool -> trả "FALSE".
inline QString toggle(const QString &s)
{
    if (!isBool(s)) return QStringLiteral("FALSE");
    return isTrue(s) ? QStringLiteral("FALSE") : QStringLiteral("TRUE");
}

} // namespace formctl
