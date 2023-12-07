#include <QtImGui.h>
#include <imgui.h>
#include <QApplication>
#include <QTimer>
#include <QSurfaceFormat>
#include <QOpenGLWidget>
#include <QOpenGLExtraFunctions>
#include <QVBoxLayout>
#include <implot.h>

#include <vtkSmartPointer.h>
#include <vtkCylinderSource.h>
#include <vtkActor.h>
#include <vtkPolyDataMapper.h>
#include <vtkOutlineFilter.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <QVTKOpenGLNativeWidget.h>
#include <vtkGenericOpenGLRenderWindow.h>

vtkSmartPointer<vtkCylinderSource> cylinder;
vtkSmartPointer<vtkRenderWindow> renw;

class DemoWindow : public QOpenGLWidget, private QOpenGLExtraFunctions
{
public:
    DemoWindow()
    {
        cylinder = vtkSmartPointer<vtkCylinderSource>::New();
        cylinder->SetResolution(18);
        auto cylindermapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        cylindermapper->SetInputConnection(cylinder->GetOutputPort());
        auto cylinderactor = vtkSmartPointer<vtkActor>::New();
        cylinderactor->SetMapper(cylindermapper);
        auto renderer = vtkSmartPointer<vtkRenderer>::New();
        renderer->AddActor(cylinderactor);
        renw = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
        renw->AddRenderer(renderer);
        renderer->ResetCamera();

        auto w = new QVTKOpenGLNativeWidget{};
        w->setRenderWindow(renw);

        auto myLayout = new QVBoxLayout{ this };
        myLayout->setContentsMargins(500, 0, 0, 0);
        myLayout->addWidget(w);
    }
protected:
    void initializeGL() override
    {
        initializeOpenGLFunctions();
        QtImGui::initialize(this);
    }
    void paintGL() override
    {
        QtImGui::newFrame();

        // Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears in a window automatically called "Debug"
        {
            ImGui::ColorEdit3("clear color", (float*)&clear_color);
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        }

        if (int resolution = cylinder->GetResolution(); ImGui::SliderInt("Resolution", &resolution, 3, 100))
        {
            cylinder->SetResolution(resolution);
        }

        // Do render before ImGui UI is rendered
        glViewport(0, 0, width(), height());
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui::Render();
        QtImGui::render();
    }

private:
    ImVec4 clear_color = ImColor(114, 144, 154);
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Use OpenGL 3 Core Profile, when available
    QSurfaceFormat glFormat;
    if (QOpenGLContext::openGLModuleType() == QOpenGLContext::LibGL)
    {
        glFormat.setVersion(3, 3);
        glFormat.setProfile(QSurfaceFormat::CoreProfile);
    }
    QSurfaceFormat::setDefaultFormat(glFormat);

    // Show window
    DemoWindow w;
    w.setWindowTitle("QtImGui widget example");
    w.resize(1280, 720);
    w.show();

    // Update at 60 fps
    QTimer timer;
    //QObject::connect(&timer, SIGNAL(timeout()), &w, SLOT(update()));
    QObject::connect(&timer, &QTimer::timeout, &w, [&w]
        {
            w.update();
            ::renw->Render();
        });
    timer.start(16);

    return a.exec();
}
