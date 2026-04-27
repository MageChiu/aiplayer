#include "mainwindow.h"
#include "core/path/apppaths.h"
#include "platform/desktop/desktopmodelcoordinator.h"
#include "platform/desktop/desktoppathprovider.h"
#include "platform/desktop/desktopplayercontroller.h"

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QIcon>
#include <QSurfaceFormat>

#include <clocale>
#include <cstdio>
#include <memory>

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
    AppPaths::setPathProvider(std::make_shared<DesktopPathProvider>());

    MainWindow window(new DesktopPlayerController(), new DesktopModelCoordinator());
    window.setWindowIcon(app.windowIcon());
    window.show();
    return app.exec();
}
