#include <vtkCamera.h>
#include <vtkGPUVolumeRayCastMapper.h>
#include <vtkPiecewiseFunction.h>
#include <vtkColorTransferFunction.h>
#include <vtkProperty.h>
#include <vtkVolumeProperty.h>
#include <vtkLinearExtrusionFilter.h>
#include <vtkAppendPolyData.h>
#include <vtkCleanPolyData.h>
#include <vtkCleanPolyData.h>
#include <vtkFeatureEdges.h>
#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkPolyLine.h>
#include <vtkImageData.h>
#include <vtkImageReslice.h>
#include <vtkImageMapToWindowLevelColors.h>
#include <vtkPlanes.h>

#include "TMJVolumeRenderingWindow.h"

#include <vtkAutoInit.h>
#include <vtkAnnotatedCubeActor.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkOrientationMarkerWidget.h>
#include <iavGlobalData.h>
VTK_MODULE_INIT(vtkRenderingVolumeOpenGL2);

TMJVolumeRenderingWindow::TMJVolumeRenderingWindow(TMJDataStruct* a, const bool isLeft)
{
	m_pTMJData = a;  
    m_left = isLeft;

	m_vrRen = vtkSmartPointer<vtkRenderer>::New();
    this->m_vrRen->SetBackground(iavGlobalData::rendererBackground);
	m_vrRenW = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    m_pTMJData->m_Volume = vtkSmartPointer<vtkVolume>::New();  // VR volume

	m_vrRenW->AddRenderer(m_vrRen);

    //if (!this->m_vrRenW->GetInteractor())
    //{
    //    // create a default interactor
    //    vtkNew<QVTKInteractor> iren;
    //    // iren->SetUseTDx(this->UseTDx);
    //    this->m_vrRenW->SetInteractor(iren);
    //    iren->Initialize();

    //    // now set the default style
    //    vtkNew<vtkInteractorStyleTrackballCamera> style;
    //    iren->SetInteractorStyle(style);
    //}



    //ShowRenderingVolume();

    using namespace std::placeholders;
    MsgCenter::attach(this, std::bind(&TMJVolumeRenderingWindow::msgHandler, this, _1, _2));
}

void TMJVolumeRenderingWindow::msgHandler(const MsgCenter::Msg msg, const std::any& arg)
{
    switch (msg)
    {
    case MsgCenter::TMJShowRenderingVolume:
        ShowRenderingVolume();
        break;

    default:
        break;
    }
}

vtkRenderWindow* TMJVolumeRenderingWindow::GetRenderWindow()
{
	return this->m_vrRenW;
}

