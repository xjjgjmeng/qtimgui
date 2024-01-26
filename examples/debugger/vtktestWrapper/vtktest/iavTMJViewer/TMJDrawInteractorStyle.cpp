#pragma once
#include "TMJDrawInteractorStyle.h"
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
#include "TMJDataStruct.h"

//#include "vtkParallelTransportFrame.h"

void TMJDrawInteractorStyle::SetDataStruct(TMJDataStruct* a)
{
	m_pTMJData = a;
}

void TMJDrawInteractorStyle::OnLeftButtonDown()
{
	std::cout << "Pressed left mouse down." << std::endl;
	isLeftButtonUp = false;
	isLeftButtonDown = true;
	m_pTMJData->isClip = true;

	if (!isRotateLine)
	{
		clickPos = this->GetInteractor()->GetEventPosition();
		m_pTMJData->point1[0] = clickPos[0];
		m_pTMJData->point1[1] = clickPos[1];

		m_pTMJData->pointBak1[0] = clickPos[0];  // mouse wheel
		m_pTMJData->pointBak1[1] = clickPos[1];
		//std::cout << "point1: " << m_pTMJData->point1[0] << " " << m_pTMJData->point1[1] << std::endl;

		vtkSmartPointer<vtkRenderer> renderer = this->m_tmjTransverseWindow->m_view->GetRenderer();
		renderer->SetDisplayPoint(m_pTMJData->point1[0], m_pTMJData->point1[1], 0);
		renderer->DisplayToWorld();
		m_pTMJData->WorldPoint1[0] = (renderer->GetWorldPoint())[0];
		m_pTMJData->WorldPoint1[1] = (renderer->GetWorldPoint())[1];
		m_pTMJData->WorldPoint1[2] = (renderer->GetWorldPoint())[2];
	}
}

void TMJDrawInteractorStyle::OnLeftButtonUp()
{
	std::cout << "Pressed left mouse up." << std::endl;
	isLeftButtonUp = true;
	isLeftButtonDown = false;

	if (!isRotateLine)
	{
		// isInImage
		double display[2];
		double pickedImagePos[3];
		double orient[9];

		display[0] = m_pTMJData->point2[0];
		display[1] = m_pTMJData->point2[1];
		m_tmjImagePointPicker->ComputeWorldPosition(m_tmjTransverseWindow->m_view->GetRenderer(), display, pickedImagePos, orient);
		//std::cout << "Picked value (image world): " << pickedImagePos[0] << " " << pickedImagePos[1] << " " << pickedImagePos[2] << std::endl;

		int isInImage = m_tmjImagePointPicker->ValidateWorldPosition(pickedImagePos);
		if (isInImage == 0)
		{
			std::cout << "current position is not in the image!" << std::endl;
		}
		else if (isInImage == 1)
		{
			m_tmjTransverseWindow->VerticalLineRenderer();
			m_tmjTransverseWindow->MirrorCross();
		}
	}
	MsgCenter::send(MsgCenter::TMJVolumeClipRequest);
}

