#pragma once
#include <iostream>
#include <vtkSmartPointer.h>
#include <vtkPoints.h>
#include <vtkRenderer.h>
#include <vtkPointSource.h>
#include <vtkRenderWindow.h>

#include <vtkImageActor.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkSplineWidget.h>
#include <vtkDataSetMapper.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>

#include <vtkDoubleArray.h>
#include <vtkImageAppend.h>

#include <vtkImageActorPointPlacer.h>
#include <vtkFocalPlanePointPlacer.h>

#include "CPRAxialWindow.h"
#include "CPRPanWindow.h"
#include "CPRVolumeRenderingWindow.h"

class DrawNerveInteractorStyle : public vtkInteractorStyleTrackballCamera
{
public:
	static DrawNerveInteractorStyle* New() { return new DrawNerveInteractorStyle(); };
	DrawNerveInteractorStyle();
	void OnLeftButtonDown() override;
	void OnMouseMove() override;
	void OnRightButtonUp() override;
	void OnMouseWheelForward() override;
	void OnMouseWheelBackward() override;

	void SetVolumeRenderingWindowClass(CPRVolumeRenderingWindow* window);
	void SetPanWindowClass(CPRPanWindow* window);

private: 
	//helper functions 
	void TransformCoordsPanToVR(double panCoords[3], double vrCoords[3]);
private:
	CPRAxialWindow* m_transverseWindow;
	CPRPanWindow* m_panWindow;
	CPRVolumeRenderingWindow* m_VolumeRenderingWindow;

	vtkSmartPointer<vtkImageActorPointPlacer> m_imagePointPicker;
	vtkSmartPointer<vtkFocalPlanePointPlacer> m_FocalPlanePointPlacer;

	vtkSmartPointer<vtkPolyDataMapper> m_straightLineMapper;
	vtkSmartPointer<vtkActor> m_straightLineActor;

	bool m_IsDrawing = false;
	double m_splineResamplingDistance = 0.3;
	unsigned int NumberOfClicks;
	int PreviousPosition[2];
	int ResetPixelDistance;

	int m_currentPointID = 0;
	int m_lastClickedSplinePtID = 0; // for update Nerve in time

};

