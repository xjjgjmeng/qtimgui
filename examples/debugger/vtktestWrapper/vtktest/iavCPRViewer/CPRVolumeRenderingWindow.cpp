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
#include <vtkParametricSpline.h>
#include <vtkParametricFunctionSource.h>
#include <vtkTubeFilter.h>
#include <vtkLineSource.h>

#include "CPRVolumeRenderingWindow.h"
#include "iadAlgoritmCPR.h"
#include "iavGlobalData.h"

#include <vtkAutoInit.h>
#include <vtkAnnotatedCubeActor.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkOrientationMarkerWidget.h>
VTK_MODULE_INIT(vtkRenderingVolumeOpenGL2);

CPRVolumeRenderingWindow::CPRVolumeRenderingWindow(CPRDataStruct* a)
{
	m_pCprData = a;

	m_vrRen = vtkSmartPointer<vtkRenderer>::New();
	m_vrRenW = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    this->m_vrRen->SetBackground(iavGlobalData::rendererBackground);

	m_vrRenW->AddRenderer(m_vrRen);

    if (!this->m_vrRenW->GetInteractor())
    {
#if 0
        // create a default interactor
        vtkNew<QVTKInteractor> iren;
        // iren->SetUseTDx(this->UseTDx);
        this->m_vrRenW->SetInteractor(iren);
        iren->Initialize();

        // now set the default style
        vtkNew<vtkInteractorStyleTrackballCamera> style;
        iren->SetInteractorStyle(style);
#endif
    }

    //ShowRenderingVolume();

    m_splineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    m_cprSlicePlaneMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    m_splineCurvePlaneMapper = vtkSmartPointer<vtkPolyDataMapper>::New();

    m_transSlicePlane = vtkSmartPointer<vtkPolyData>::New();
    m_curvePlane = vtkSmartPointer<vtkPolyData>::New();
    m_cprSlicePlane = vtkSmartPointer<vtkPolyData>::New();

    m_splineActor = vtkSmartPointer<vtkActor>::New();
    m_splineActor->SetMapper(m_splineMapper);
    m_splineActor->GetProperty()->SetColor(1, 1, 1);
    m_splineActor->GetProperty()->SetLineWidth(3);
    this->m_vrRen->AddActor(m_splineActor);

    m_imageOutlineActorInVRView = vtkSmartPointer<vtkActor>::New();
    //DrawTransSlicePlane();

    m_transSlicePlaneActor = vtkSmartPointer<vtkActor>::New();
    m_curvePlaneActor = vtkSmartPointer<vtkActor>::New();
    m_cprSlicePlaneActor = vtkSmartPointer<vtkActor>::New();
    actorNerveTube = vtkSmartPointer<vtkActor>::New();
    actorNerveTube2 = vtkSmartPointer<vtkActor>::New();

    implantActor = vtkSmartPointer<vtkActor>::New();

    using namespace std::placeholders;
    MsgCenter::attach(this, std::bind(&CPRVolumeRenderingWindow::msgHandler, this, _1, _2));
}

void CPRVolumeRenderingWindow::msgHandler(const MsgCenter::Msg msg, const std::any& arg)
{
    switch (msg)
    {
    case MsgCenter::CprAxialSliceChanged:
        this->DrawTransSlicePlane();
        break;
    case MsgCenter::CprUpdateSpline:
        this->DrawSplineInVRWindow();
        break;
    case MsgCenter::CprUpdateCurvePlane:
        this->DrawSplineCurvePlane();
        this->DrawSplineTangentPlane();
        break;
    case MsgCenter::CprRoiChanged:
        break;
    case MsgCenter::CprUpdataNerveTube:
        this->DrawNerveTube();
        break;
    case MsgCenter::CprUpdatePanCursorLineTranslate:
        this->DrawSplineTangentPlane();
        break;
    case MsgCenter::CprUpdatePanCursorLineRotate:
        this->DrawSplineTangentPlane();
        break;
    case MsgCenter::CprUpdateImplant:
        this->DrawImplant();
        break;
    case MsgCenter::CprUpdateImplantPosition:
        this->UpdateImplantPosition();
        break;
    default:
        break;
    }
}

vtkRenderWindow* CPRVolumeRenderingWindow::GetRenderWindow()
{
	return this->m_vrRenW;
}

void CPRVolumeRenderingWindow::DrawSplineInVRWindow()
{
    m_splineMapper->SetInputData(m_pCprData->spline);
    this->m_vrRenW->Render();
}

