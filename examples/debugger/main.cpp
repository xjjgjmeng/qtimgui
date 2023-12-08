#include <QtImGui.h>
#include <imgui.h>
#include <QApplication>
#include <QTimer>
#include <QSurfaceFormat>
#include <QOpenGLWidget>
#include <QOpenGLExtraFunctions>
#include <QVBoxLayout>
#include <QGridLayout>
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
#include <vtkResliceCursorWidget.h>
#include <vtkResliceCursorActor.h>
#include <vtkImagePlaneWidget.h>
#include <vtkResliceCursorLineRepresentation.h>
#include <vtkResliceCursorPolyDataAlgorithm.h>
#include <vtkPlaneSource.h>
#include <vtkResliceImageViewer.h>
#include <vtkDistanceWidget.h>
#include <vtkDICOMImageReader.h>
#include <vtkImageData.h>
#include <vtkImageMapToColors.h>
#include <vtkCellPicker.h>
#include <vtkProperty.h>
#include <vtkInteractorStyleImage.h>
#include <vtkResliceCursorThickLineRepresentation.h>
#include <vtkImageSlabReslice.h>
#include <vtkResliceCursor.h>
#include <vtkDistanceRepresentation2D.h>
#include <vtkPlane.h>
#include <vtkPointHandleRepresentation2D.h>
#include <vtkResliceImageViewerMeasurements.h>
#include <vtkBoundedPlanePointPlacer.h>

//vtkSmartPointer<vtkCylinderSource> cylinder;
//vtkSmartPointer<vtkRenderWindow> renw;

class vtkResliceCursorCallback : public vtkCommand
{
public:
    static vtkResliceCursorCallback* New() { return new vtkResliceCursorCallback; }

    void Execute(vtkObject* caller, unsigned long ev, void* callData) override
    {

        if (ev == vtkResliceCursorWidget::WindowLevelEvent || ev == vtkCommand::WindowLevelEvent ||
            ev == vtkResliceCursorWidget::ResliceThicknessChangedEvent)
        {
            // Render everything
            for (int i = 0; i < 3; i++)
            {
                this->RCW[i]->Render();
            }
            this->IPW[0]->GetInteractor()->GetRenderWindow()->Render();
            return;
        }

        vtkImagePlaneWidget* ipw = dynamic_cast<vtkImagePlaneWidget*>(caller);
        if (ipw)
        {
            double* wl = static_cast<double*>(callData);

            if (ipw == this->IPW[0])
            {
                this->IPW[1]->SetWindowLevel(wl[0], wl[1], 1);
                this->IPW[2]->SetWindowLevel(wl[0], wl[1], 1);
            }
            else if (ipw == this->IPW[1])
            {
                this->IPW[0]->SetWindowLevel(wl[0], wl[1], 1);
                this->IPW[2]->SetWindowLevel(wl[0], wl[1], 1);
            }
            else if (ipw == this->IPW[2])
            {
                this->IPW[0]->SetWindowLevel(wl[0], wl[1], 1);
                this->IPW[1]->SetWindowLevel(wl[0], wl[1], 1);
            }
        }

        vtkResliceCursorWidget* rcw = dynamic_cast<vtkResliceCursorWidget*>(caller);
        if (rcw)
        {
            vtkResliceCursorLineRepresentation* rep =
                dynamic_cast<vtkResliceCursorLineRepresentation*>(rcw->GetRepresentation());
            // Although the return value is not used, we keep the get calls
            // in case they had side-effects
            rep->GetResliceCursorActor()->GetCursorAlgorithm()->GetResliceCursor();
            for (int i = 0; i < 3; i++)
            {
                vtkPlaneSource* ps = static_cast<vtkPlaneSource*>(this->IPW[i]->GetPolyDataAlgorithm());
                ps->SetOrigin(
                    this->RCW[i]->GetResliceCursorRepresentation()->GetPlaneSource()->GetOrigin());
                ps->SetPoint1(
                    this->RCW[i]->GetResliceCursorRepresentation()->GetPlaneSource()->GetPoint1());
                ps->SetPoint2(
                    this->RCW[i]->GetResliceCursorRepresentation()->GetPlaneSource()->GetPoint2());

                // If the reslice plane has modified, update it on the 3D widget
                this->IPW[i]->UpdatePlacement();
            }
        }

        // Render everything
        for (int i = 0; i < 3; i++)
        {
            this->RCW[i]->Render();
        }
        this->IPW[0]->GetInteractor()->GetRenderWindow()->Render();
    }

