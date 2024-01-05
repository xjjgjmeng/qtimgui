#include "QtVTKRenderWindows.h"

#include <QGridLayout>

#include "vtkObjSetup.h"

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

QtVTKRenderWindows::QtVTKRenderWindows(int vtkNotUsed(argc), char* argv[])
{
    for (auto i : { &this->view1, &this->view2, &this->view3, &this->view4 })
    {
        *i = new QVTKOpenGLNativeWidget{};
    }
    auto pLayout = new QGridLayout{ this };
    pLayout->addWidget(this->view1, 0, 0);
    pLayout->addWidget(this->view2, 0, 1);
    pLayout->addWidget(this->view3, 1, 0);
    pLayout->addWidget(this->view4, 1, 1);
    pLayout->setContentsMargins(888, 0, 0, 0);

    vtkSmartPointer<vtkDICOMImageReader> reader = vtkSmartPointer<vtkDICOMImageReader>::New();
    //reader->SetDirectoryName(argv[1]);
    reader->SetDirectoryName("D:\\test_data\\series");
    reader->Update();
    int imageDims[3];
    reader->GetOutput()->GetDimensions(imageDims);

    for (int i = 0; i < 3; i++)
    {
        riw[i] = vtkSmartPointer<vtkResliceImageViewer>::New();
#if 0
        auto oldRep = vtkResliceCursorLineRepresentation::SafeDownCast(riw[i]->GetResliceCursorWidget()->GetResliceCursorRepresentation());
        auto thickRep = vtkSmartPointer<vtkResliceCursorThickLineRepresentation>::New();
        thickRep->GetResliceCursorActor()->GetCursorAlgorithm()->SetResliceCursor(oldRep->GetResliceCursorActor()->GetCursorAlgorithm()->GetResliceCursor());
        thickRep->GetResliceCursorActor()->GetCursorAlgorithm()->SetReslicePlaneNormal(vtkImageViewer2::SLICE_ORIENTATION_XY); //??
        riw[i]->GetResliceCursorWidget()->SetRepresentation(thickRep);
#else
        riw[i]->SetThickMode(1);
#endif
        vtkNew<vtkGenericOpenGLRenderWindow> renderWindow;
        riw[i]->SetRenderWindow(renderWindow);
    }

    view1->setRenderWindow(riw[0]->GetRenderWindow());
    riw[0]->SetupInteractor(view1->renderWindow()->GetInteractor());

    view2->setRenderWindow(riw[1]->GetRenderWindow());
    riw[1]->SetupInteractor(view2->renderWindow()->GetInteractor());

    view3->setRenderWindow(riw[2]->GetRenderWindow());
    riw[2]->SetupInteractor(view3->renderWindow()->GetInteractor());

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
    view4->setRenderWindow(renderWindow);
    view4->renderWindow()->AddRenderer(ren);
    vtkRenderWindowInteractor* iren = view4->interactor();

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

        for (auto i : { this->view1,this->view2,this->view3,this->view4, })
        {
            i->renderWindow()->GetRenderers()->GetFirstRenderer()->ResetCamera();
        }
    }
}

void QtVTKRenderWindows::vtkUpdate()
{
    if (this->m_vtkOpList.size())
    {
        for (auto& i : this->m_vtkOpList)
        {
            i();
        }
        this->m_vtkOpList.clear();
    }

    for (auto i : { this->view1,this->view2,this->view3,this->view4, })
    {
        i->renderWindow()->Render();
    }
}

void QtVTKRenderWindows::initializeGL()
{
    initializeOpenGLFunctions();
    QtImGui::initialize(this);

    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/simhei.ttf", 18.8f, nullptr, io.Fonts->GetGlyphRangesChineseFull());
}