void CPRVolumeRenderingWindow::DrawSplineCurvePlane()
{
    //  The extrusion distance need to be set. Maybe project the spline ontot the top plane in future ? 
    vtkSmartPointer<vtkLinearExtrusionFilter> extrudeFilterUpward = vtkSmartPointer<vtkLinearExtrusionFilter>::New();
    extrudeFilterUpward->SetInputData(m_pCprData->spline);
    extrudeFilterUpward->SetExtrusionTypeToVectorExtrusion();
    extrudeFilterUpward->SetVector(0, 0, 1);  
    extrudeFilterUpward->SetScaleFactor(50); //set extrusion distance, have to decide the distance adaptively
    extrudeFilterUpward->Update();

    vtkSmartPointer<vtkLinearExtrusionFilter> extrudeFilterDownward = vtkSmartPointer<vtkLinearExtrusionFilter>::New();
    extrudeFilterDownward->SetInputData(m_pCprData->spline);
    extrudeFilterDownward->SetExtrusionTypeToVectorExtrusion();
    extrudeFilterDownward->SetVector(0, 0, -1); 
    extrudeFilterDownward->SetScaleFactor(50); 
    extrudeFilterDownward->Update();

    vtkSmartPointer<vtkAppendPolyData> appendPoly = vtkSmartPointer<vtkAppendPolyData>::New();
    appendPoly->AddInputData(extrudeFilterUpward->GetOutput());
    appendPoly->AddInputData(extrudeFilterDownward->GetOutput());
    appendPoly->Update();

    vtkSmartPointer<vtkCleanPolyData> cleanFilter =
        vtkSmartPointer<vtkCleanPolyData>::New();
    cleanFilter->SetInputConnection(appendPoly->GetOutputPort());
    cleanFilter->Update();

    // extract contour
    vtkSmartPointer<vtkFeatureEdges> featureEdges = vtkSmartPointer<vtkFeatureEdges>::New();
    featureEdges->SetInputConnection(cleanFilter->GetOutputPort());
    featureEdges->BoundaryEdgesOn();
    featureEdges->FeatureEdgesOff();
    featureEdges->ManifoldEdgesOff();
    featureEdges->NonManifoldEdgesOff();
    //featureEdges->ColoringOn();
    featureEdges->Update();

    m_splineCurvePlaneMapper->SetInputData(featureEdges->GetOutput());
    m_curvePlaneActor->SetMapper(m_splineCurvePlaneMapper);
    //m_splineCurvePlaneActorInVRView->GetProperty()->SetColor(247.0 / 255, 74.0 / 255, 54.0 / 255);
    m_curvePlaneActor->GetProperty()->SetColor(0, 1, 0);
    m_curvePlaneActor->GetProperty()->SetLineWidth(3);

    this->m_vrRen->AddActor(m_curvePlaneActor);
    this->m_vrRenW->Render();
}

