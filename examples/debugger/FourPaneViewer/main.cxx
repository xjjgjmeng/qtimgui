#include <QApplication>
#include <QSurfaceFormat>
#include <QTimer>

#include "QVTKOpenGLStereoWidget.h"
#include "QtVTKRenderWindows.h"

int main(int argc, char** argv)
{
    // QT Stuff
    QApplication app(argc, argv);

#if 0
    QSurfaceFormat::setDefaultFormat(QVTKOpenGLStereoWidget::defaultFormat()); // 使用这句话桌面背景会从ImGui界面透出来
#else
    QSurfaceFormat glFormat;
    if (QOpenGLContext::openGLModuleType() == QOpenGLContext::LibGL)
    {
        glFormat.setVersion(3, 3);
        glFormat.setProfile(QSurfaceFormat::CoreProfile);
    }
    QSurfaceFormat::setDefaultFormat(glFormat);
#endif

    QtVTKRenderWindows w(argc, argv);
    w.showMaximized();

    // Update at 60 fps
    QTimer timer;
    QObject::connect(&timer, &QTimer::timeout, &w, [&w]
        {
            w.update();
            w.vtkUpdate();
        });
    timer.start(16);

    return app.exec();
}