#pragma once
#include <vtkInteractorStyle.h>
#include <vtkRenderWindowInteractor.h>
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
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkRendererCollection.h>
#include <vtkPointData.h>
#include <vtkImageActorPointPlacer.h>
#include <vtkTriangle.h>
#include <vtkRegularPolygonSource.h>
#include <vtkXMLImageDataWriter.h>
#include <vtkImageReslice.h>
#include <vtkXMLImageDataWriter.h>
#include <vtkImageReslice.h>
#include <vtkImageData.h>
#include <array>

#include "AxialWindow.h"
#include "TMJDrawInteractorStyle.h"
#include "TMJSliceWindow.h"
#include <iavGlobalData.h>
//#include "vtkParallelTransportFrame.h"


AxialWindow::AxialWindow(TMJDataStruct* a)
{
	// Set sharing data
	m_pTMJData = a;

	m_biWidthLineLeftSource = vtkSmartPointer<vtkLineSource>::New();  //left
	m_biHightLineLeftSource = vtkSmartPointer<vtkLineSource>::New();
	m_biWidthLineRightSource = vtkSmartPointer<vtkLineSource>::New();  //right
	m_biHightLineRightSource = vtkSmartPointer<vtkLineSource>::New();
	m_biWidthLineLeftTopSourceBak = vtkSmartPointer<vtkLineSource>::New(); //  left top
	m_biWidthLineLeftBottomSourceBak = vtkSmartPointer<vtkLineSource>::New(); //  left bottom
	m_biWidthLineRightTopSourceBak = vtkSmartPointer<vtkLineSource>::New();  // right top
	m_biWidthLineRightBottomSourceBak = vtkSmartPointer<vtkLineSource>::New();  // right bottom

	// Mappers
	m_biWidthLineLeftMapper = vtkSmartPointer<vtkPolyDataMapper>::New(); //left
	m_biHightLineLeftMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	m_biWidthLineRightMapper = vtkSmartPointer<vtkPolyDataMapper>::New(); //right
	m_biHightLineRightMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	m_biWidthLineLeftTopMapperBak = vtkSmartPointer<vtkPolyDataMapper>::New();  // left top
	m_biWidthLineLeftBottomMapperBak = vtkSmartPointer<vtkPolyDataMapper>::New();  // left bottom
	m_biWidthLineRightTopMapperBak = vtkSmartPointer<vtkPolyDataMapper>::New();  // right top
	m_biWidthLineRightBottomMapperBak = vtkSmartPointer<vtkPolyDataMapper>::New();  // right bottom

	// Actors
	m_biWidthLineLeftActor = vtkSmartPointer<vtkActor>::New();
	m_biHightLineLeftActor = vtkSmartPointer<vtkActor>::New();
	m_biWidthLineRightActor = vtkSmartPointer<vtkActor>::New();
	m_biHightLineRightActor = vtkSmartPointer<vtkActor>::New();
	m_biWidthLineLeftTopActorBak = vtkSmartPointer<vtkActor>::New();  // left top
	m_biWidthLineLeftBottomActorBak = vtkSmartPointer<vtkActor>::New();  // left bottom
	m_biWidthLineRightTopActorBak = vtkSmartPointer<vtkActor>::New();  // right top
	m_biWidthLineRightBottomActorBak = vtkSmartPointer<vtkActor>::New();  // right bottom

	m_view = vtkSmartPointer<vtkImageViewer2>::New();
	m_view->SetRenderWindow(vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New()); // 需在构建完成后重新设置window否则会弹窗
	m_view->GetRenderer()->SetBackground(iavGlobalData::rendererBackground);

	m_pTMJData->currentTransSliceImgActor->ShallowCopy(m_view->GetImageActor());

	using namespace std::placeholders;
	MsgCenter::attach(this, std::bind(&AxialWindow::msgHandler, this, _1, _2));

}

void AxialWindow::msgHandler(const MsgCenter::Msg msg, const std::any& arg)
{
	switch (msg)
	{
	case MsgCenter::TMJToolDrawLine:
		{
			vtkNew<TMJDrawInteractorStyle> Interactorstyle;
			Interactorstyle->SetDataStruct(m_pTMJData);
			Interactorstyle->SetTransWindowClass(this);
			this->m_view->GetRenderWindow()->GetInteractor()->SetInteractorStyle(Interactorstyle);
		}
		break;

	case MsgCenter::TMJVolumeClipRequest:
		{
			this->GetLeftRectangleWorldPoints();
			this->GetRightRectangleWorldPoints();
		}
		break;

	case MsgCenter::TMJLeftTopCursorLineChanged:
		LeftTopLineRender();
		break;
	case MsgCenter::TMJRightTopCursorLineChanged:
		RightTopLineRender();
		break;
	case MsgCenter::TMJLeftBottomCursorLineChanged:
		LeftBottomLineRender();
		break;
	case MsgCenter::TMJRightBottomCursorLineChanged:
		RightBottomLineRender();
		break;

	default:
		break;
	}
}