void CPRVolumeRenderingWindow::DrawSplineTangentPlane()
{
    double sliceSizeMm[2] = { 40, 100 };

    // Generate probe plane using the points on the spline and the tangent vector
    vtkNew<vtkPoints> splinePoints;
    splinePoints->DeepCopy(m_pCprData->spline->GetPoints());

    int numOfPoints = splinePoints->GetNumberOfPoints();
    double centerPoint[3];

    splinePoints->GetPoint(m_pCprData->currentPickedPointID, centerPoint);

    double binormal[3];
    binormal[0] = m_pCprData->ctrlineBinormal->GetTuple3(m_pCprData->currentPickedPointID)[0];
    binormal[1] = m_pCprData->ctrlineBinormal->GetTuple3(m_pCprData->currentPickedPointID)[1];
    binormal[2] = m_pCprData->ctrlineBinormal->GetTuple3(m_pCprData->currentPickedPointID)[2];

    double rotateW[4];
    rotateW[0] = m_pCprData->panCursorLineRoateAngleRadian;
    rotateW[1] = m_pCprData->ctrlineNormal->GetTuple3(m_pCprData->currentPickedPointID)[0];
    rotateW[2] = m_pCprData->ctrlineNormal->GetTuple3(m_pCprData->currentPickedPointID)[1];
    rotateW[3] = m_pCprData->ctrlineNormal->GetTuple3(m_pCprData->currentPickedPointID)[2];

    double rotatedBinormal[3];

    vtkMath::RotateVectorByWXYZ(binormal, rotateW, rotatedBinormal);
    vtkMath::Normalize(rotatedBinormal);

    // Find two diagonal points and origin of the plane
    double axisX[3];
    double axisY[3];
    m_pCprData->ctrlineNormal->GetTuple(m_pCprData->currentPickedPointID, axisX);
    //m_pCprData->ctrlineBinormal->GetTuple(m_pCprData->currentPickedPointID, axisY);
    vtkMath::Normalize(axisX);
    //vtkMath::Normalize(axisY);

    axisY[0] = rotatedBinormal[0];
    axisY[1] = rotatedBinormal[1];
    axisY[2] = rotatedBinormal[2];

    double planePoint1[3];
    planePoint1[0] = centerPoint[0] + (1 - 0.5) * sliceSizeMm[0] * axisX[0]
                                    + (0 - 0.5) * sliceSizeMm[1] * axisY[0];
    planePoint1[1] = centerPoint[1] + (1 - 0.5) * sliceSizeMm[0] * axisX[1]
                                    + (0 - 0.5) * sliceSizeMm[1] * axisY[1];
    planePoint1[2] = centerPoint[2] + (1 - 0.5) * sliceSizeMm[0] * axisX[2]
                                    + (0 - 0.5) * sliceSizeMm[1] * axisY[2];

    double planePoint2[3];
    planePoint2[0] = centerPoint[0] + (0 - 0.5) * sliceSizeMm[0] * axisX[0]
                                    + (1 - 0.5) * sliceSizeMm[1] * axisY[0];
    planePoint2[1] = centerPoint[1] + (0 - 0.5) * sliceSizeMm[0] * axisX[1]
                                    + (1 - 0.5) * sliceSizeMm[1] * axisY[1];
    planePoint2[2] = centerPoint[2] + (0 - 0.5) * sliceSizeMm[0] * axisX[2]
                                    + (1 - 0.5) * sliceSizeMm[1] * axisY[2];

    double planeOrigin[3];
    planeOrigin[0] = centerPoint[0] + (0 - 0.5) * sliceSizeMm[0] * axisX[0]
                                    + (0 - 0.5) * sliceSizeMm[1] * axisY[0];
    planeOrigin[1] = centerPoint[1] + (0 - 0.5) * sliceSizeMm[0] * axisX[1]
                                    + (0 - 0.5) * sliceSizeMm[1] * axisY[1];
    planeOrigin[2] = centerPoint[2] + (0 - 0.5) * sliceSizeMm[0] * axisX[2]
                                    + (0 - 0.5) * sliceSizeMm[1] * axisY[2];

    double planePoint3[3];
    planePoint3[0] = centerPoint[0] + (1 - 0.5) * sliceSizeMm[0] * axisX[0]
                                    + (1 - 0.5) * sliceSizeMm[1] * axisY[0];
    planePoint3[1] = centerPoint[1] + (1 - 0.5) * sliceSizeMm[0] * axisX[1]
                                    + (1 - 0.5) * sliceSizeMm[1] * axisY[1];
    planePoint3[2] = centerPoint[2] + (1 - 0.5) * sliceSizeMm[0] * axisX[2]
                                    + (1 - 0.5) * sliceSizeMm[1] * axisY[2];

    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    points->InsertNextPoint(planeOrigin);
    points->InsertNextPoint(planePoint1);
    points->InsertNextPoint(planePoint3);
    points->InsertNextPoint(planePoint2);
    points->InsertNextPoint(planeOrigin);

    vtkNew<vtkPolyLine> polyLine;
    polyLine->GetPointIds()->SetNumberOfIds(points->GetNumberOfPoints());
    for (unsigned int i = 0; i < points->GetNumberOfPoints(); i++)
    {
        polyLine->GetPointIds()->SetId(i, i);
    }
    vtkNew<vtkCellArray> cells;
    cells->InsertNextCell(polyLine);
    vtkNew<vtkPolyData> polyData;
    polyData->SetPoints(points);
    polyData->SetLines(cells);

    //vtkNew<vtkPlaneSource> samplingPlane;
    //samplingPlane->SetOrigin(planeOrigin);
    ////samplingPlane->SetNormal(tangent); // do not need it
    //samplingPlane->SetPoint1(planePoint1);
    //samplingPlane->SetPoint2(planePoint2);
    //samplingPlane->SetResolution(50, 100);
    //samplingPlane->Update();

    // The plane should be displayed at the middle points as the spline extends
    m_cprSlicePlaneMapper->SetInputData(polyData);
    m_cprSlicePlaneActor->SetMapper(m_cprSlicePlaneMapper);
    m_cprSlicePlaneActor->GetProperty()->SetColor(0, 153.0 / 255, 1); //RGB
    m_cprSlicePlaneActor->GetProperty()->SetLineWidth(3);

    this->m_vrRen->AddActor(m_cprSlicePlaneActor);
    this->m_vrRenW->Render();

}

