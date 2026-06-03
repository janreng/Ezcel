// Điểm vào — tạo QApplication, mở MainWindow. Tương đương main.py/__main__.py bản Python.
#include <QApplication>
#include <QIcon>
#include "MainWindow.h"
#include "ui/Theme.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    // Giữ style Fusion như bản Python (bỏ thanh accent chọn ô của Windows 11).
    QApplication::setStyle("Fusion");
    QApplication::setFont(theme::uiFont()); // font UI Segoe UI (giống Excel)
    QApplication::setApplicationName("Ezcel");
    QApplication::setOrganizationName("PyExcel"); // giữ QSettings org cũ
    QApplication::setApplicationDisplayName("Ezcel");
    QApplication::setWindowIcon(QIcon(QStringLiteral(":/icon.png"))); // icon app (nhúng qua .qrc)

    MainWindow w;
    // Mở kèm file nếu truyền qua CLI (port dần ở P4).
    if (argc > 1) {
        w.openPath(QString::fromLocal8Bit(argv[1]));
    }
    w.resize(1100, 700);
    w.show();
    return app.exec();
}