void AxialWindow::loadImage()
{
	m_view->SetInputData(iavGlobalData::getImageData());
	this->SetSlice(iavGlobalData::getImageData()->GetDimensions()[2] / 2);
	//this->SetSlice(iavGlobalData::getImageData()->GetDimensions()[2] / 2);
	m_view->SetColorLevel(2200);
	m_view->SetColorWindow(6500);
	m_pTMJData->sliceNum = iavGlobalData::getImageData()->GetDimensions()[2] / 2;
	m_pTMJData->currentTransSliceImgActor = m_view->GetImageActor();
}

void AxialWindow::VerticalLineRenderer()
{
	double k = (m_pTMJData->point1[1] - m_pTMJData->point2[1]) / (m_pTMJData->point1[0] - m_pTMJData->point2[0]);
	double v_k = -(1 / k);
	double xDiff = halfDistanceLine / std::sqrt(1 + v_k * v_k);

	m_pTMJData->mid_point[0] = (m_pTMJData->point1[0] + m_pTMJData->point2[0]) / 2;
	m_pTMJData->mid_point[1] = (m_pTMJData->point1[1] + m_pTMJData->point2[1]) / 2;
	vPoint1[0] = m_pTMJData->mid_point[0] + xDiff;
	vPoint1[1] = m_pTMJData->mid_point[1] + v_k * xDiff;
	vPoint2[0] = m_pTMJData->mid_point[0] - xDiff;
	vPoint2[1] = m_pTMJData->mid_point[1] - v_k * xDiff;

	// mouse wheel
	m_pTMJData->vPointBak1[0] = m_pTMJData->mid_point[0] + xDiff;
	m_pTMJData->vPointBak1[1] = m_pTMJData->mid_point[1] + v_k * xDiff;
	m_pTMJData->vPointBak2[0] = m_pTMJData->mid_point[0] - xDiff;
	m_pTMJData->vPointBak2[1] = m_pTMJData->mid_point[1] - v_k * xDiff;

	vtkSmartPointer<vtkRenderer> renderer = this->m_view->GetRenderer();
	renderer->SetDisplayPoint(vPoint1[0], vPoint1[1], 0);
	renderer->DisplayToWorld();
	double VWorldPoint1[3];
	double VWorldPoint2[3];
	VWorldPoint1[0] = (renderer->GetWorldPoint())[0];
	VWorldPoint1[1] = (renderer->GetWorldPoint())[1];
	VWorldPoint1[2] = (renderer->GetWorldPoint())[2];

	renderer->SetDisplayPoint(vPoint2[0], vPoint2[1], 0);
	renderer->DisplayToWorld();
	VWorldPoint2[0] = (renderer->GetWorldPoint())[0];
	VWorldPoint2[1] = (renderer->GetWorldPoint())[1];
	VWorldPoint2[2] = (renderer->GetWorldPoint())[2];

	m_biWidthLineLeftSource->SetPoint1(VWorldPoint1);
	m_biWidthLineLeftSource->SetPoint2(VWorldPoint2);
	m_biWidthLineLeftMapper->SetInputConnection(m_biWidthLineLeftSource->GetOutputPort());
	m_biWidthLineLeftActor->SetMapper(m_biWidthLineLeftMapper);
	m_biWidthLineLeftActor->GetProperty()->SetColor(0.0, 1.0, 0.0);

	this->m_view->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->AddActor(m_biWidthLineLeftActor);
	renderer->GetRenderWindow()->GetInteractor()->Initialize();
	renderer->GetRenderWindow()->GetInteractor()->Render();

}