void TMJVolumeRenderingWindow::ShowRenderingVolume()
{
    vtkSmartPointer<vtkPiecewiseFunction> compositeOpacity = vtkSmartPointer<vtkPiecewiseFunction>::New();
    compositeOpacity->AddPoint(-50.0, 0.0);
    compositeOpacity->AddPoint(625.49, 0.0);
    compositeOpacity->AddPoint(1286.34, 0.0);
    compositeOpacity->AddPoint(1917.15, 0.7); // 0.7
    compositeOpacity->AddPoint(2300, 1.0); // 1.0
    compositeOpacity->AddPoint(4043.31, 1.0); // 1.0
    compositeOpacity->AddPoint(5462.06, 1.0); // 1.0 

    vtkSmartPointer<vtkColorTransferFunction> colorTransfer = vtkSmartPointer<vtkColorTransferFunction>::New();
    colorTransfer->AddRGBPoint(-50.38, 0.0, 1.0, 1.0);
    colorTransfer->AddHSVPoint(-50.38, 180.0 / 255.0, 1.0, 1.0);
    colorTransfer->AddRGBPoint(595.45, 76.0 / 255, 141.0 / 255, 141.0 / 255);
    colorTransfer->AddHSVPoint(595.45, 180.0 / 255.0, 118.0 / 255.0, 141.0 / 255.0);
    colorTransfer->AddRGBPoint(1196.22, 170.0 / 255, 0.0 / 255, 0.0 / 255);
    colorTransfer->AddHSVPoint(1196.22, 0.0, 1.0, 170.0 / 255.0);
    colorTransfer->AddRGBPoint(1568.38, 208.0 / 255.0, 116.0 / 255.0, 79.0 / 255.0);
    colorTransfer->AddHSVPoint(1568.38, 17.0 / 255.0, 158.0 / 255.0, 208.0 / 255.0);
    colorTransfer->AddRGBPoint(2427.80, 235.0 / 255, 197.0 / 255, 133.0 / 255);
    colorTransfer->AddHSVPoint(2427.80, 37.0 / 255.0, 111.0 / 255.0, 235.0 / 255.0);
    colorTransfer->AddRGBPoint(2989.06, 255.0 / 255, 255.0 / 255, 255.0 / 255);
    colorTransfer->AddHSVPoint(2989.06, 0.0, 0.0, 1.0);
    colorTransfer->AddRGBPoint(4680.69, 1.0, 1.0, 1.0);
    colorTransfer->AddHSVPoint(4680.69, 0.0, 0.0, 1.0);

    vtkSmartPointer<vtkVolumeProperty> volumeProperty = vtkSmartPointer<vtkVolumeProperty>::New();
    volumeProperty->SetInterpolationTypeToLinear();
    volumeProperty->ShadeOn();
    volumeProperty->SetAmbient(0.30);
    volumeProperty->SetDiffuse(0.50);
    volumeProperty->SetSpecular(0.25);
    volumeProperty->SetSpecularPower(37.5);
    volumeProperty->SetScalarOpacity(compositeOpacity);
    volumeProperty->SetColor(colorTransfer);
    //volumeProperty->SetGradientOpacity(gradientOpacity);
    volumeProperty->SetDisableGradientOpacity(1);

    vtkSmartPointer<vtkImageData> dcmImg = vtkSmartPointer<vtkImageData>::New();
    if (m_left)
    {
        dcmImg->DeepCopy(m_pTMJData->appendLeftTMJ->GetOutput());
        m_pTMJData->appendLeftTMJ->RemoveAllInputs();  // DeepCopy以后清空appendLeftTMJ中的输入
    }
    else
    {
        dcmImg->DeepCopy(m_pTMJData->appendRightTMJ->GetOutput());
        m_pTMJData->appendRightTMJ->RemoveAllInputs();
    }
    vtkSmartPointer<vtkGPUVolumeRayCastMapper> volumeMapperGPU = vtkSmartPointer<vtkGPUVolumeRayCastMapper>::New();
    volumeMapperGPU->SetInputData(dcmImg);
    volumeMapperGPU->SetImageSampleDistance(1.0);
    volumeMapperGPU->SetSampleDistance(1.0);
    volumeMapperGPU->SetAutoAdjustSampleDistances(1);
    volumeMapperGPU->SetUseJittering(1);

    m_pTMJData->m_Volume->SetMapper(volumeMapperGPU);
    m_pTMJData->m_Volume->SetProperty(volumeProperty);
    m_pTMJData->m_Volume->Update();

    double* pCenter = nullptr;
    double y = 0;
    pCenter = m_pTMJData->m_Volume->GetCenter();
    y = m_pTMJData->m_Volume->GetBounds()[3] - m_pTMJData->m_Volume->GetBounds()[2];
    m_pTMJData->m_Volume->SetOrigin(pCenter);
    pCenter = m_pTMJData->m_Volume->GetCenter();

    //vtkSmartPointer<vtkRenderer> ren = vtkSmartPointer<vtkRenderer>::New();
    m_vrRen->AddActor(m_pTMJData->m_Volume);
    m_vrRen->SetBackground(0, 0, 0);
    m_vrRen->SetUseDepthPeelingForVolumes(1);
    m_vrRen->SetMaximumNumberOfPeels(100);
    m_vrRen->SetOcclusionRatio(0.5);

    if (m_left)
    {
        m_vrRen->ResetCamera();
        m_vrRen->GetActiveCamera()->SetClippingRange(0.01, 100000.1);
        m_vrRen->GetActiveCamera()->SetViewUp(0, 0, 1);
        m_vrRen->GetActiveCamera()->SetPosition(pCenter[0], pCenter[1] + y * 2, pCenter[2]);
        m_vrRen->GetActiveCamera()->SetFocalPoint(pCenter[0], pCenter[1], pCenter[2]);
        m_vrRen->GetActiveCamera()->Zoom(1); 
    }
    else
    {
        m_vrRen->ResetCamera();
        m_vrRen->GetActiveCamera()->SetClippingRange(0.01, 100000.1);
        m_vrRen->GetActiveCamera()->SetViewUp(0, 0, -1);
        m_vrRen->GetActiveCamera()->SetPosition(pCenter[0], pCenter[1] + y * 2, pCenter[2]);
        m_vrRen->GetActiveCamera()->SetFocalPoint(pCenter[0], pCenter[1], pCenter[2]);
        m_vrRen->GetActiveCamera()->Zoom(1); 
    }

    {
        auto axesActor = vtkSmartPointer<vtkAnnotatedCubeActor>::New();
        axesActor->SetXPlusFaceText("L");
        axesActor->SetXMinusFaceText("R");
        axesActor->SetYMinusFaceText("I");
        axesActor->SetYPlusFaceText("S");
        axesActor->SetZMinusFaceText("P");
        axesActor->SetZPlusFaceText("A");
        axesActor->GetTextEdgesProperty()->SetColor(0, 1, 1);
        axesActor->GetTextEdgesProperty()->SetLineWidth(2);
        axesActor->GetCubeProperty()->SetColor(0, 0, 1);
        auto widet = vtkOrientationMarkerWidget::New();
        widet->SetOrientationMarker(axesActor);
        widet->SetOutlineColor(0.93, 0.57, 0.13);
        widet->SetInteractor(this->m_vrRenW->GetInteractor());
        widet->SetViewport(0, 0, 0.2, 0.2);
        widet->EnabledOn();
        widet->InteractiveOff();
    }
    //m_vrRenW->AddRenderer(m_vrRen);
    m_vrRenW->Render();
}

