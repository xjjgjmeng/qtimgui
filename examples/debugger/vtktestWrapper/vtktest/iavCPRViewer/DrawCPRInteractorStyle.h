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

#include "CPRAxialWindow.h"
#include "CPRVolumeRenderingWindow.h"

class DrawCPRInteractorStyle : public vtkInteractorStyleTrackballCamera
{
public:
	static DrawCPRInteractorStyle* New() { return new DrawCPRInteractorStyle(); };
	DrawCPRInteractorStyle();
	void OnLeftButtonDown() override;
	void OnMouseMove() override;
	void OnMouseWheelForward() override;
	void OnMouseWheelBackward() override;

	void SetTransWindowClass(CPRAxialWindow* window);
	void SetVolumeRenderingWindowClass(CPRVolumeRenderingWindow* window);

private:
	CPRAxialWindow* m_transverseWindow;
	CPRVolumeRenderingWindow* m_VolumeRenderingWindow;

	vtkSmartPointer<vtkImageActorPointPlacer> m_imagePointPicker;

	vtkSmartPointer<vtkPolyDataMapper> m_straightLineMapper;
	vtkSmartPointer<vtkActor> m_straightLineActor;

	bool m_IsDrawing = false;
	double m_splineResamplingDistance = 0.3;
	int ResetPixelDistance;
};