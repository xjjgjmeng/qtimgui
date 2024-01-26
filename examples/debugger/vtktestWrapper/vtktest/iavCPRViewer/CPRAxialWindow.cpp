#include "CPRAxialWindow.h"
#include <vtkInteractorStyle.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkDICOMImageReader.h>
#include <vtkResliceImageViewer.h>
#include <vtkInteractorStyleImage.h>
#include <vtkImageSlice.h>
#include <vtkParametricSpline.h>
#include <vtkParametricFunctionSource.h>
#include <vtkSplineFilter.h>
#include <vtkPolyLine.h>
#include <vtkLineSource.h>
#include <vtkDistanceWidget.h>

#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkRendererCollection.h>
#include <vtkPointData.h>

#include <vtkTriangle.h>
#include <vtkRegularPolygonSource.h>
#include <vtkImageSlab.h>
#include <vtkImageData.h>

#include <iavGlobalData.h>

#include "DrawCPRInteractorStyle.h"
#include "vtkParallelTransportFrame.h"


CPRAxialWindow::CPRAxialWindow(CPRDataStruct* a)
{
	// Set sharing data
	m_pCprData = a;

	//this->m_transRenW->AddRenderer(m_transRen);
	m_thickSpline = vtkSmartPointer<vtkPolyData>::New();  //red 
	m_markerLine = vtkSmartPointer<vtkPolyData>::New(); //blue
	m_adjoiningMarkerLine = vtkSmartPointer<vtkPolyData>::New();

	// Mappers
	m_splinePointsMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	m_splineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	m_thickSplineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	m_markerLineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	m_adjoiningMarkerLineMapperUpstream = vtkSmartPointer<vtkPolyDataMapper>::New();
	m_adjoiningMarkerLineMapperDownstream = vtkSmartPointer<vtkPolyDataMapper>::New();
	m_thickSplineUpMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	m_thickSplineDownMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	m_closeLineLeftMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	m_closeLineRightMapper = vtkSmartPointer<vtkPolyDataMapper>::New();

	// Actors
	m_splinePointsActor = vtkSmartPointer<vtkActor>::New();
	m_splineActor = vtkSmartPointer<vtkActor>::New();
	m_thickSplineActor = vtkSmartPointer<vtkActor>::New();
	m_markerLineActor = vtkSmartPointer<vtkActor>::New();
	m_adjoiningMarkerLineActorUpstream = vtkSmartPointer<vtkActor>::New();
	m_adjoiningMarkerLineActorDownstream = vtkSmartPointer<vtkActor>::New();
	m_thickSplineUpActor = vtkSmartPointer<vtkActor>::New();
	m_thickSplineDownActor = vtkSmartPointer<vtkActor>::New();
	m_closeLineLeftActor = vtkSmartPointer<vtkActor>::New();
	m_closeLineRightActor = vtkSmartPointer<vtkActor>::New();

	view2 = vtkSmartPointer<vtkImageViewer2>::New();
	view2->SetRenderWindow(vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New()); // 需在构建完成后重新设置window否则会弹窗
	view2->GetRenderer()->SetBackground(iavGlobalData::rendererBackground);

	using namespace std::placeholders;
	MsgCenter::attach(this, std::bind(&CPRAxialWindow::msgHandler, this, _1, _2));
}

void CPRAxialWindow::msgHandler(const MsgCenter::Msg msg, const std::any& arg)
{
	switch (msg)
	{
	case MsgCenter::CprSliceIntervalChanged:
		this->DrawAdjoiningMarkLineOnTransImage();
		break;
	case MsgCenter::CprToolLine:
		this->m_distanceWidget = vtkSmartPointer<vtkDistanceWidget>::New();
		this->m_distanceWidget->SetInteractor(this->view2->GetRenderWindow()->GetInteractor());
		this->m_distanceWidget->CreateDefaultRepresentation();
		this->m_distanceWidget->On();
		break;
	case MsgCenter::CprToolDrawArch:
		{
			auto style = vtkSmartPointer<DrawCPRInteractorStyle>::New();
			style->SetTransWindowClass(this);
			this->view2->GetRenderWindow()->GetInteractor()->SetInteractorStyle(style);
		}
		break;
	case MsgCenter::CprUpdatePanCursorLineTranslate:
		this->DrawMarkLineOnTransImage();
		this->DrawAdjoiningMarkLineOnTransImage();
		break;
	default:
		break;
	}
}