void CPRVolumeRenderingWindow::DrawTransSlicePlane()
{
    //show image border 
    double bounds[6];
    double spacing[3];
    iavGlobalData::getImageData()->GetSpacing(spacing);

    m_pCprData->currentTransSliceImgActor->GetBounds(bounds);
    //view2->GetImageActor()->GetBounds(bounds);
    //std::cout << " bounds x: " << bounds[0] << " " << bounds[1] << std::endl;
    //std::cout << " bounds y: " << bounds[2] << " " << bounds[3] << std::endl;
    //std::cout << " bounds z: " << bounds[4] << " " << bounds[5] << std::endl;

    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    points->InsertNextPoint(0, 0, bounds[4]);
    points->InsertNextPoint(bounds[1], 0, bounds[4]);
    points->InsertNextPoint(bounds[1], bounds[3], bounds[4]);
    points->InsertNextPoint(0, bounds[3], bounds[4]);
    points->InsertNextPoint(0, 0, bounds[4]);

    vtkNew<vtkPolyLine> polyLine;
    polyLine->GetPointIds()->SetNumberOfIds(points->GetNumberOfPoints());
    for (unsigned int i = 0; i < points->GetNumberOfPoints(); i++)
    {
        polyLine->GetPointIds()->SetId(i, i);
    }

    // Create a cell array to store the lines in and add the lines to it
    vtkNew<vtkCellArray> cells;
    cells->InsertNextCell(polyLine);

    // Create a polydata to store everything in
    vtkNew<vtkPolyData> polyData;

    // Add the points to the dataset
    polyData->SetPoints(points);

    // Add the lines to the dataset
    polyData->SetLines(cells);

    vtkSmartPointer<vtkPolyDataMapper> imageOutlineMapperInVRView = vtkSmartPointer<vtkPolyDataMapper>::New();
    imageOutlineMapperInVRView->SetInputData(polyData);


    m_imageOutlineActorInVRView->SetMapper(imageOutlineMapperInVRView);
    m_imageOutlineActorInVRView->GetProperty()->SetColor(1, 1, 0);
    m_imageOutlineActorInVRView->GetProperty()->SetLineWidth(1);

    this->m_vrRen->AddActor(m_imageOutlineActorInVRView);
    this->m_vrRenW->Render();
}

void CPRVolumeRenderingWindow::loadImage()
{

    vtkSmartPointer<vtkGPUVolumeRayCastMapper> volumeMapperGPU = vtkSmartPointer<vtkGPUVolumeRayCastMapper>::New();
    volumeMapperGPU->SetInputData(iavGlobalData::getImageData());
    volumeMapperGPU->SetImageSampleDistance(1.0);
    volumeMapperGPU->SetSampleDistance(1.0);
    volumeMapperGPU->SetAutoAdjustSampleDistances(1);
    volumeMapperGPU->SetUseJittering(1);

    vtkSmartPointer<vtkPiecewiseFunction> compositeOpacity = vtkSmartPointer<vtkPiecewiseFunction>::New();
    compositeOpacity->AddPoint(-50.0, 0.0);
    compositeOpacity->AddPoint(625.49, 0.0);
    compositeOpacity->AddPoint(1286.34, 0.0);
    compositeOpacity->AddPoint(1917.15, 0.08); // 0.7
    compositeOpacity->AddPoint(2300, 0.125); // 1.0
    compositeOpacity->AddPoint(4043.31, 0.125); // 1.0
    compositeOpacity->AddPoint(5462.06, 0.125); // 1.0 

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

    vtkSmartPointer<vtkVolume> dentVolume = vtkSmartPointer<vtkVolume>::New();
    dentVolume->SetMapper(volumeMapperGPU);
    dentVolume->SetProperty(volumeProperty);
    dentVolume->Update();

    double* pCenter = nullptr;
    double y = 0;

    pCenter = dentVolume->GetCenter();
    y = dentVolume->GetBounds()[3] - dentVolume->GetBounds()[2];
    dentVolume->SetOrigin(pCenter);
    pCenter = dentVolume->GetCenter();

    //vtkSmartPointer<vtkRenderer> ren = vtkSmartPointer<vtkRenderer>::New();
    m_vrRen->AddActor(dentVolume);
    m_vrRen->SetUseDepthPeelingForVolumes(1);
    m_vrRen->SetMaximumNumberOfPeels(100);
    m_vrRen->SetOcclusionRatio(0.5);

    m_vrRen->GetActiveCamera()->SetClippingRange(0.01, 100000.1);
    m_vrRen->GetActiveCamera()->SetViewUp(0, 0, -1);
    m_vrRen->GetActiveCamera()->SetPosition(pCenter[0], pCenter[1] + y * 2, pCenter[2]);
    m_vrRen->GetActiveCamera()->SetFocalPoint(pCenter[0], pCenter[1], pCenter[2]);

    m_vrRenW->AddRenderer(m_vrRen);

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

    m_vrRenW->Render();

    DrawTransSlicePlane();
}