void AxialWindow::MirrorCross()
{
	// mirror line
	vtkSmartPointer<vtkRenderer> renderer = this->m_view->GetRenderer();
	int* renderWindowSize = renderer->GetRenderWindow()->GetSize();
	int renderWindowWidth = renderWindowSize[0];
	int renderWindowHeight = renderWindowSize[1];
	int axis_line = renderWindowWidth / 2;

	if (!m_pTMJData->isRotateP1 && !m_pTMJData->isRotateP2)  // 镜像点生成以后，不再受point1和point2变化而变化
	{
		m_pTMJData->mirrorPoint1[0] = renderWindowWidth - m_pTMJData->point1[0];
		m_pTMJData->mirrorPoint1[1] = m_pTMJData->point1[1];
		m_pTMJData->mirrorPoint2[0] = renderWindowWidth - m_pTMJData->point2[0];
		m_pTMJData->mirrorPoint2[1] = m_pTMJData->point2[1];

		m_pTMJData->mirrorPointBak1[0] = renderWindowWidth - m_pTMJData->point1[0];
		m_pTMJData->mirrorPointBak1[1] = m_pTMJData->point1[1];
		m_pTMJData->mirrorPointBak2[0] = renderWindowWidth - m_pTMJData->point2[0];
		m_pTMJData->mirrorPointBak2[1] = m_pTMJData->point2[1];
	}

	renderer->SetDisplayPoint(m_pTMJData->mirrorPoint1[0], m_pTMJData->mirrorPoint1[1], 0);
	renderer->DisplayToWorld();
	m_pTMJData->MWorldPoint1[0] = (renderer->GetWorldPoint())[0];
	m_pTMJData->MWorldPoint1[1] = (renderer->GetWorldPoint())[1];
	m_pTMJData->MWorldPoint1[2] = (renderer->GetWorldPoint())[2];

	renderer->SetDisplayPoint(m_pTMJData->mirrorPoint2[0], m_pTMJData->mirrorPoint2[1], 0);
	renderer->DisplayToWorld();
	m_pTMJData->MWorldPoint2[0] = (renderer->GetWorldPoint())[0];
	m_pTMJData->MWorldPoint2[1] = (renderer->GetWorldPoint())[1];
	m_pTMJData->MWorldPoint2[2] = (renderer->GetWorldPoint())[2];

	m_biHightLineRightSource->SetPoint1(m_pTMJData->MWorldPoint1);
	m_biHightLineRightSource->SetPoint2(m_pTMJData->MWorldPoint2);
	m_biHightLineRightMapper->SetInputConnection(m_biHightLineRightSource->GetOutputPort());
	m_biHightLineRightActor->SetMapper(m_biHightLineRightMapper);
	m_biHightLineRightActor->GetProperty()->SetColor(0.0, 0.0, 1.0);

	this->m_view->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->AddActor(m_biHightLineRightActor);
	renderer->GetRenderWindow()->GetInteractor()->Initialize();
	renderer->GetRenderWindow()->GetInteractor()->Render();

	MirrorVerticalLine(); // generate mirror vertical line
}

void AxialWindow::LineRenderer()  // WorldPoint1 and WorldPoint2
{
	vtkSmartPointer<vtkRenderer> renderer = this->m_view->GetRenderer();
	m_biHightLineLeftSource->SetPoint1(m_pTMJData->WorldPoint1);
	m_biHightLineLeftSource->SetPoint2(m_pTMJData->WorldPoint2);
	m_biHightLineLeftMapper->SetInputConnection(m_biHightLineLeftSource->GetOutputPort());
	m_biHightLineLeftActor->SetMapper(m_biHightLineLeftMapper);
	m_biHightLineLeftActor->GetProperty()->SetColor(1.0, 0.0, 0.0);

	this->m_view->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->AddActor(m_biHightLineLeftActor);
	renderer->GetRenderWindow()->GetInteractor()->Initialize();
	renderer->GetRenderWindow()->GetInteractor()->Render();
}

void AxialWindow::MirrorLineRenderer()
{
	vtkSmartPointer<vtkRenderer> renderer = this->m_view->GetRenderer();
	m_biHightLineRightSource->SetPoint1(m_pTMJData->MWorldPoint1);
	m_biHightLineRightSource->SetPoint2(m_pTMJData->MWorldPoint2);
	m_biHightLineRightMapper->SetInputConnection(m_biHightLineRightSource->GetOutputPort());
	m_biHightLineRightActor->SetMapper(m_biHightLineRightMapper);
	m_biHightLineRightActor->GetProperty()->SetColor(1.0, 1.0, 0.0);

	this->m_view->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->AddActor(m_biHightLineRightActor);
	renderer->GetRenderWindow()->GetInteractor()->Initialize();
	renderer->GetRenderWindow()->GetInteractor()->Render();
}