void CPRAxialWindow::loadImage()
{
	view2->SetInputData(iavGlobalData::getImageData());
	//view2->SetSlice(iavGlobalData::getImageData()->GetDimensions()[2] / 2);
	this->SetSlice(iavGlobalData::getImageData()->GetDimensions()[2] / 2);
	view2->SetColorLevel(2200);
	view2->SetColorWindow(6500);
	m_pCprData->sliceNum = iavGlobalData::getImageData()->GetDimensions()[2] / 2;
	m_pCprData->currentTransSliceImgActor = view2->GetImageActor();
}

vtkRenderWindow* CPRAxialWindow::GetRenderWindow()
{
	return view2->GetRenderWindow();
}

std::tuple<int, int, int> CPRAxialWindow::getTransverseSliceIdx() const
{
	return { this->view2->GetSliceMin(), this->view2->GetSliceMax(), this->view2->GetSlice() };
}

void CPRAxialWindow::SetSlice(const int v)
{
	this->view2->SetSlice(v);
	m_pCprData->sliceNum = v;
	MsgCenter::send(MsgCenter::CprAxialSliceChanged);
}

void CPRAxialWindow::setThickness(const int v)
{
	int numOfSlabRange = floor(0.5 * v / iavGlobalData::getImageData()->GetSpacing()[1]);

	//slab
	vtkNew<vtkImageSlab> slab;
	slab->SetMultiSliceOutput(1);
	slab->SetInputData(iavGlobalData::getImageData());
	slab->SetOrientationToX();
	slab->SetOperationToMean();
	slab->SetTrapezoidIntegration(1);
	slab->SetSliceRange(view2->GetSlice() - numOfSlabRange, view2->GetSlice() + numOfSlabRange);
	slab->SetEnableSMP(1);
	slab->Update();

	view2->SetInputData(slab->GetOutput());
	//m_viewer->SetSlice(slab->GetOutput()->GetDimensions()[0] / 2);
	view2->GetRenderWindow()->Render();
}

void CPRAxialWindow::DrawSplineOnTransImage()
{
	vtkSmartPointer<vtkParametricSpline> splineOnAxialImage = vtkSmartPointer<vtkParametricSpline>::New();
	splineOnAxialImage->ParameterizeByLengthOn();
	splineOnAxialImage->SetPoints(m_pCprData->splinePointsSet);
	splineOnAxialImage->ClosedOff();

	vtkSmartPointer<vtkParametricFunctionSource> archSplineDataOnAxialImage = vtkSmartPointer<vtkParametricFunctionSource>::New();
	archSplineDataOnAxialImage->SetParametricFunction(splineOnAxialImage);
	archSplineDataOnAxialImage->Update();

	vtkSmartPointer<vtkSplineFilter> archSplineFilterOnAxialImage = vtkSmartPointer<vtkSplineFilter>::New();
	archSplineFilterOnAxialImage->SetInputConnection(archSplineDataOnAxialImage->GetOutputPort());
	archSplineFilterOnAxialImage->SetSubdivideToLength();
	archSplineFilterOnAxialImage->SetLength(CPRDataStruct::splineResamplingDistance);
	archSplineFilterOnAxialImage->Update();

	m_pCprData->spline->DeepCopy(archSplineFilterOnAxialImage->GetOutput());

	std::cout << " m_Spline->GetNumberOfPoints(): " << m_pCprData->spline->GetNumberOfPoints() << std::endl;
	if (!this->splineMapperInTransView.Get())
	{
		this->splineMapperInTransView = vtkSmartPointer<vtkPolyDataMapper>::New();
		vtkSmartPointer<vtkActor> splineActorInTransView = vtkSmartPointer<vtkActor>::New();
		splineActorInTransView->SetMapper(splineMapperInTransView);
		splineActorInTransView->GetProperty()->SetColor(247.0 / 255, 74.0 / 255, 54.0 / 255);
		splineActorInTransView->GetProperty()->SetLineWidth(3);
		this->view2->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->AddActor(splineActorInTransView);
	}
	splineMapperInTransView->SetInputData(m_pCprData->spline);

	this->view2->GetRenderWindow()->Render();
}