void CPRVolumeRenderingWindow::DrawNerveTube()
{

    vtkNew<vtkParametricSpline> spline;
    //spline->SetPoints(m_pCprData->NervePointsSetInVrWin);
    if (m_pCprData->numOfTubes == 0)
    {
        spline->SetPoints(m_pCprData->NervePointsSetInVrWin);
    }
    else if (m_pCprData->numOfTubes == 1)
    {
        spline->SetPoints(m_pCprData->NervePointsSetInVrWin2);
    }
    vtkNew<vtkParametricFunctionSource> functionSource;
    functionSource->SetParametricFunction(spline);
    functionSource->Update();

    vtkNew<vtkTubeFilter> Tube;
    Tube->SetInputConnection(functionSource->GetOutputPort());
    Tube->SetRadius(m_pCprData->tubeRadius);
    Tube->SetNumberOfSides(20);
    Tube->CappingOn();
    Tube->SidesShareVerticesOn();
    Tube->Update();

    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputData(Tube->GetOutput());

    //actorNerveTube->SetMapper(mapper);
    //actorNerveTube->GetProperty()->SetColor(1, 0.5, 0);
    //actorNerveTube->GetProperty()->SetOpacity(1.0);

    if (m_pCprData->numOfTubes == 0)
    {
        actorNerveTube->SetMapper(mapper);
        actorNerveTube->GetProperty()->SetColor(1, 0.5, 0);
        actorNerveTube->GetProperty()->SetOpacity(0.3);

        this->m_vrRen->AddActor(actorNerveTube);
        this->m_vrRenW->Render();
    }
    else if (m_pCprData->numOfTubes == 1)
    {
        actorNerveTube2->SetMapper(mapper);
        actorNerveTube2->GetProperty()->SetColor(1, 0.5, 0);
        actorNerveTube2->GetProperty()->SetOpacity(0.3);

        this->m_vrRen->AddActor(actorNerveTube2);
        this->m_vrRenW->Render();
    }

    //this->m_vrRen->AddActor(actorNerveTube);
    //this->m_vrRenW->Render();
}

void CPRVolumeRenderingWindow::DrawImplant()
{
    vtkNew<vtkLineSource> lineSource;
    lineSource->SetPoints(m_pCprData->ImplantPointsSetInVrWin);
    lineSource->Update();

    vtkNew<vtkTubeFilter> tubeFilter;
    tubeFilter->SetInputConnection(lineSource->GetOutputPort());
    tubeFilter->SetRadius(2.5);
    tubeFilter->SetNumberOfSides(50);
    tubeFilter->CappingOn();
    tubeFilter->SidesShareVerticesOn();
    tubeFilter->Update();

    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputData(tubeFilter->GetOutput());

    //vtkNew<vtkActor> actor;
    implantActor->SetMapper(mapper);
    //actor->GetProperty()->SetOpacity(0.5);
    implantActor->GetProperty()->SetColor(1, 1, 0);

    this->m_vrRen->AddActor(implantActor);
    this->m_vrRenW->Render();
}

void CPRVolumeRenderingWindow::UpdateImplantPosition()
{
    double transformedPt0[3]; 
    TransformPanWinImplantCoordsToVrWin(m_pCprData->point0, transformedPt0);
    double transformedPt1[3];
    TransformPanWinImplantCoordsToVrWin(m_pCprData->point1, transformedPt1);

    vtkNew<vtkLineSource> lineSource;
    lineSource->SetPoint1(transformedPt0);
    lineSource->SetPoint2(transformedPt1);
    lineSource->Update();

    vtkNew<vtkTubeFilter> tubeFilter;
    tubeFilter->SetInputConnection(lineSource->GetOutputPort());
    tubeFilter->SetRadius(2.5);
    tubeFilter->SetNumberOfSides(50);
    tubeFilter->CappingOn();
    tubeFilter->SidesShareVerticesOn();
    tubeFilter->Update();

    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputData(tubeFilter->GetOutput());

    //vtkNew<vtkActor> actor;
    implantActor->SetMapper(mapper);
    //actor->GetProperty()->SetOpacity(0.5);
    implantActor->GetProperty()->SetColor(1, 1, 0);

    this->m_vrRen->AddActor(implantActor);
    this->m_vrRenW->Render();

}