void TMJDrawInteractorStyle::OnMouseMove()
{
	if (clickPos == nullptr) return;

	int* movePos = this->GetInteractor()->GetEventPosition();

	if (isLeftButtonDown && !isRotateLine)
	{
		int* movePos = this->GetInteractor()->GetEventPosition();
		m_pTMJData->point2[0] = movePos[0];
		m_pTMJData->point2[1] = movePos[1];

		m_pTMJData->pointBak2[0] = movePos[0];  // mouse wheel
		m_pTMJData->pointBak2[1] = movePos[1];
		//std::cout << "move Point2: " << movePos[0] << " " << movePos[1] << std::endl;

		vtkSmartPointer<vtkRenderer> renderer = this->m_tmjTransverseWindow->m_view->GetRenderer();
		renderer->SetDisplayPoint(m_pTMJData->point2[0], m_pTMJData->point2[1], 0);
		renderer->DisplayToWorld();
		m_pTMJData->WorldPoint2[0] = (renderer->GetWorldPoint())[0];
		m_pTMJData->WorldPoint2[1] = (renderer->GetWorldPoint())[1];
		m_pTMJData->WorldPoint2[2] = (renderer->GetWorldPoint())[2];
		m_tmjTransverseWindow->LineRenderer();
	}

	if (isLeftButtonUp)  
	{
		int* currPos = this->GetInteractor()->GetEventPosition();
		double currPoint[2];
		currPoint[0] = currPos[0];
		currPoint[1] = currPos[1];
		//std::cout << "current mouse position: " << currPoint[0] << " " << currPoint[1] << std::endl;

		// current world position
		vtkSmartPointer<vtkRenderer> renderer = this->m_tmjTransverseWindow->m_view->GetRenderer();
		renderer->SetDisplayPoint(currPoint[0], currPoint[1], 0);
		renderer->DisplayToWorld();
		double currWorldPoint[3];
		currWorldPoint[0] = (renderer->GetWorldPoint())[0];
		currWorldPoint[1] = (renderer->GetWorldPoint())[1];
		currWorldPoint[2] = (renderer->GetWorldPoint())[2];

		// mid-point world position
		m_pTMJData->mid_point[0] = (m_pTMJData->point1[0] + m_pTMJData->point2[0]) / 2;
		m_pTMJData->mid_point[1] = (m_pTMJData->point1[1] + m_pTMJData->point2[1]) / 2;
		renderer->SetDisplayPoint(m_pTMJData->mid_point[0], m_pTMJData->mid_point[1], 0);
		renderer->DisplayToWorld();
		double midWorldPoint[3];
		midWorldPoint[0] = (renderer->GetWorldPoint())[0];
		midWorldPoint[1] = (renderer->GetWorldPoint())[1];
		midWorldPoint[2] = (renderer->GetWorldPoint())[2];

		// mirror mid-point world position
		m_pTMJData->mid_mpoint[0] = (m_pTMJData->mirrorPoint1[0] + m_pTMJData->mirrorPoint2[0]) / 2;
		m_pTMJData->mid_mpoint[1] = (m_pTMJData->mirrorPoint1[1] + m_pTMJData->mirrorPoint2[1]) / 2;
		renderer->SetDisplayPoint(m_pTMJData->mid_mpoint[0], m_pTMJData->mid_mpoint[1], 0);
		renderer->DisplayToWorld();
		double midMWorldPoint[3];
		midMWorldPoint[0] = (renderer->GetWorldPoint())[0];
		midMWorldPoint[1] = (renderer->GetWorldPoint())[1];
		midMWorldPoint[2] = (renderer->GetWorldPoint())[2];

		double distancePoint1 = vtkMath::Distance2BetweenPoints(currWorldPoint, m_pTMJData->WorldPoint1);
		double distancePoint2 = vtkMath::Distance2BetweenPoints(currWorldPoint, m_pTMJData->WorldPoint2);
		double distanceMWorldPoint1 = vtkMath::Distance2BetweenPoints(currWorldPoint, m_pTMJData->MWorldPoint1);
		double distanceMWorldPoint2 = vtkMath::Distance2BetweenPoints(currWorldPoint, m_pTMJData->MWorldPoint2);
		double distanceMidPoint = vtkMath::Distance2BetweenPoints(currWorldPoint, midWorldPoint);
		double distanceMidMirrorPoint = vtkMath::Distance2BetweenPoints(currWorldPoint, midMWorldPoint);

		// line
		if (distancePoint2 < threshold)
		{
			renderer->GetRenderWindow()->SetCurrentCursor(VTK_CURSOR_HAND);
			isRotateLine = true;
			m_pTMJData->isRotateP2 = true;
			m_pTMJData->isRotateP1 = false;
			isRotateMirrorP1 = false;
			isRotateMirrorP2 = false;
			isTranslateCursor = false;
			isMirrorTranslateCursor = false;
		}
		else if (distancePoint1 < threshold)
		{
			renderer->GetRenderWindow()->SetCurrentCursor(VTK_CURSOR_HAND);
			isRotateLine = true;
			m_pTMJData->isRotateP1 = true;
			m_pTMJData->isRotateP2 = false;
			isRotateMirrorP1 = false;
			isRotateMirrorP2 = false;
			isTranslateCursor = false;
			isMirrorTranslateCursor = false;
		}
		else if (distanceMWorldPoint1 < threshold)
		{
			renderer->GetRenderWindow()->SetCurrentCursor(VTK_CURSOR_HAND);
			isRotateLine = true;
			isRotateMirrorP1 = true;
			isRotateMirrorP2 = false;
			m_pTMJData->isRotateP2 = false;
			m_pTMJData->isRotateP1 = false;
			isTranslateCursor = false;
			isMirrorTranslateCursor = false;
		}
		else if (distanceMWorldPoint2 < threshold)
		{
			renderer->GetRenderWindow()->SetCurrentCursor(VTK_CURSOR_HAND);
			isRotateLine = true;
			isRotateMirrorP2 = true;
			isRotateMirrorP1 = false;
			m_pTMJData->isRotateP2 = false;
			m_pTMJData->isRotateP1 = false;
			isTranslateCursor = false;
			isMirrorTranslateCursor = false;
		}
		else if (distanceMidPoint < threshold)
		{
			renderer->GetRenderWindow()->SetCurrentCursor(VTK_CURSOR_CROSSHAIR);
			isRotateLine = true;
			isTranslateCursor = true;
			isRotateMirrorP2 = false;
			isRotateMirrorP1 = false;
			m_pTMJData->isRotateP2 = false;
			m_pTMJData->isRotateP1 = false;
			isMirrorTranslateCursor = false;
		}
		else if (distanceMidMirrorPoint < threshold)
		{
			renderer->GetRenderWindow()->SetCurrentCursor(VTK_CURSOR_CROSSHAIR);
			isRotateLine = true;
			isMirrorTranslateCursor = true;
			isTranslateCursor = false;
			isRotateMirrorP2 = false;
			isRotateMirrorP1 = false;
			m_pTMJData->isRotateP2 = false;
			m_pTMJData->isRotateP1 = false;
		}
		else
		{
			renderer->GetRenderWindow()->SetCurrentCursor(VTK_CURSOR_DEFAULT);
		}
	}

	if (isLeftButtonDown && m_pTMJData->isRotateP1)  // rotate p1
	{
		m_pTMJData->point1[0] = movePos[0];
		m_pTMJData->point1[1] = movePos[1];

		// 直线旋转后，坐标点改变
		m_pTMJData->pointBak1[0] = movePos[0];  // mouse wheel
		m_pTMJData->pointBak1[1] = movePos[1];
		//std::cout << "Point1: " << movePos[0] << " " << movePos[1] << std::endl;

		vtkSmartPointer<vtkRenderer> renderer = this->m_tmjTransverseWindow->m_view->GetRenderer();
		renderer->SetDisplayPoint(m_pTMJData->point1[0], m_pTMJData->point1[1], 0);
		renderer->DisplayToWorld();
		m_pTMJData->WorldPoint1[0] = (renderer->GetWorldPoint())[0];
		m_pTMJData->WorldPoint1[1] = (renderer->GetWorldPoint())[1];
		m_pTMJData->WorldPoint1[2] = (renderer->GetWorldPoint())[2];

		m_tmjTransverseWindow->LineRenderer();
		m_tmjTransverseWindow->VerticalLineRenderer();
	}

	if (isLeftButtonDown && m_pTMJData->isRotateP2) // rotate p2
	{
		m_pTMJData->point2[0] = movePos[0];
		m_pTMJData->point2[1] = movePos[1];

		// 直线旋转后，坐标点改变
		m_pTMJData->pointBak2[0] = movePos[0];  // mouse wheel
		m_pTMJData->pointBak2[1] = movePos[1];

		vtkSmartPointer<vtkRenderer> renderer = this->m_tmjTransverseWindow->m_view->GetRenderer();
		renderer->SetDisplayPoint(m_pTMJData->point2[0], m_pTMJData->point2[1], 0);
		renderer->DisplayToWorld();
		m_pTMJData->WorldPoint2[0] = (renderer->GetWorldPoint())[0];
		m_pTMJData->WorldPoint2[1] = (renderer->GetWorldPoint())[1];
		m_pTMJData->WorldPoint2[2] = (renderer->GetWorldPoint())[2];

		m_tmjTransverseWindow->LineRenderer();
		m_tmjTransverseWindow->VerticalLineRenderer();
	}

	if (isLeftButtonDown && isRotateMirrorP1)  // rotate mirror p1
	{
		m_pTMJData->mirrorPoint1[0] = movePos[0];
		m_pTMJData->mirrorPoint1[1] = movePos[1];

		// 直线旋转后，坐标点改变
		m_pTMJData->mirrorPointBak1[0] = movePos[0];
		m_pTMJData->mirrorPointBak1[1] = movePos[1];

		vtkSmartPointer<vtkRenderer> renderer = this->m_tmjTransverseWindow->m_view->GetRenderer();
		renderer->SetDisplayPoint(m_pTMJData->mirrorPoint1[0], m_pTMJData->mirrorPoint1[1], 0);
		renderer->DisplayToWorld();
		m_pTMJData->MWorldPoint1[0] = (renderer->GetWorldPoint())[0];
		m_pTMJData->MWorldPoint1[1] = (renderer->GetWorldPoint())[1];
		m_pTMJData->MWorldPoint1[2] = (renderer->GetWorldPoint())[2];

		m_tmjTransverseWindow->MirrorLineRenderer();
		m_tmjTransverseWindow->MirrorVerticalLine();
	}

	if (isLeftButtonDown && isRotateMirrorP2)  // rotate mirror p2
	{
		m_pTMJData->mirrorPoint2[0] = movePos[0];
		m_pTMJData->mirrorPoint2[1] = movePos[1];

		// 直线旋转后，坐标点改变
		m_pTMJData->mirrorPointBak2[0] = movePos[0];
		m_pTMJData->mirrorPointBak2[1] = movePos[1];

		vtkSmartPointer<vtkRenderer> renderer = this->m_tmjTransverseWindow->m_view->GetRenderer();
		renderer->SetDisplayPoint(m_pTMJData->mirrorPoint2[0], m_pTMJData->mirrorPoint2[1], 0);
		renderer->DisplayToWorld();
		m_pTMJData->MWorldPoint2[0] = (renderer->GetWorldPoint())[0];
		m_pTMJData->MWorldPoint2[1] = (renderer->GetWorldPoint())[1];
		m_pTMJData->MWorldPoint2[2] = (renderer->GetWorldPoint())[2];

		m_tmjTransverseWindow->MirrorLineRenderer();
		m_tmjTransverseWindow->MirrorVerticalLine();
	}

	if (isLeftButtonDown && isTranslateCursor)  // translate crosshair
	{
		m_pTMJData->mid_point[0] = movePos[0];
		m_pTMJData->mid_point[1] = movePos[1];
		m_tmjTransverseWindow->TranslateCrosshair();
	}

	if (isLeftButtonDown && isMirrorTranslateCursor)  // translate mirror crosshair
	{
		m_pTMJData->mid_mpoint[0] = movePos[0];
		m_pTMJData->mid_mpoint[1] = movePos[1];
		m_tmjTransverseWindow->TranslateMirrorCrosshair();
	}
}

void TMJDrawInteractorStyle::SetTransWindowClass(AxialWindow* window)
{
	m_tmjTransverseWindow = window;
	m_tmjImagePointPicker = vtkSmartPointer<vtkImageActorPointPlacer>::New();
	m_tmjImagePointPicker->SetImageActor(this->m_tmjTransverseWindow->m_view->GetImageActor());
}