// define slice size
void CPRAxialWindow::DrawMarkLineOnTransImage()
{
	double sliceSizeMm[2] = { 40, 70 };

	std::cout << "Current point id: " << m_pCprData->currentPickedPointID << std::endl;

	double normal[3];
	m_pCprData->ctrlineNormal->GetTuple(m_pCprData->currentPickedPointID, normal);

	double currentPoint[3];
	m_pCprData->spline->GetPoint(m_pCprData->currentPickedPointID, currentPoint);

	double point1[3];
	point1[0] = currentPoint[0] + (1 - 0.5) * sliceSizeMm[0] * normal[0]
								+ (0 - 0.5) * sliceSizeMm[1] * normal[0];
	point1[1] = currentPoint[1] + (1 - 0.5) * sliceSizeMm[0] * normal[1]
								+ (0 - 0.5) * sliceSizeMm[1] * normal[1];
	point1[2] = currentPoint[2];

	double point2[3];
	point2[0] = currentPoint[0] + (0 - 0.5) * sliceSizeMm[0] * normal[0]
								+ (1 - 0.5) * sliceSizeMm[1] * normal[0];
	point2[1] = currentPoint[1] + (0 - 0.5) * sliceSizeMm[0] * normal[1]
								+ (1 - 0.5) * sliceSizeMm[1] * normal[1];
	point2[2] = currentPoint[2];

	vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
	points->InsertNextPoint(point1);
	points->InsertNextPoint(point2);

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

	m_markerLineMapper->SetInputData(polyData);
	m_markerLineActor->SetMapper(m_markerLineMapper);
	m_markerLineActor->GetProperty()->SetColor(0, 153.0 / 255, 1); //RGB
	m_markerLineActor->GetProperty()->SetLineWidth(3);
	
	//for (const auto& i : { point1, point2 })
	//{
	//	vtkNew<vtkRegularPolygonSource> pSource;
	//	pSource->GeneratePolygonOn();
	//	pSource->SetNumberOfSides(10);
	//	pSource->SetRadius(1);
	//	pSource->SetCenter(i);
	//	pSource->Update();

	//	vtkNew<vtkPolyDataMapper> pMapper;
	//	pMapper->SetInputData(pSource->GetOutput());

	//	vtkNew<vtkActor> pActor;
	//	pActor->GetProperty()->SetColor(0, 153.0 / 255, 1);
	//	pActor->SetMapper(pMapper);
	//	this->view2->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->AddActor(pActor);
	//}

	this->view2->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->AddActor(m_markerLineActor);
	this->view2->GetRenderWindow()->Render();
}