void CPRVolumeRenderingWindow::TransformPanWinImplantCoordsToVrWin(double panCoords[3], double vrCoords[3])
{
    std::cout << "TransformPanWinImplantCoordsToVrWin" << std::endl;
    std::cout << "Image Bounds: " << m_pCprData->straightVolume->GetBounds()[1]
        << " " << m_pCprData->straightVolume->GetBounds()[3]
        << " " << m_pCprData->straightVolume->GetBounds()[5] << std::endl;
    std::cout << "Image Extents: " << m_pCprData->straightVolume->GetExtent()[1]
        << " " << m_pCprData->straightVolume->GetExtent()[3]
        << " " << m_pCprData->straightVolume->GetExtent()[5] << std::endl;

    double spacing[3];
    iavGlobalData::getImageData()->GetSpacing(spacing);

    double imgIJK[3];
    m_pCprData->straightVolume->TransformPhysicalPointToContinuousIndex(panCoords, imgIJK);

    std::cout << "imgIJK: " << imgIJK[0] << " " << imgIJK[1] << " " << imgIJK[2] << std::endl;

    int xIdx = floor(imgIJK[0]);
    int yIdx = floor(imgIJK[1]);
    int zIdx = m_pCprData->spline->GetNumberOfPoints() - 1 - floor(imgIJK[2]);
    std::cout << "final yIdx: " << yIdx << std::endl;
    std::cout << "final zIdx: " << zIdx << std::endl;

    double xfactor = 0;
    xfactor = fabs(xIdx - ((m_pCprData->straightVolume->GetExtent()[1] + 1) / 2));

    double yfactor = 0;
    yfactor = fabs(yIdx - ((m_pCprData->straightVolume->GetExtent()[3] + 1) / 2));

    double correspondingSplinePt[3];
    m_pCprData->spline->GetPoint(zIdx, correspondingSplinePt);

    // pan slice 
    if (xIdx > (m_pCprData->straightVolume->GetExtent()[1] + 1) / 2)
    {
        // outward 
        double* normal;
        normal = m_pCprData->ctrlineNormal->GetTuple3(zIdx);

        vrCoords[0] = correspondingSplinePt[0] + xfactor * spacing[0] * normal[0];
        vrCoords[1] = correspondingSplinePt[1] + xfactor * spacing[0] * normal[1];
        vrCoords[2] = correspondingSplinePt[2] + xfactor * spacing[0] * normal[2];
    }
    else
    {
        // inward
        double* normal;
        normal = m_pCprData->ctrlineNormal->GetTuple3(zIdx);
        normal[0] = -1 * normal[0];
        normal[1] = -1 * normal[1];
        normal[2] = -1 * normal[2];

        vrCoords[0] = correspondingSplinePt[0] + xfactor * spacing[0] * normal[0];
        vrCoords[1] = correspondingSplinePt[1] + xfactor * spacing[0] * normal[1];
        vrCoords[2] = correspondingSplinePt[2] + xfactor * spacing[0] * normal[2];
    }

    // pan height
    if (yIdx > (m_pCprData->straightVolume->GetExtent()[3] + 1) / 2)
    {
        // above spline 
        double binormal[3] = { 0, 0, -1 };

        vrCoords[0] = vrCoords[0] + yfactor * spacing[1] * binormal[0];
        vrCoords[1] = vrCoords[1] + yfactor * spacing[1] * binormal[1];
        vrCoords[2] = vrCoords[2] + yfactor * spacing[1] * binormal[2];
    }
    else
    {
        // below spline
        double binormal[3] = { 0, 0, 1 };

        vrCoords[0] = vrCoords[0] + yfactor * spacing[1] * binormal[0];
        vrCoords[1] = vrCoords[1] + yfactor * spacing[1] * binormal[1];
        vrCoords[2] = vrCoords[2] + yfactor * spacing[1] * binormal[2];
    }
}
