#pragma once

#include "vtkDistanceWidget.h"
#include "vtkImagePlaneWidget.h"
#include "vtkResliceImageViewer.h"
#include "vtkResliceImageViewerMeasurements.h"
#include "vtkSmartPointer.h"
#include <QOpenGLExtraFunctions>
#include <QVTKOpenGLNativeWidget.h>

#include <imgui.h>

class QtVTKRenderWindows : public QOpenGLWidget, private QOpenGLExtraFunctions
{
    Q_OBJECT
public:
    QtVTKRenderWindows(int argc, char* argv[]);
    void vtkUpdate();

protected:
    void initializeGL() override;
    void paintGL() override;

protected:
    vtkSmartPointer<vtkResliceImageViewer> riw[3];
    vtkSmartPointer<vtkImagePlaneWidget> planeWidget[3];
    vtkSmartPointer<vtkDistanceWidget> DistanceWidget[3];
    vtkSmartPointer<vtkResliceImageViewerMeasurements> ResliceMeasurements;

private:
    ImVec4 clear_color = ImColor(114, 144, 154);
    std::list<std::function<void()>> m_vtkOpList;
    QVTKOpenGLNativeWidget* view1, * view2, * view3, * view4;
};