void AxialWindow::MirrorVerticalLine()
{
	double k = (m_pTMJData->mirrorPoint1[1] - m_pTMJData->mirrorPoint2[1]) / (m_pTMJData->mirrorPoint1[0] - m_pTMJData->mirrorPoint2[0]);
	double v_k = -(1 / k);
	double xDiff = halfDistanceLine / std::sqrt(1 + v_k * v_k);

	double spacing[3];
	iavGlobalData::getImageData()->GetSpacing(spacing);

	// mirror vertical line
	m_pTMJData->mid_mpoint[0] = (m_pTMJData->mirrorPoint1[0] + m_pTMJData->mirrorPoint2[0]) / 2;
	m_pTMJData->mid_mpoint[1] = (m_pTMJData->mirrorPoint1[1] + m_pTMJData->mirrorPoint2[1]) / 2;

	vMirrorPoint1[0] = m_pTMJData->mid_mpoint[0] + xDiff;
	vMirrorPoint1[1] = m_pTMJData->mid_mpoint[1] + v_k * xDiff;
	vMirrorPoint2[0] = m_pTMJData->mid_mpoint[0] - xDiff;
	vMirrorPoint2[1] = m_pTMJData->mid_mpoint[1] - v_k * xDiff;

	// mouse wheel
	m_pTMJData->vMirrorPointBak1[0] = m_pTMJData->mid_mpoint[0] + xDiff;
	m_pTMJData->vMirrorPointBak1[1] = m_pTMJData->mid_mpoint[1] + v_k * xDiff;
	m_pTMJData->vMirrorPointBak2[0] = m_pTMJData->mid_mpoint[0] - xDiff;
	m_pTMJData->vMirrorPointBak2[1] = m_pTMJData->mid_mpoint[1] - v_k * xDiff;

	vtkSmartPointer<vtkRenderer> renderer = this->m_view->GetRenderer();
	renderer->SetDisplayPoint(vMirrorPoint1[0], vMirrorPoint1[1], 0);
	renderer->DisplayToWorld();
	double MVWorldPoint1[3];
	MVWorldPoint1[0] = (renderer->GetWorldPoint())[0];
	MVWorldPoint1[1] = (renderer->GetWorldPoint())[1];
	MVWorldPoint1[2] = (renderer->GetWorldPoint())[2];

	renderer->SetDisplayPoint(vMirrorPoint2[0], vMirrorPoint2[1], 0);
	renderer->DisplayToWorld();
	double MVWorldPoint2[3];
	MVWorldPoint2[0] = (renderer->GetWorldPoint())[0];
	MVWorldPoint2[1] = (renderer->GetWorldPoint())[1];
	MVWorldPoint2[2] = (renderer->GetWorldPoint())[2];

	m_biWidthLineRightSource->SetPoint1(MVWorldPoint1);
	m_biWidthLineRightSource->SetPoint2(MVWorldPoint2);
	m_biWidthLineRightMapper->SetInputConnection(m_biWidthLineRightSource->GetOutputPort());
	m_biWidthLineRightActor->SetMapper(m_biWidthLineRightMapper);
	m_biWidthLineRightActor->GetProperty()->SetColor(0.0, 1.0, 1.0);

	this->m_view->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->AddActor(m_biWidthLineRightActor);
	renderer->GetRenderWindow()->GetInteractor()->Initialize();
	renderer->GetRenderWindow()->GetInteractor()->Render();

	// 在这里发信号
	/*MsgCenter::send(MsgCenter::TMJImageSlice);*/
}

void AxialWindow::TranslateCrosshair()
{
	// line
	double lengthLine = GetDistance(m_pTMJData->point1, m_pTMJData->point2);
	double k = (m_pTMJData->point1[1] - m_pTMJData->point2[1]) / (m_pTMJData->point1[0] - m_pTMJData->point2[0]);
	double xDiff = lengthLine * 0.5 / std::sqrt(1 + k * k);
	m_pTMJData->point1[0] = m_pTMJData->mid_point[0] + xDiff;
	m_pTMJData->point1[1] = m_pTMJData->mid_point[1] + k * xDiff;
	m_pTMJData->point2[0] = m_pTMJData->mid_point[0] - xDiff;
	m_pTMJData->point2[1] = m_pTMJData->mid_point[1] - k * xDiff;

	// 如果平移crosshair后，坐标点改变
	m_pTMJData->vPointBak1[0] = m_pTMJData->mid_point[0] + xDiff;
	m_pTMJData->vPointBak1[1] = m_pTMJData->mid_point[1] + k * xDiff;
	m_pTMJData->vPointBak2[0] = m_pTMJData->mid_point[0] - xDiff;
	m_pTMJData->vPointBak2[1] = m_pTMJData->mid_point[1] - k * xDiff;

	vtkSmartPointer<vtkRenderer> renderer = this->m_view->GetRenderer();
	renderer->SetDisplayPoint(m_pTMJData->point1[0], m_pTMJData->point1[1], 0);
	renderer->DisplayToWorld();
	m_pTMJData->WorldPoint1[0] = (renderer->GetWorldPoint())[0];
	m_pTMJData->WorldPoint1[1] = (renderer->GetWorldPoint())[1];
	m_pTMJData->WorldPoint1[2] = (renderer->GetWorldPoint())[2];

	renderer->SetDisplayPoint(m_pTMJData->point2[0], m_pTMJData->point2[1], 0);
	renderer->DisplayToWorld();
	m_pTMJData->WorldPoint2[0] = (renderer->GetWorldPoint())[0];
	m_pTMJData->WorldPoint2[1] = (renderer->GetWorldPoint())[1];
	m_pTMJData->WorldPoint2[2] = (renderer->GetWorldPoint())[2];

	LineRenderer();
	VerticalLineRenderer();
}

