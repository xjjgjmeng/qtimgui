#pragma once
#include "TMJDrawSliceWindowInteractorStyle.h"
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
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkRendererCollection.h>
#include <vtkPointData.h>
#include <vtkImageActorPointPlacer.h>
#include <vtkTriangle.h>
#include <vtkRegularPolygonSource.h>
#include "TMJDataStruct.h"
#include <iavGlobalData.h>
#include <MsgCenter.h>


 void TMJDrawSliceWindowInteractorStyle::SetDataStructSliceWindow(TMJDataStruct* a)
{
	m_pTMJData = a;
}

void TMJDrawSliceWindowInteractorStyle::OnMouseWheelForward()
{

	m_pTMJData->isMouseWheel = true;
	double spacing[3];
	iavGlobalData::getImageData()->GetSpacing(spacing);

	SlicePosition curPosition = this->m_pTMJSliceWindow->GetCurrentPostion();	// get current position
	if (curPosition.s_isLeft)
	{
		double k = (m_pTMJData->point1[1] - m_pTMJData->point2[1]) / (m_pTMJData->point1[0] - m_pTMJData->point2[0]);
		double v_k = -(1 / k);

		if (curPosition.s_isSagittal)
		{
			m_pTMJData->countOfForwardLeftTop += 1;
			m_pTMJData->isLeftTop = true;
			m_pTMJData->isLeftBottom = false;
			m_pTMJData->isRightTop = false;
			m_pTMJData->isRightBottom = false;
			
			double delta_x = spacing[2] * sqrt(1.0 / (1.0 + k * k));
			double delta_y = k * delta_x;
			m_pTMJData->vPointBak1[0] = m_pTMJData->vPointBak1[0] + delta_x;
			m_pTMJData->vPointBak1[1] = m_pTMJData->vPointBak1[1] + delta_y;
			m_pTMJData->vPointBak2[0] = m_pTMJData->vPointBak2[0] + delta_x;
			m_pTMJData->vPointBak2[1] = m_pTMJData->vPointBak2[1] + delta_y;
			MsgCenter::send(MsgCenter::TMJLeftTopCursorLineChanged);
		}
		else
		{
			m_pTMJData->countOfForwardLeftBottom += 1;
			m_pTMJData->isLeftTop = false;
			m_pTMJData->isLeftBottom = true;
			m_pTMJData->isRightTop = false;
			m_pTMJData->isRightBottom = false;

			double delta_x = spacing[2] * sqrt(1.0 / (1.0 + v_k * v_k)); 
			double delta_y = v_k * delta_x;
			m_pTMJData->pointBak1[0] = m_pTMJData->pointBak1[0] - delta_x;
			m_pTMJData->pointBak1[1] = m_pTMJData->pointBak1[1] - delta_y;
			m_pTMJData->pointBak2[0] = m_pTMJData->pointBak2[0] - delta_x;
			m_pTMJData->pointBak2[1] = m_pTMJData->pointBak2[1] - delta_y;
			MsgCenter::send(MsgCenter::TMJLeftBottomCursorLineChanged);
		}
	}
	else
	{
		if (curPosition.s_isSagittal)
		{
			m_pTMJData->countOfForwardRightTop += 1;
			m_pTMJData->isLeftTop = false;
			m_pTMJData->isLeftBottom = false;
			m_pTMJData->isRightTop = true;
			m_pTMJData->isRightBottom = false;

			double k = (m_pTMJData->mirrorPoint1[1] - m_pTMJData->mirrorPoint2[1]) / (m_pTMJData->mirrorPoint1[0] - m_pTMJData->mirrorPoint2[0]);
			double delta_x = spacing[2] * sqrt(1.0 / (1.0 + k * k));  
			double delta_y = k * delta_x;

			m_pTMJData->vMirrorPointBak1[0] = m_pTMJData->vMirrorPointBak1[0] - delta_x;
			m_pTMJData->vMirrorPointBak1[1] = m_pTMJData->vMirrorPointBak1[1] - delta_y;
			m_pTMJData->vMirrorPointBak2[0] = m_pTMJData->vMirrorPointBak2[0] - delta_x;
			m_pTMJData->vMirrorPointBak2[1] = m_pTMJData->vMirrorPointBak2[1] - delta_y;
			MsgCenter::send(MsgCenter::TMJRightTopCursorLineChanged);
		}
		else
		{
			m_pTMJData->countOfForwardRightBottom += 1;
			m_pTMJData->isLeftTop = false;
			m_pTMJData->isLeftBottom = false;
			m_pTMJData->isRightTop = false;
			m_pTMJData->isRightBottom = true;

			double k = (m_pTMJData->mirrorPoint1[1] - m_pTMJData->mirrorPoint2[1]) / (m_pTMJData->mirrorPoint1[0] - m_pTMJData->mirrorPoint2[0]);
			double v_k = -(1 / k);
			double delta_x = spacing[2] * sqrt(1.0 / (1.0 + v_k * v_k)); 
			double delta_y = v_k * delta_x;

			m_pTMJData->mirrorPointBak1[0] = m_pTMJData->mirrorPointBak1[0] + delta_x;
			m_pTMJData->mirrorPointBak1[1] = m_pTMJData->mirrorPointBak1[1] + delta_y;
			m_pTMJData->mirrorPointBak2[0] = m_pTMJData->mirrorPointBak2[0] + delta_x;
			m_pTMJData->mirrorPointBak2[1] = m_pTMJData->mirrorPointBak2[1] + delta_y;
			std::cout << "bak mirror Point1: " << m_pTMJData->mirrorPointBak1[0] << " " << m_pTMJData->mirrorPointBak1[1] << endl;
			std::cout << "bak mirror Point2: " << m_pTMJData->mirrorPointBak2[0] << " " << m_pTMJData->mirrorPointBak2[1] << endl;
			
			MsgCenter::send(MsgCenter::TMJRightBottomCursorLineChanged);
		}
	}
}