void QtVTKRenderWindows::paintGL()
{
    QtImGui::newFrame();

    // Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears in a window automatically called "Debug"
    {
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::ColorEdit3("clear color", (float*)&clear_color);
    }

    if (ImGui::TreeNodeEx("ResliceImageViewer", ImGuiTreeNodeFlags_DefaultOpen))
    {
        {
            float near_, far_;
            double rangeVal[2];
            auto pCamera = riw[0]->GetRenderer()->GetActiveCamera();
            pCamera->GetClippingRange(rangeVal);
            near_ = rangeVal[0];
            far_ = rangeVal[1];
            if (ImGui::DragFloatRange2("ClippingRange", &near_, &far_, 0.1f, 0.0f, 1000000.0f, "Near: %lf", "Far: %lf"))
            {
                this->m_vtkOpList.push_back([this, near_, far_, rangeVal, pCamera]()mutable
                    {
                        rangeVal[0] = near_;
                        rangeVal[1] = far_;
                        pCamera->SetClippingRange(rangeVal);
                    });
            }
        }

        ImGui::TreePop();
    }

    if (bool v = riw[0]->GetThickMode(); ImGui::Checkbox("ThickMode", &v))
    {
        this->m_vtkOpList.push_back([this, v]
            {
                for (auto i = 0; i < 3; ++i)
                {
                    riw[i]->SetThickMode(v);
                    riw[i]->GetRenderer()->ResetCamera();
                }
            });
    }

    {
        const char* modeText[] = { "RESLICE_AXIS_ALIGNED", "RESLICE_OBLIQUE" };
        if (auto v = riw[0]->GetResliceMode(); ImGui::Combo("ResliceMode", &v, modeText, IM_ARRAYSIZE(modeText)))
        {
            this->m_vtkOpList.push_back([this, v]
                {
                    for (int i = 0; i < 3; i++)
                    {
                        riw[i]->SetResliceMode(v);
                        riw[i]->GetRenderer()->ResetCamera();
                    }
                });
        }
    }

    if (ImGui::TreeNodeEx("gloablCfg"))
    {
        if (riw[0]->GetThickMode())
        {
            auto pRep0 = vtkResliceCursorThickLineRepresentation::SafeDownCast(riw[0]->GetResliceCursorWidget()->GetRepresentation());
            auto pReslice0 = vtkImageSlabReslice::SafeDownCast(pRep0->GetReslice());
            {
                const char* modeText[] = { "VTK_IMAGE_SLAB_MIN", "VTK_IMAGE_SLAB_MAX", "VTK_IMAGE_SLAB_MEAN", "VTK_IMAGE_SLAB_SUM" };
                if (auto v = pReslice0->GetBlendMode(); ImGui::Combo("BlendMode", &v, modeText, IM_ARRAYSIZE(modeText)))
                {
                    this->m_vtkOpList.push_back([this, v]
                        {
                            for (int i = 0; i < 3; i++)
                            {
                                auto pRep = vtkResliceCursorThickLineRepresentation::SafeDownCast(riw[i]->GetResliceCursorWidget()->GetRepresentation());
                                auto pReslice = vtkImageSlabReslice::SafeDownCast(pRep->GetReslice());
                                pReslice->SetBlendMode(v);
                            }
                        });
                }
            }

            if (float v = pReslice0->GetSlabThickness(); ImGui::SliderFloat("SlabThickness", &v, 0.1, 30))
            {
                this->m_vtkOpList.push_back([this, v]
                    {
                        for (int i = 0; i < 3; i++)
                        {
                            auto pRep = vtkResliceCursorThickLineRepresentation::SafeDownCast(riw[i]->GetResliceCursorWidget()->GetRepresentation());
                            auto pReslice = vtkImageSlabReslice::SafeDownCast(pRep->GetReslice());
                            //pReslice->SetSlabThickness(v);
                            pRep->GetResliceCursor()->SetThickness(v, v, v);
                        }
                    });
            }
        }

        ImGui::TreePop();
    }

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
        this->m_vtkOpList.push_back([this]
            {
                // 在OBLIQUE下无法出现widget，在ALIGNED下才可以
                for (auto i = 0; i < std::size(this->riw); ++i)
                {
                    this->DistanceWidget[i] = vtkSmartPointer<vtkDistanceWidget>::New();
                    this->DistanceWidget[i]->SetInteractor(this->riw[i]->GetInteractor());
                    this->DistanceWidget[i]->CreateDefaultRepresentation();
                    this->DistanceWidget[i]->On();
                }
            }
        );
    }

    if (ImGui::Button("ResetCamera"))
    {
        this->m_vtkOpList.push_back([this]
            {
                for (auto i : { this->view1,this->view2,this->view3,this->view4, })
                {
                    i->renderWindow()->GetRenderers()->GetFirstRenderer()->ResetCamera();
                }
            });
    }

    vtkObjImgui("ResliceImageViewer", riw[0], this->m_vtkOpList, ImGuiTreeNodeFlags_DefaultOpen);

    // Do render before ImGui UI is rendered
    glViewport(0, 0, width(), height());
    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui::Render();
    QtImGui::render();
}
