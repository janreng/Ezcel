#pragma once
#include <QObject>
#include <QString>
#include <QNetworkAccessManager>

class QWidget;
class QNetworkReply;

// Kiểm tra & tải bản cập nhật từ GitHub Releases của janreng/Ezcel (port updater.py).
// Đọc release mới nhất qua GitHub API, so phiên bản; nếu mới hơn thì tải file
// installer (.exe) rồi chạy để cài đè.
class Updater : public QObject
{
    Q_OBJECT
public:
    explicit Updater(const QString &currentVersion, QWidget *parent = nullptr);

    // Kiểm tra bản mới. silentIfNone=true: không báo gì khi đã mới nhất/khi lỗi
    // (dùng cho kiểm tra ngầm lúc khởi động).
    void checkForUpdates(bool silentIfNone);

private:
    void onLatestReply(QNetworkReply *reply, bool silentIfNone);
    void downloadAndRun(const QString &url, const QString &fileName);

    QNetworkAccessManager m_net;
    QString m_current;
    QWidget *m_parent;
};