// define slice size and gap
void CPRAxialWindow::DrawAdjoiningMarkLineOnTransImage()
{
	double sliceSizeMm[2] = { 40, 70 };
	//int steps = 3; // should compute from user setting in UI
	const int steps = 2 * std::round(m_pCprData->sliceInterval / CPRDataStruct::splineResamplingDistance);

	std::cout << "m_currentPointID: " << m_pCprData->currentPickedPointID << std::endl;
	// upstream line
	double upStreamNormal[3];
	m_pCprData->ctrlineNormal->GetTuple(m_pCprData->currentPickedPointID - steps, upStreamNormal);
	double upStreamPoint[3];
	m_pCprData->spline->GetPoint(m_pCprData->currentPickedPointID - steps, upStreamPoint);

	double upStreamPoint1[3];
	upStreamPoint1[0] = upStreamPoint[0] + (1 - 0.5) * sliceSizeMm[0] * upStreamNormal[0]
										 + (0 - 0.5) * sliceSizeMm[1] * upStreamNormal[0];
	upStreamPoint1[1] = upStreamPoint[1] + (1 - 0.5) * sliceSizeMm[0] * upStreamNormal[1]
										 + (0 - 0.5) * sliceSizeMm[1] * upStreamNormal[1];
	upStreamPoint1[2] = upStreamPoint[2];

	double upStreamPoint2[3];
	upStreamPoint2[0] = upStreamPoint[0] + (0 - 0.5) * sliceSizeMm[0] * upStreamNormal[0]
										 + (1 - 0.5) * sliceSizeMm[1] * upStreamNormal[0];
	upStreamPoint2[1] = upStreamPoint[1] + (0 - 0.5) * sliceSizeMm[0] * upStreamNormal[1]
										 + (1 - 0.5) * sliceSizeMm[1] * upStreamNormal[1];
	upStreamPoint2[2] = upStreamPoint[2];

	vtkSmartPointer<vtkPoints> upStreamPoints = vtkSmartPointer<vtkPoints>::New();
	upStreamPoints->InsertNextPoint(upStreamPoint1);
	upStreamPoints->InsertNextPoint(upStreamPoint2);

	vtkNew<vtkPolyLine> upStreamPolyLine;
	upStreamPolyLine->GetPointIds()->SetNumberOfIds(upStreamPoints->GetNumberOfPoints());
	for (unsigned int i = 0; i < upStreamPoints->GetNumberOfPoints(); i++)
	{
		upStreamPolyLine->GetPointIds()->SetId(i, i);
	}
	vtkNew<vtkCellArray> upStreamCells;
	upStreamCells->InsertNextCell(upStreamPolyLine);
	vtkNew<vtkPolyData> upStreamPolyData;
	upStreamPolyData->SetPoints(upStreamPoints);
	upStreamPolyData->SetLines(upStreamCells);

	// downstream line
	double downStreamNormal[3];
	m_pCprData->ctrlineNormal->GetTuple(m_pCprData->currentPickedPointID + steps, downStreamNormal);
	double downStreamPoint[3];
	m_pCprData->spline->GetPoint(m_pCprData->currentPickedPointID + steps, downStreamPoint);

	double downStreamPoint1[3];
	downStreamPoint1[0] = downStreamPoint[0] + (1 - 0.5) * sliceSizeMm[0] * downStreamNormal[0]
											 + (0 - 0.5) * sliceSizeMm[1] * downStreamNormal[0];
	downStreamPoint1[1] = downStreamPoint[1] + (1 - 0.5) * sliceSizeMm[0] * downStreamNormal[1]
											 + (0 - 0.5) * sliceSizeMm[1] * downStreamNormal[1];
	downStreamPoint1[2] = downStreamPoint[2];

	double downStreamPoint2[3];
	downStreamPoint2[0] = downStreamPoint[0] + (0 - 0.5) * sliceSizeMm[0] * downStreamNormal[0]
											 + (1 - 0.5) * sliceSizeMm[1] * downStreamNormal[0];
	downStreamPoint2[1] = downStreamPoint[1] + (0 - 0.5) * sliceSizeMm[0] * downStreamNormal[1]
											 + (1 - 0.5) * sliceSizeMm[1] * downStreamNormal[1];
	downStreamPoint2[2] = downStreamPoint[2];

	//vtkSmartPointer<vtkPoints> downStreamPoints = vtkSmartPointer<vtkPoints>::New();
	//downStreamPoints->InsertNextPoint(downStreamPoint1);
	//downStreamPoints->InsertNextPoint(downStreamPoint2);

	//vtkNew<vtkPolyLine> downStreamPolyLine;
	//downStreamPolyLine->GetPointIds()->SetNumberOfIds(downStreamPoints->GetNumberOfPoints());
	//for (unsigned int i = 0; i < downStreamPoints->GetNumberOfPoints(); i++)
	//{
	//	downStreamPolyLine->GetPointIds()->SetId(i, i);
	//}
	//vtkNew<vtkCellArray> downStreamCells;
	//downStreamCells->InsertNextCell(downStreamPolyLine);
	//vtkNew<vtkPolyData> downStreamPolyData;
	//downStreamPolyData->SetPoints(downStreamPoints);
	//downStreamPolyData->SetLines(downStreamCells);

	vtkNew<vtkLineSource> downLinesource;
	downLinesource->SetPoint1(downStreamPoint1);
	downLinesource->SetPoint2(downStreamPoint2);
	downLinesource->Update();

	m_adjoiningMarkerLineMapperUpstream->SetInputData(upStreamPolyData);
	m_adjoiningMarkerLineActorUpstream->SetMapper(m_adjoiningMarkerLineMapperUpstream);
	m_adjoiningMarkerLineActorUpstream->GetProperty()->SetColor(0, 153.0 / 255, 1); //RGB
	m_adjoiningMarkerLineActorUpstream->GetProperty()->SetLineWidth(3);
	m_adjoiningMarkerLineActorUpstream->GetProperty()->SetLineStipplePattern(0xF0F0);
	m_adjoiningMarkerLineActorUpstream->GetProperty()->SetLineStippleRepeatFactor(1);

	m_adjoiningMarkerLineMapperDownstream->SetInputData(downLinesource->GetOutput());
	m_adjoiningMarkerLineActorDownstream->SetMapper(m_adjoiningMarkerLineMapperDownstream);
	m_adjoiningMarkerLineActorDownstream->GetProperty()->SetColor(0, 153.0 / 255, 1); //RGB
	m_adjoiningMarkerLineActorDownstream->GetProperty()->SetLineWidth(3);
	m_adjoiningMarkerLineActorDownstream->GetProperty()->SetLineStipplePattern(0xF0F0);
	m_adjoiningMarkerLineActorDownstream->GetProperty()->SetLineStippleRepeatFactor(1);

	this->view2->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->AddActor(m_adjoiningMarkerLineActorUpstream);
	this->view2->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->AddActor(m_adjoiningMarkerLineActorDownstream);
	this->view2->GetRenderWindow()->Render();
}

