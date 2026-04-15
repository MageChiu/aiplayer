#include "mainwindow.h"

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QSurfaceFormat>

#include <clocale>
#include <cstdio>
#include <cstdlib>

int main(int argc, char *argv[]) {
    freopen("/tmp/aiplayer_stderr.log", "w", stderr);
    freopen("/tmp/aiplayer_stdout.log", "w", stdout);
    setenv("PATH", "/opt/homebrew/bin:/usr/local/bin:/usr/bin:/bin:/usr/sbin:/sbin", 1);
    setenv("DYLD_LIBRARY_PATH", "/opt/homebrew/lib:/usr/local/lib", 1);
    setenv("DYLD_FALLBACK_LIBRARY_PATH", "/opt/homebrew/lib:/usr/local/lib", 1);

    setlocale(LC_NUMERIC, "C");
    QFile::remove(QDir::current().absoluteFilePath(QStringLiteral("mpv.log")));

    QSurfaceFormat format;
    format.setRenderableType(QSurfaceFormat::OpenGL);
    format.setVersion(2, 1);
    format.setProfile(QSurfaceFormat::NoProfile);
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
    QSurfaceFormat::setDefaultFormat(format);

    QApplication app(argc, argv);
    setlocale(LC_NUMERIC, "C");
    app.setApplicationName(QStringLiteral("AIPlayer"));
    app.setOrganizationName(QStringLiteral("MageChiu"));

    MainWindow window;
    window.show();
    return app.exec();
}