void AxialWindow::TranslateMirrorCrosshair()
{
	// mirror line
	double lengthLine = GetDistance(m_pTMJData->mirrorPoint1, m_pTMJData->mirrorPoint2);
	double k = (m_pTMJData->mirrorPoint1[1] - m_pTMJData->mirrorPoint2[1]) / (m_pTMJData->mirrorPoint1[0] - m_pTMJData->mirrorPoint2[0]);
	double xDiff = lengthLine * 0.5 / std::sqrt(1 + k * k);
	m_pTMJData->mirrorPoint1[0] = m_pTMJData->mid_mpoint[0] + xDiff;
	m_pTMJData->mirrorPoint1[1] = m_pTMJData->mid_mpoint[1] + k * xDiff;
	m_pTMJData->mirrorPoint2[0] = m_pTMJData->mid_mpoint[0] - xDiff;
	m_pTMJData->mirrorPoint2[1] = m_pTMJData->mid_mpoint[1] - k * xDiff;

	// 如果平移crosshair后，坐标点改变
	m_pTMJData->mirrorPointBak1[0] = m_pTMJData->mid_mpoint[0] + xDiff;
	m_pTMJData->mirrorPointBak1[1] = m_pTMJData->mid_mpoint[1] + k * xDiff;
	m_pTMJData->mirrorPointBak2[0] = m_pTMJData->mid_mpoint[0] - xDiff;
	m_pTMJData->mirrorPointBak2[1] = m_pTMJData->mid_mpoint[1] - k * xDiff;

	vtkSmartPointer<vtkRenderer> renderer = this->m_view->GetRenderer();
	renderer->SetDisplayPoint(m_pTMJData->mirrorPoint1[0], m_pTMJData->mirrorPoint1[1], 0);
	renderer->DisplayToWorld();
	m_pTMJData->MWorldPoint1[0] = (renderer->GetWorldPoint())[0];
	m_pTMJData->MWorldPoint1[1] = (renderer->GetWorldPoint())[1];
	m_pTMJData->MWorldPoint1[2] = (renderer->GetWorldPoint())[2];

	renderer->SetDisplayPoint(m_pTMJData->mirrorPoint2[0], m_pTMJData->mirrorPoint2[1], 0);
	renderer->DisplayToWorld();
	m_pTMJData->MWorldPoint2[0] = (renderer->GetWorldPoint())[0];
	m_pTMJData->MWorldPoint2[1] = (renderer->GetWorldPoint())[1];
	m_pTMJData->MWorldPoint2[2] = (renderer->GetWorldPoint())[2];

	MirrorLineRenderer();
	MirrorVerticalLine();
}