    vtkResliceCursorCallback() {}
    vtkImagePlaneWidget* IPW[3];
    vtkResliceCursorWidget* RCW[3];
};

class DemoWindow : public QOpenGLWidget, private QOpenGLExtraFunctions
{
public:
    DemoWindow()
    {
        //this->ui = new Ui_QtVTKRenderWindows;
        //this->ui->setupUi(this);

        vtkSmartPointer<vtkDICOMImageReader> reader = vtkSmartPointer<vtkDICOMImageReader>::New();
        reader->SetDirectoryName("D:\\test_data\\202110020082000");
        reader->Update();
        int imageDims[3];
        reader->GetOutput()->GetDimensions(imageDims);

        for (int i = 0; i < 3; i++)
        {
            riw[i] = vtkSmartPointer<vtkResliceImageViewer>::New();
            vtkNew<vtkGenericOpenGLRenderWindow> renderWindow;
            riw[i]->SetRenderWindow(renderWindow);
        }

        for (auto i : { &view1, &view2, &view3, &view4 })
        {
            *i = new QVTKOpenGLNativeWidget{};
        }

        this->view1->setRenderWindow(this->riw[0]->GetRenderWindow());
        riw[0]->SetupInteractor(view1->renderWindow()->GetInteractor());

        this->view2->setRenderWindow(riw[1]->GetRenderWindow());
        riw[1]->SetupInteractor(this->view2->renderWindow()->GetInteractor());

        this->view3->setRenderWindow(riw[2]->GetRenderWindow());
        riw[2]->SetupInteractor(this->view3->renderWindow()->GetInteractor());

        for (int i = 0; i < 3; i++)
        {
            // make them all share the same reslice cursor object.
            vtkResliceCursorLineRepresentation* rep = vtkResliceCursorLineRepresentation::SafeDownCast(
                riw[i]->GetResliceCursorWidget()->GetRepresentation());
            riw[i]->SetResliceCursor(riw[0]->GetResliceCursor());

            rep->GetResliceCursorActor()->GetCursorAlgorithm()->SetReslicePlaneNormal(i);

            riw[i]->SetInputData(reader->GetOutput());
            riw[i]->SetSliceOrientation(i);
            riw[i]->SetResliceModeToAxisAligned();
        }

        vtkSmartPointer<vtkCellPicker> picker = vtkSmartPointer<vtkCellPicker>::New();
        picker->SetTolerance(0.005);

        vtkSmartPointer<vtkProperty> ipwProp = vtkSmartPointer<vtkProperty>::New();

        vtkSmartPointer<vtkRenderer> ren = vtkSmartPointer<vtkRenderer>::New();

        vtkNew<vtkGenericOpenGLRenderWindow> renderWindow;
        this->view4->setRenderWindow(renderWindow);
        this->view4->renderWindow()->AddRenderer(ren);
        vtkRenderWindowInteractor* iren = this->view4->interactor();

        for (int i = 0; i < 3; i++)
        {
            planeWidget[i] = vtkSmartPointer<vtkImagePlaneWidget>::New();
            planeWidget[i]->SetInteractor(iren);
            planeWidget[i]->SetPicker(picker);
            planeWidget[i]->RestrictPlaneToVolumeOn();
            double color[3] = { 0, 0, 0 };
            color[i] = 1;
            planeWidget[i]->GetPlaneProperty()->SetColor(color);

            color[0] /= 4.0;
            color[1] /= 4.0;
            color[2] /= 4.0;
            riw[i]->GetRenderer()->SetBackground(color);

            planeWidget[i]->SetTexturePlaneProperty(ipwProp);
            planeWidget[i]->TextureInterpolateOff();
            planeWidget[i]->SetResliceInterpolateToLinear();
            planeWidget[i]->SetInputConnection(reader->GetOutputPort());
            planeWidget[i]->SetPlaneOrientation(i);
            planeWidget[i]->SetSliceIndex(imageDims[i] / 2);
            planeWidget[i]->DisplayTextOn();
            planeWidget[i]->SetDefaultRenderer(ren);
            planeWidget[i]->SetWindowLevel(1358, -27);
            planeWidget[i]->On();
            planeWidget[i]->InteractionOn();
        }

        vtkSmartPointer<vtkResliceCursorCallback> cbk = vtkSmartPointer<vtkResliceCursorCallback>::New();

        for (int i = 0; i < 3; i++)
        {
            cbk->IPW[i] = planeWidget[i];
            cbk->RCW[i] = riw[i]->GetResliceCursorWidget();
            riw[i]->GetResliceCursorWidget()->AddObserver(
                vtkResliceCursorWidget::ResliceAxesChangedEvent, cbk);
            riw[i]->GetResliceCursorWidget()->AddObserver(vtkResliceCursorWidget::WindowLevelEvent, cbk);
            riw[i]->GetResliceCursorWidget()->AddObserver(
                vtkResliceCursorWidget::ResliceThicknessChangedEvent, cbk);
            riw[i]->GetResliceCursorWidget()->AddObserver(vtkResliceCursorWidget::ResetCursorEvent, cbk);
            riw[i]->GetInteractorStyle()->AddObserver(vtkCommand::WindowLevelEvent, cbk);

            // Make them all share the same color map.
            riw[i]->SetLookupTable(riw[0]->GetLookupTable());
            planeWidget[i]->GetColorMap()->SetLookupTable(riw[0]->GetLookupTable());
            // planeWidget[i]->GetColorMap()->SetInput(riw[i]->GetResliceCursorWidget()->GetResliceCursorRepresentation()->GetColorMap()->GetInput());
            planeWidget[i]->SetColorMap(
                riw[i]->GetResliceCursorWidget()->GetResliceCursorRepresentation()->GetColorMap());
        }
        //cylinder = vtkSmartPointer<vtkCylinderSource>::New();
        //cylinder->SetResolution(18);
        //auto cylindermapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        //cylindermapper->SetInputConnection(cylinder->GetOutputPort());
        //auto cylinderactor = vtkSmartPointer<vtkActor>::New();
        //cylinderactor->SetMapper(cylindermapper);
        //auto renderer = vtkSmartPointer<vtkRenderer>::New();
        //renderer->AddActor(cylinderactor);
        //renw = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
        //renw->AddRenderer(renderer);
        //renderer->ResetCamera();

        //auto w = new QVTKOpenGLNativeWidget{};
        //w->setRenderWindow(renw);

        //auto myLayout = new QVBoxLayout{ this };
        //myLayout->setContentsMargins(500, 0, 0, 0);
        //myLayout->addWidget(w);

        auto myLayout = new QGridLayout{ this };
        myLayout->setContentsMargins(500, 0, 0, 0);
        myLayout->addWidget(view1, 0, 0);
        myLayout->addWidget(view2, 0, 1);
        myLayout->addWidget(view3, 1, 0);
        myLayout->addWidget(view4, 1, 1);
    }

