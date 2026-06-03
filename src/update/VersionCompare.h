#pragma once
#include <QString>
#include <QVector>

// So sánh phiên bản (port _parse_version / is_newer của updater.py). Thuần, test được.
namespace updater {

// "v0.20.0" / "0.20.0" -> [0,20,0]. Bỏ qua phần không phải số.
QVector<int> parseVersion(const QString &text);

// latest > current?
bool isNewer(const QString &latest, const QString &current);

} // namespace updater