vtkSmartPointer<vtkImageAppend> AxialWindow::getImageData(const bool isLeft)
{
	vtkSmartPointer<vtkImageAppend> retval = vtkSmartPointer<vtkImageAppend>::New();

	auto& worldPtArray = isLeft ? m_pTMJData->l_worldArray : m_pTMJData->r_worldArray;

	worldPtArray.clear();

	//double l_p1[2], l_p2[2], l_p3[2], l_p4[2];  // points of retangle
	double l_p1[2];
	double l_p2[2];
	double l_p3[2];
	double l_p4[2];

	{
		auto displayPt1 = isLeft ? m_pTMJData->point1 : m_pTMJData->mirrorPoint1;
		auto displayPt2 = isLeft ? m_pTMJData->point2 : m_pTMJData->mirrorPoint2;
		auto displayPt1v = isLeft ? this->vPoint1 : this->vMirrorPoint1;
		auto displayPt2v = isLeft ? this->vPoint2 : this->vMirrorPoint2;

		double l_k = (displayPt1[1] - displayPt2[1]) / (displayPt1[0] - displayPt2[0]);
		double l_b1, l_b2, l_b3, l_b4;
		l_b1 = displayPt1[1] + 1 / l_k * displayPt1[0];
		l_b2 = displayPt2[1] + 1 / l_k * displayPt2[0];
		l_b3 = displayPt1v[1] - l_k * displayPt1v[0];
		l_b4 = displayPt2v[1] - l_k * displayPt2v[0];

		// intersection of p3 and p5
		l_p1[0] = (l_b3 - l_b1) / (-1 / l_k - l_k);
		l_p1[1] = l_b3 + l_k * l_p1[0];
		// intersection of p4 and p5
		l_p2[0] = (l_b3 - l_b2) / (-1 / l_k - l_k);
		l_p2[1] = l_b3 + l_k * l_p2[0];
		// intersection of p4 and p6
		l_p3[0] = (l_b4 - l_b2) / (-1 / l_k - l_k);
		l_p3[1] = l_b4 + l_k * l_p3[0];
		// intersection of p6 and p3
		l_p4[0] = (l_b4 - l_b1) / (-1 / l_k - l_k);
		l_p4[1] = l_b4 + l_k * l_p4[0];
	}

	double* left_array[4] = { l_p1,l_p2,l_p3,l_p4 };

	for (int i = 0; i < std::size(left_array); i++) {
		//double* pickedImagePos = new double[3];
		std::array<double, 3> pickedImagePos;
		double orient[9];
		vtkSmartPointer<vtkImageActorPointPlacer> imagePointPicker = vtkSmartPointer<vtkImageActorPointPlacer>::New();
		imagePointPicker->SetImageActor(this->m_view->GetImageActor());
		imagePointPicker->ComputeWorldPosition(this->m_view->GetRenderWindow()->GetRenderers()->GetFirstRenderer(), left_array[i], pickedImagePos.data(), orient);
		//std::cout << "Display value(display): " << left_array[i][0] << " " << left_array[i][1] << std::endl;
		//std::cout << "Picked value (world): " << pickedImagePos[0] << " " << pickedImagePos[1] << " " << pickedImagePos[2] << std::endl;
		worldPtArray.push_back(pickedImagePos);
	}

	//this->LeftImageReslice();
		// 清空appendTMJ，防止堆叠
	//m_pTMJData->appendLeftTMJ->RemoveAllInputs();

	double spacing[3] = { 0.0 };
	iavGlobalData::getImageData()->GetSpacing(spacing);
	int CountSlice = numberOfSlice / spacing[2];
	double z_point = 0.0;
	retval->SetAppendAxis(2);
	//std::cout << "l_worldArray size: " << m_pTMJData->l_worldArray.size() << std::endl;
	// resliceMatrix
	double* p1 = worldPtArray[0].data();
	double* p2 = worldPtArray[1].data();
	double* p3 = worldPtArray[2].data();
	double* p4 = worldPtArray[3].data();
	double cur_zpoint = p1[2];

	vtkSmartPointer<vtkMatrix4x4> resliceMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
	double xNormal[3] = { p2[0] - p3[0] , p2[1] - p3[1] ,0 };
	double yBiNormal[3] = { p4[0] - p3[0] , p4[1] - p3[1] ,0 };
	vtkMath::Normalize(xNormal);
	vtkMath::Normalize(yBiNormal);
	resliceMatrix->Identity();
	resliceMatrix->SetElement(0, 0, xNormal[0]);  //X
	resliceMatrix->SetElement(1, 0, xNormal[1]);
	resliceMatrix->SetElement(2, 0, xNormal[2]);
	resliceMatrix->SetElement(0, 1, yBiNormal[0]);  //Y
	resliceMatrix->SetElement(1, 1, yBiNormal[1]);
	resliceMatrix->SetElement(2, 1, yBiNormal[2]);
	resliceMatrix->SetElement(0, 2, 0);  //Z
	resliceMatrix->SetElement(1, 2, 0);
	resliceMatrix->SetElement(2, 2, 1);

	double up_zpoint = cur_zpoint + spacing[2] * CountSlice * 0.5;  	// up->50，down->100
	p1[2] = p2[2] = p3[2] = p4[2] = up_zpoint;
	for (int index_slice = 0; index_slice < CountSlice; index_slice++)
	{
		z_point = up_zpoint - index_slice * spacing[2];
		p1[2] = p2[2] = p3[2] = p4[2] = z_point;
		resliceMatrix->SetElement(0, 3, p3[0]); //origin
		resliceMatrix->SetElement(1, 3, p3[1]);
		resliceMatrix->SetElement(2, 3, p3[2]);

		double w_dist = GetDistance(p2, p3);
		double h_dist = GetDistance(p3, p4);

		vtkSmartPointer<vtkImageReslice> reslice = vtkSmartPointer<vtkImageReslice>::New();
		reslice->SetInputData(iavGlobalData::getImageData());
		reslice->TransformInputSamplingOff();
		//reslice->SetOutputDimensionality(2);
		reslice->SetInterpolationModeToLinear();
		reslice->SetResliceAxes(resliceMatrix);
		reslice->SetOutputSpacing(spacing);
		reslice->SetOutputExtent(0, w_dist * 4, 0, h_dist * 4, 0, 0);
		reslice->SetOutputOrigin(-(w_dist - 1) * 0.1 * spacing[0], -(h_dist - 1) * 0.4 * spacing[1], 0);
		reslice->Update();
		retval->AddInputData(reslice->GetOutput());
	}
	retval->Update();

	worldPtArray.clear();

	return retval;
}

