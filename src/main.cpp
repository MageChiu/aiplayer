#include "mainwindow.h"

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QIcon>
#include <QSurfaceFormat>

#include <clocale>
#include <cstdio>

int main(int argc, char *argv[]) {
    setlocale(LC_NUMERIC, "C");
    QFile::remove(QDir::current().absoluteFilePath(QStringLiteral("mpv.log")));

    QSurfaceFormat format;
    format.setRenderableType(QSurfaceFormat::OpenGL);
    format.setVersion(4, 1);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
    QSurfaceFormat::setDefaultFormat(format);

    QApplication app(argc, argv);
    setlocale(LC_NUMERIC, "C");
    app.setApplicationName(QStringLiteral("AIPlayer"));
    app.setOrganizationName(QStringLiteral("MageChiu"));
    app.setWindowIcon(QIcon(QStringLiteral(":/icons/player.png")));

    MainWindow window;
    window.setWindowIcon(app.windowIcon());
    window.show();
    return app.exec();
}