void TMJDrawSliceWindowInteractorStyle::OnMouseWheelBackward()
{

	m_pTMJData->isMouseWheel = true;
	double spacing[3];
	iavGlobalData::getImageData()->GetSpacing(spacing);

	SlicePosition curPosition = this->m_pTMJSliceWindow->GetCurrentPostion();	// get current position
	if (curPosition.s_isLeft)
	{
		if (curPosition.s_isSagittal)
		{
			m_pTMJData->countOfBackLeftTop += 1;
			m_pTMJData->isLeftTop = true;
			m_pTMJData->isLeftBottom = false;
			m_pTMJData->isRightTop = false;
			m_pTMJData->isRightBottom = false;

			double k = (m_pTMJData->point1[1] - m_pTMJData->point2[1]) / (m_pTMJData->point1[0] - m_pTMJData->point2[0]);
			double delta_x = spacing[2] * sqrt(1.0 / (1.0 + k * k));  //spacing[2]=0.2
			double delta_y = k * delta_x;

			m_pTMJData->vPointBak1[0] = m_pTMJData->vPointBak1[0] - delta_x;
			m_pTMJData->vPointBak1[1] = m_pTMJData->vPointBak1[1] - delta_y;
			m_pTMJData->vPointBak2[0] = m_pTMJData->vPointBak2[0] - delta_x;
			m_pTMJData->vPointBak2[1] = m_pTMJData->vPointBak2[1] - delta_y;
			MsgCenter::send(MsgCenter::TMJLeftTopCursorLineChanged);
		}
		else
		{
			m_pTMJData->countOfBackLeftBottom += 1;
			m_pTMJData->isLeftTop = false;
			m_pTMJData->isLeftBottom =true;
			m_pTMJData->isRightTop = false;
			m_pTMJData->isRightBottom = false;

			double k = (m_pTMJData->point1[1] - m_pTMJData->point2[1]) / (m_pTMJData->point1[0] - m_pTMJData->point2[0]);
			double v_k = -(1 / k);
			double delta_x = spacing[2] * sqrt(1.0 / (1.0 + v_k * v_k));
			double delta_y = v_k * delta_x;
			m_pTMJData->pointBak1[0] = m_pTMJData->pointBak1[0] + delta_x;
			m_pTMJData->pointBak1[1] = m_pTMJData->pointBak1[1] + delta_y;
			m_pTMJData->pointBak2[0] = m_pTMJData->pointBak2[0] + delta_x;
			m_pTMJData->pointBak2[1] = m_pTMJData->pointBak2[1] + delta_y;
			MsgCenter::send(MsgCenter::TMJLeftBottomCursorLineChanged);
		}
	}
	else
	{
		if (curPosition.s_isSagittal)
		{
			m_pTMJData->countOfBackRightTop += 1;
			m_pTMJData->isLeftTop = false;
			m_pTMJData->isLeftBottom = false;
			m_pTMJData->isRightTop = true;
			m_pTMJData->isRightBottom = false;

			double k = (m_pTMJData->mirrorPoint1[1] - m_pTMJData->mirrorPoint2[1]) / (m_pTMJData->mirrorPoint1[0] - m_pTMJData->mirrorPoint2[0]);
			double delta_x = spacing[2] * sqrt(1.0 / (1.0 + k * k));  //spacing[2]=0.2
			double delta_y = k * delta_x;

			m_pTMJData->vMirrorPointBak1[0] = m_pTMJData->vMirrorPointBak1[0] + delta_x;
			m_pTMJData->vMirrorPointBak1[1] = m_pTMJData->vMirrorPointBak1[1] + delta_y;
			m_pTMJData->vMirrorPointBak2[0] = m_pTMJData->vMirrorPointBak2[0] + delta_x;
			m_pTMJData->vMirrorPointBak2[1] = m_pTMJData->vMirrorPointBak2[1] + delta_y;
			MsgCenter::send(MsgCenter::TMJRightTopCursorLineChanged);
		}
		else
		{
			m_pTMJData->countOfBackRightBottom += 1;
			m_pTMJData->isLeftTop = false;
			m_pTMJData->isLeftBottom = false;
			m_pTMJData->isRightTop = false;
			m_pTMJData->isRightBottom = true;

			double k = (m_pTMJData->mirrorPoint1[1] - m_pTMJData->mirrorPoint2[1]) / (m_pTMJData->mirrorPoint1[0] - m_pTMJData->mirrorPoint2[0]);
			double v_k = -(1 / k);
			double delta_x = spacing[2] * sqrt(1.0 / (1.0 + v_k * v_k));  //spacing[2]=0.2
			double delta_y = v_k * delta_x;

			m_pTMJData->mirrorPointBak1[0] = m_pTMJData->mirrorPointBak1[0] - delta_x;
			m_pTMJData->mirrorPointBak1[1] = m_pTMJData->mirrorPointBak1[1] - delta_y;
			m_pTMJData->mirrorPointBak2[0] = m_pTMJData->mirrorPointBak2[0] - delta_x;
			m_pTMJData->mirrorPointBak2[1] = m_pTMJData->mirrorPointBak2[1] - delta_y;
			std::cout << "bak mirror Point1: " << m_pTMJData->mirrorPointBak1[0] << " " << m_pTMJData->mirrorPointBak1[1] << endl;
			std::cout << "bak mirror Point2: " << m_pTMJData->mirrorPointBak2[0] << " " << m_pTMJData->mirrorPointBak2[1] << endl;

			MsgCenter::send(MsgCenter::TMJRightBottomCursorLineChanged);
		}
	}
}

void TMJDrawSliceWindowInteractorStyle::SetSliceWindowClass(TMJSliceWindow* window)
{
	m_pTMJSliceWindow = window;

	//m_imagePointPicker->SetImageActor(window->GetResliceImageViewer()->GetImageActor());
	//m_imagePointPicker->Modified();
}