    void vtkRender()
    {
        for (auto i : { view1 ,view2, view3 })
        {
            i->renderWindow()->Render();
        }
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

        //if (int resolution = cylinder->GetResolution(); ImGui::SliderInt("Resolution", &resolution, 3, 100))
        //{
        //    cylinder->SetResolution(resolution);
        //}

        {
            const char* modeText[] = { "RESLICE_AXIS_ALIGNED", "RESLICE_OBLIQUE" };
            auto currentMode = riw[0]->GetResliceMode();
            if (ImGui::Combo("ResliceMode", &currentMode, modeText, IM_ARRAYSIZE(modeText)))
            {
                for (int i = 0; i < 3; i++)
                {
                    riw[i]->SetResliceMode(currentMode);
                    riw[i]->GetRenderer()->ResetCamera();
                    //riw[i]->Render();
                }
            }
        }
        {
            auto currentMode = riw[0]->GetThickMode();
            bool thickMode = 1==currentMode;
            if (ImGui::Checkbox("ThickMode", &thickMode))
            {
                for (auto i = 0; i < 3; ++i)
                {
                    riw[i]->SetThickMode(currentMode);
                }
            }
        }
#if 0
        {
            const char* slabModeText[] = { "VTK_IMAGE_SLAB_MIN", "VTK_IMAGE_SLAB_MAX", "VTK_IMAGE_SLAB_MEAN", "VTK_IMAGE_SLAB_SUM" };
            auto currentSlabMode = vtkImageSlabReslice::SafeDownCast(vtkResliceCursorThickLineRepresentation::SafeDownCast(
                riw[0]->GetResliceCursorWidget()->GetRepresentation())
                ->GetReslice())->GetBlendMode();
            if (ImGui::Combo("SlabMode", &currentSlabMode, slabModeText, IM_ARRAYSIZE(slabModeText)))
            {
                for (int i = 0; i < 3; i++)
                {
                    auto thickSlabReslice =
                        vtkImageSlabReslice::SafeDownCast(vtkResliceCursorThickLineRepresentation::SafeDownCast(
                            riw[i]->GetResliceCursorWidget()->GetRepresentation())
                            ->GetReslice());
                    thickSlabReslice->SetBlendMode(currentSlabMode);
                    riw[i]->Render();
                }
            }
        }
#endif
        if (ImGui::Button("Reset"))
        {
            for (auto i = 0; i < 3; ++i)
            {
                riw[i]->Reset();
            }

            for (auto i = 0; i < 3; ++i)
            {
                auto ps = static_cast<vtkPlaneSource*>(planeWidget[i]->GetPolyDataAlgorithm());
                ps->SetNormal(riw[0]->GetResliceCursor()->GetPlane(i)->GetNormal());
                ps->SetCenter(riw[0]->GetResliceCursor()->GetPlane(i)->GetOrigin());

                this->planeWidget[i]->UpdatePlacement();
            }
        }

        if (ImGui::Button("Distance"))
        {
            {
                int i = 1;
                // remove existing widgets.
                if (this->DistanceWidget[i])
                {
                    this->DistanceWidget[i]->SetEnabled(0);
                    this->DistanceWidget[i] = nullptr;
                }

                // add new widget
                this->DistanceWidget[i] = vtkSmartPointer<vtkDistanceWidget>::New();
                this->DistanceWidget[i]->SetInteractor(this->riw[i]->GetResliceCursorWidget()->GetInteractor());

                // Set a priority higher than our reslice cursor widget
                this->DistanceWidget[i]->SetPriority(
                    this->riw[i]->GetResliceCursorWidget()->GetPriority() + 0.01);

                vtkSmartPointer<vtkPointHandleRepresentation2D> handleRep =
                    vtkSmartPointer<vtkPointHandleRepresentation2D>::New();
                vtkSmartPointer<vtkDistanceRepresentation2D> distanceRep =
                    vtkSmartPointer<vtkDistanceRepresentation2D>::New();
                distanceRep->SetHandleRepresentation(handleRep);
                this->DistanceWidget[i]->SetRepresentation(distanceRep);
                distanceRep->InstantiateHandleRepresentation();
                distanceRep->GetPoint1Representation()->SetPointPlacer(riw[i]->GetPointPlacer());
                distanceRep->GetPoint2Representation()->SetPointPlacer(riw[i]->GetPointPlacer());

                // Add the distance to the list of widgets whose visibility is managed based
                // on the reslice plane by the ResliceImageViewerMeasurements class
                this->riw[i]->GetMeasurements()->AddItem(this->DistanceWidget[i]);

                this->DistanceWidget[i]->CreateDefaultRepresentation();
                this->DistanceWidget[i]->EnabledOn();
            }
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
    vtkSmartPointer<vtkResliceImageViewer> riw[3];
    vtkSmartPointer<vtkImagePlaneWidget> planeWidget[3];
    vtkSmartPointer<vtkDistanceWidget> DistanceWidget[3];
    vtkSmartPointer<vtkResliceImageViewerMeasurements> ResliceMeasurements;
    QVTKOpenGLNativeWidget* view1;
    QVTKOpenGLNativeWidget* view2;
    QVTKOpenGLNativeWidget* view3;
    QVTKOpenGLNativeWidget* view4;
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
    w.showMaximized();

    // Update at 60 fps
    QTimer timer;
    //QObject::connect(&timer, SIGNAL(timeout()), &w, SLOT(update()));
    QObject::connect(&timer, &QTimer::timeout, &w, [&w]
        {
            w.update();
            w.vtkRender();
            //::renw->Render();
        });
    timer.start(16);

    return a.exec();
}