void AxialWindow::GetLeftRectangleWorldPoints()
{
	m_pTMJData->appendLeftTMJ = getImageData(true);
	MsgCenter::send(MsgCenter::TMJShowRenderingVolume);
	MsgCenter::send(MsgCenter::TMJShowImageViewer);
}

void AxialWindow::GetRightRectangleWorldPoints()
{
	m_pTMJData->appendRightTMJ = getImageData(false);
	MsgCenter::send(MsgCenter::TMJShowRenderingVolume);
	MsgCenter::send(MsgCenter::TMJShowImageViewer);
}

vtkRenderWindow* AxialWindow::GetRenderWindow()
{
	return m_view->GetRenderWindow();
}

void AxialWindow::StartBidirectionalDrawing()
{

	this->m_view->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->AddActor(m_biHightLineLeftActor);
	this->m_view->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->AddActor(m_biWidthLineLeftActor);
	this->m_view->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->AddActor(m_biHightLineRightActor);
	this->m_view->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->AddActor(m_biWidthLineRightActor);
	this->m_view->GetRenderWindow()->Render();
}

std::tuple<int, int, int> AxialWindow::getTransverseSliceIdx() const
{
	return { this->m_view->GetSliceMin(), this->m_view->GetSliceMax(), this->m_view->GetSlice() };
}

void AxialWindow::SetSlice(const int v)
{
	this->m_view->SetSlice(v);
	MsgCenter::send(MsgCenter::TMJAxialSliceChanged);
}

void AxialWindow::setThickness(const int v)
{
}

double AxialWindow::GetDistance(double* p2, double* p1)
{
	double dx = p2[0] - p1[0];
	double dy = p2[1] - p1[1];
	return std::sqrt(dx * dx + dy * dy);
}

void AxialWindow::LeftTopLineRender()  // left top
{
	vtkSmartPointer<vtkRenderer> renderer = this->m_view->GetRenderer();
	renderer->SetDisplayPoint(m_pTMJData->vPointBak1[0], m_pTMJData->vPointBak1[1], 0);
	renderer->DisplayToWorld();
	double VWorldPointBak1[3];
	double VWorldPointBak2[3];
	VWorldPointBak1[0] = (renderer->GetWorldPoint())[0];
	VWorldPointBak1[1] = (renderer->GetWorldPoint())[1];
	VWorldPointBak1[2] = (renderer->GetWorldPoint())[2];

	renderer->SetDisplayPoint(m_pTMJData->vPointBak2[0], m_pTMJData->vPointBak2[1], 0);
	renderer->DisplayToWorld();
	VWorldPointBak2[0] = (renderer->GetWorldPoint())[0];
	VWorldPointBak2[1] = (renderer->GetWorldPoint())[1];
	VWorldPointBak2[2] = (renderer->GetWorldPoint())[2];

	m_biWidthLineLeftTopSourceBak->SetPoint1(VWorldPointBak1);
	m_biWidthLineLeftTopSourceBak->SetPoint2(VWorldPointBak2);
	m_biWidthLineLeftTopMapperBak->SetInputConnection(m_biWidthLineLeftTopSourceBak->GetOutputPort());
	m_biWidthLineLeftTopActorBak->SetMapper(m_biWidthLineLeftTopMapperBak);
	m_biWidthLineLeftTopActorBak->GetProperty()->SetColor(1.0, 1.0, 1.0);

	this->m_view->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->AddActor(m_biWidthLineLeftTopActorBak);
	renderer->GetRenderWindow()->GetInteractor()->Initialize();
	renderer->GetRenderWindow()->GetInteractor()->Render();
	MsgCenter::send(MsgCenter::TMJShowImageViewer);
}

void AxialWindow::LeftBottomLineRender()  // left bottom
{
	vtkSmartPointer<vtkRenderer> renderer = this->m_view->GetRenderer();
	renderer->SetDisplayPoint(m_pTMJData->pointBak1[0], m_pTMJData->pointBak1[1], 0);
	renderer->DisplayToWorld();
	double pointBak1[3];
	double pointBak2[3];
	pointBak1[0] = (renderer->GetWorldPoint())[0];
	pointBak1[1] = (renderer->GetWorldPoint())[1];
	pointBak1[2] = (renderer->GetWorldPoint())[2];

	renderer->SetDisplayPoint(m_pTMJData->pointBak2[0], m_pTMJData->pointBak2[1], 0);
	renderer->DisplayToWorld();
	pointBak2[0] = (renderer->GetWorldPoint())[0];
	pointBak2[1] = (renderer->GetWorldPoint())[1];
	pointBak2[2] = (renderer->GetWorldPoint())[2];

	m_biWidthLineLeftBottomSourceBak->SetPoint1(pointBak1);
	m_biWidthLineLeftBottomSourceBak->SetPoint2(pointBak2);
	m_biWidthLineLeftBottomMapperBak->SetInputConnection(m_biWidthLineLeftBottomSourceBak->GetOutputPort());
	m_biWidthLineLeftBottomActorBak->SetMapper(m_biWidthLineLeftBottomMapperBak);
	m_biWidthLineLeftBottomActorBak->GetProperty()->SetColor(1.0, 1.0, 1.0);

	this->m_view->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->AddActor(m_biWidthLineLeftBottomActorBak);
	renderer->GetRenderWindow()->GetInteractor()->Initialize();
	renderer->GetRenderWindow()->GetInteractor()->Render();
	MsgCenter::send(MsgCenter::TMJShowImageViewer);
}

