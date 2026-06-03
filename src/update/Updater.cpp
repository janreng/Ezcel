#include "update/Updater.h"
#include "update/VersionCompare.h"

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMessageBox>
#include <QProgressDialog>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QProcess>
#include <QApplication>
#include <QWidget>

static const char *kApi = "https://api.github.com/repos/janreng/Ezcel/releases/latest";

Updater::Updater(const QString &currentVersion, QWidget *parent)
    : QObject(parent), m_current(currentVersion), m_parent(parent) {}

void Updater::checkForUpdates(bool silentIfNone)
{
    QNetworkRequest req{QUrl(QString::fromLatin1(kApi))};
    req.setRawHeader("Accept", "application/vnd.github+json");
    req.setRawHeader("User-Agent", "Ezcel-Updater");
    QNetworkReply *reply = m_net.get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply, silentIfNone] {
        onLatestReply(reply, silentIfNone);
        reply->deleteLater();
    });
}

void Updater::onLatestReply(QNetworkReply *reply, bool silentIfNone)
{
    if (reply->error() != QNetworkReply::NoError) {
        if (!silentIfNone)
            QMessageBox::warning(m_parent, QStringLiteral("Cập nhật"),
                QStringLiteral("Không kiểm tra được bản cập nhật:\n%1").arg(reply->errorString()));
        return;
    }
    QJsonObject obj = QJsonDocument::fromJson(reply->readAll()).object();
    const QString tag = obj.value("tag_name").toString();
    if (tag.isEmpty() || !updater::isNewer(tag, m_current)) {
        if (!silentIfNone)
            QMessageBox::information(m_parent, QStringLiteral("Cập nhật"),
                QStringLiteral("Bạn đang dùng bản mới nhất (%1).").arg(m_current));
        return;
    }
    // Tìm asset .exe (ưu tiên tên có 'setup').
    QString url, name;
    const QJsonArray assets = obj.value("assets").toArray();
    for (const QJsonValue &v : assets) {
        const QString an = v.toObject().value("name").toString();
        if (an.endsWith(".exe", Qt::CaseInsensitive)) {
            url = v.toObject().value("browser_download_url").toString();
            name = an;
            if (an.contains("setup", Qt::CaseInsensitive)) break; // ưu tiên
        }
    }
    if (url.isEmpty()) {
        if (!silentIfNone)
            QMessageBox::information(m_parent, QStringLiteral("Cập nhật"),
                QStringLiteral("Có bản mới %1 nhưng chưa có file cài đính kèm.").arg(tag));
        return;
    }
    auto btn = QMessageBox::question(m_parent, QStringLiteral("Có bản mới"),
        QStringLiteral("Đã có Ezcel %1 (bạn đang dùng %2).\nTải về và cài ngay?").arg(tag, m_current));
    if (btn == QMessageBox::Yes) downloadAndRun(url, name);
}

void Updater::downloadAndRun(const QString &url, const QString &fileName)
{
    QString dest = QDir(QStandardPaths::writableLocation(QStandardPaths::TempLocation))
                       .filePath(fileName);
    auto *dlg = new QProgressDialog(QStringLiteral("Đang tải bản cập nhật..."),
                                    QStringLiteral("Hủy"), 0, 100, m_parent);
    dlg->setWindowModality(Qt::WindowModal);

    QNetworkRequest req{QUrl(url)};
    req.setRawHeader("User-Agent", "Ezcel-Updater");
    QNetworkReply *reply = m_net.get(req);

    connect(reply, &QNetworkReply::downloadProgress, dlg, [dlg](qint64 r, qint64 t) {
        if (t > 0) dlg->setValue(int(r * 100 / t));
    });
    connect(dlg, &QProgressDialog::canceled, reply, &QNetworkReply::abort);
    connect(reply, &QNetworkReply::finished, this, [this, reply, dest, dlg] {
        dlg->reset(); dlg->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            reply->deleteLater();
            QMessageBox::warning(m_parent, QStringLiteral("Cập nhật"),
                QStringLiteral("Tải thất bại."));
            return;
        }
        QFile f(dest);
        if (f.open(QIODevice::WriteOnly)) { f.write(reply->readAll()); f.close(); }
        reply->deleteLater();
        // Chạy installer rồi thoát app để cài đè.
        if (QProcess::startDetached(dest, {}))
            QApplication::quit();
        else
            QMessageBox::warning(m_parent, QStringLiteral("Cập nhật"),
                QStringLiteral("Không chạy được file cài: %1").arg(dest));
    });
}