// slice size (for aip) and thickness
void CPRAxialWindow::DrawThickSplineOnTransImage()
{
	double sliceSizeMm[2] = { 40, 70 };
	double thickness = 15; // slice num * spacing
	vtkNew<vtkPoints> pointsSet1;
	vtkNew<vtkPoints> pointsSet2;

	for (int i = 0; i < m_pCprData->spline->GetNumberOfPoints(); i++)
	{
		//std::cout << "m_currentPointID: " << i << std::endl;
		// upstream line
		double tempNormal[3];
		m_pCprData->ctrlineNormal->GetTuple(i, tempNormal);
		double tempPoint[3];
		m_pCprData->spline->GetPoint(i, tempPoint);

		double splinePoint1[3];
		splinePoint1[0] = tempPoint[0] + (1 - 0.5) * sliceSizeMm[0] * tempNormal[0]
									   + (0 - 0.5) * sliceSizeMm[1] * tempNormal[0];
		splinePoint1[1] = tempPoint[1] + (1 - 0.5) * sliceSizeMm[0] * tempNormal[1]
									   + (0 - 0.5) * sliceSizeMm[1] * tempNormal[1];
		splinePoint1[2] = tempPoint[2];

		double splinePoint2[3];
		splinePoint2[0] = tempPoint[0] + (0 - 0.5) * sliceSizeMm[0] * tempNormal[0]
									   + (1 - 0.5) * sliceSizeMm[1] * tempNormal[0];
		splinePoint2[1] = tempPoint[1] + (0 - 0.5) * sliceSizeMm[0] * tempNormal[1]
									   + (1 - 0.5) * sliceSizeMm[1] * tempNormal[1];
		splinePoint2[2] = tempPoint[2];

		pointsSet1->InsertNextPoint(splinePoint1);
		pointsSet2->InsertNextPoint(splinePoint2);
	}

	// draw top thick spline 
	vtkNew<vtkPolyLine> upPolyLine;
	upPolyLine->GetPointIds()->SetNumberOfIds(pointsSet1->GetNumberOfPoints());
	for (unsigned int i = 0; i < pointsSet1->GetNumberOfPoints(); i++)
	{
		upPolyLine->GetPointIds()->SetId(i, i);
	}
	vtkNew<vtkCellArray> upLineCells;
	upLineCells->InsertNextCell(upPolyLine);
	vtkNew<vtkPolyData> upLinePolyData;
	upLinePolyData->SetPoints(pointsSet1);
	upLinePolyData->SetLines(upLineCells);

	// draw bottom thick spline
	vtkNew<vtkPolyLine> downPolyLine;
	downPolyLine->GetPointIds()->SetNumberOfIds(pointsSet2->GetNumberOfPoints());
	for (unsigned int i = 0; i < pointsSet2->GetNumberOfPoints(); i++)
	{
		downPolyLine->GetPointIds()->SetId(i, i);
	}
	vtkNew<vtkCellArray> downLineCells;
	downLineCells->InsertNextCell(downPolyLine);
	vtkNew<vtkPolyData> downLinePolyData;
	downLinePolyData->SetPoints(pointsSet2);
	downLinePolyData->SetLines(downLineCells);

	m_thickSplineUpMapper->SetInputData(upLinePolyData);
	m_thickSplineUpActor->SetMapper(m_thickSplineUpMapper);
	m_thickSplineUpActor->GetProperty()->SetColor(247.0 / 255, 74.0 / 255, 54.0 / 255);
	m_thickSplineUpActor->GetProperty()->SetLineWidth(2);

	m_thickSplineDownMapper->SetInputData(downLinePolyData);
	m_thickSplineDownActor->SetMapper(m_thickSplineDownMapper);
	m_thickSplineDownActor->GetProperty()->SetColor(247.0 / 255, 74.0 / 255, 54.0 / 255);
	m_thickSplineDownActor->GetProperty()->SetLineWidth(2);

	// close the line
	vtkNew<vtkPoints> pointsSetCloseLine1;
	pointsSetCloseLine1->InsertNextPoint(pointsSet1->GetPoint(0));
	pointsSetCloseLine1->InsertNextPoint(pointsSet2->GetPoint(0));

	vtkNew<vtkPoints> pointsSetCloseline2;
	pointsSetCloseline2->InsertNextPoint(pointsSet1->GetPoint(pointsSet1->GetNumberOfPoints()-1));
	pointsSetCloseline2->InsertNextPoint(pointsSet2->GetPoint(pointsSet1->GetNumberOfPoints()-1));

	vtkNew<vtkPolyLine> closePolyLine1;
	closePolyLine1->GetPointIds()->SetNumberOfIds(pointsSetCloseLine1->GetNumberOfPoints());
	for (unsigned int i = 0; i < pointsSetCloseLine1->GetNumberOfPoints(); i++)
	{
		closePolyLine1->GetPointIds()->SetId(i, i);
	}
	vtkNew<vtkCellArray> closeLineCells1;
	closeLineCells1->InsertNextCell(closePolyLine1);
	vtkNew<vtkPolyData> closeLinePolyData1;
	closeLinePolyData1->SetPoints(pointsSetCloseLine1);
	closeLinePolyData1->SetLines(closeLineCells1);

	vtkNew<vtkPolyLine> closePolyLine2;
	closePolyLine2->GetPointIds()->SetNumberOfIds(pointsSetCloseline2->GetNumberOfPoints());
	for (unsigned int i = 0; i < pointsSetCloseline2->GetNumberOfPoints(); i++)
	{
		closePolyLine2->GetPointIds()->SetId(i, i);
	}
	vtkNew<vtkCellArray> closeLineCells2;
	closeLineCells2->InsertNextCell(closePolyLine2);
	vtkNew<vtkPolyData> closeLinePolyData2;
	closeLinePolyData2->SetPoints(pointsSetCloseline2);
	closeLinePolyData2->SetLines(closeLineCells2);

	m_closeLineLeftMapper->SetInputData(closeLinePolyData1);
	m_closeLineLeftActor->SetMapper(m_closeLineLeftMapper);
	m_closeLineLeftActor->GetProperty()->SetColor(247.0 / 255, 74.0 / 255, 54.0 / 255);
	m_closeLineLeftActor->GetProperty()->SetLineWidth(2);

	m_closeLineRightMapper->SetInputData(closeLinePolyData2);
	m_closeLineRightActor->SetMapper(m_closeLineRightMapper);
	m_closeLineRightActor->GetProperty()->SetColor(247.0 / 255, 74.0 / 255, 54.0 / 255);
	m_closeLineRightActor->GetProperty()->SetLineWidth(2);

	this->view2->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->AddActor(m_thickSplineUpActor);
	this->view2->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->AddActor(m_thickSplineDownActor);
	this->view2->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->AddActor(m_closeLineLeftActor);
	this->view2->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->AddActor(m_closeLineRightActor);
	this->view2->GetRenderWindow()->Render();
}

void CPRAxialWindow::ComputeSplineFrenetArray()
{
	double initialBinormalVec[3] = { 0, 0, 1 };
	// Compute tangent vector array of the spline (using parallel normal)
	vtkSmartPointer<vtkParallelTransportFrame> patrallelTrans = vtkSmartPointer<vtkParallelTransportFrame>::New();
	patrallelTrans->SetInputData(m_pCprData->spline);
	patrallelTrans->SetPreferredInitialBinormalVector(initialBinormalVec);
	patrallelTrans->Update();

	m_pCprData->ctrlineNormal->DeepCopy(patrallelTrans->GetOutput()->GetPointData()->GetAbstractArray(patrallelTrans->GetNormalsArrayName()));
	m_pCprData->ctrlineBinormal->DeepCopy(patrallelTrans->GetOutput()->GetPointData()->GetAbstractArray(patrallelTrans->GetBinormalsArrayName()));
	m_pCprData->ctrlineTangent->DeepCopy(patrallelTrans->GetOutput()->GetPointData()->GetAbstractArray(patrallelTrans->GetTangentsArrayName()));

}