void AxialWindow::RightTopLineRender()  // right top
{
	vtkSmartPointer<vtkRenderer> renderer = this->m_view->GetRenderer();
	renderer->SetDisplayPoint(m_pTMJData->vMirrorPointBak1[0], m_pTMJData->vMirrorPointBak1[1], 0);
	renderer->DisplayToWorld();
	double vMWorldPointBak1[3];
	double vMWorldPointBak2[3];
	vMWorldPointBak1[0] = (renderer->GetWorldPoint())[0];
	vMWorldPointBak1[1] = (renderer->GetWorldPoint())[1];
	vMWorldPointBak1[2] = (renderer->GetWorldPoint())[2];

	renderer->SetDisplayPoint(m_pTMJData->vMirrorPointBak2[0], m_pTMJData->vMirrorPointBak2[1], 0);
	renderer->DisplayToWorld();
	vMWorldPointBak2[0] = (renderer->GetWorldPoint())[0];
	vMWorldPointBak2[1] = (renderer->GetWorldPoint())[1];
	vMWorldPointBak2[2] = (renderer->GetWorldPoint())[2];

	m_biWidthLineRightTopSourceBak->SetPoint1(vMWorldPointBak1);
	m_biWidthLineRightTopSourceBak->SetPoint2(vMWorldPointBak2);
	m_biWidthLineRightTopMapperBak->SetInputConnection(m_biWidthLineRightTopSourceBak->GetOutputPort());
	m_biWidthLineRightTopActorBak->SetMapper(m_biWidthLineRightTopMapperBak);
	m_biWidthLineRightTopActorBak->GetProperty()->SetColor(1.0, 1.0, 1.0);

	this->m_view->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->AddActor(m_biWidthLineRightTopActorBak);
	renderer->GetRenderWindow()->GetInteractor()->Initialize();
	renderer->GetRenderWindow()->GetInteractor()->Render();
	MsgCenter::send(MsgCenter::TMJShowImageViewer);
}

void AxialWindow::RightBottomLineRender()  // right bottom
{
	vtkSmartPointer<vtkRenderer> renderer = this->m_view->GetRenderer();
	renderer->SetDisplayPoint(m_pTMJData->mirrorPointBak1[0], m_pTMJData->mirrorPointBak1[1], 0);
	renderer->DisplayToWorld();
	double mWorldPointBak1[3];
	mWorldPointBak1[0] = (renderer->GetWorldPoint())[0];
	mWorldPointBak1[1] = (renderer->GetWorldPoint())[1];
	mWorldPointBak1[2] = (renderer->GetWorldPoint())[2];

	renderer->SetDisplayPoint(m_pTMJData->mirrorPointBak2[0], m_pTMJData->mirrorPointBak2[1], 0);
	renderer->DisplayToWorld();
	double mWorldPointBak2[3];
	mWorldPointBak2[0] = (renderer->GetWorldPoint())[0];
	mWorldPointBak2[1] = (renderer->GetWorldPoint())[1];
	mWorldPointBak2[2] = (renderer->GetWorldPoint())[2];

	m_biWidthLineRightBottomSourceBak->SetPoint1(mWorldPointBak1);
	m_biWidthLineRightBottomSourceBak->SetPoint2(mWorldPointBak2);
	m_biWidthLineRightBottomMapperBak->SetInputConnection(m_biWidthLineRightBottomSourceBak->GetOutputPort());
	m_biWidthLineRightBottomActorBak->SetMapper(m_biWidthLineRightBottomMapperBak);
	m_biWidthLineRightBottomActorBak->GetProperty()->SetColor(1.0, 1.0, 1.0);

	this->m_view->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->AddActor(m_biWidthLineRightBottomActorBak);
	renderer->GetRenderWindow()->GetInteractor()->Initialize();
	renderer->GetRenderWindow()->GetInteractor()->Render();
	MsgCenter::send(MsgCenter::TMJShowImageViewer);
}