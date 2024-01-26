#pragma once
#include <vtkSmartPointer.h>
#include <vtkRenderer.h>
#include <vtkGenericOpenGLRenderWindow.h> 
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkImageActor.h>
#include <vtkImageViewer2.h>
#include <vtkResliceImageViewer.h>
#include <vtkResliceImageViewer.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkImageAppend.h>
#include <vtkLineSource.h>
#include <vtkImageActorPointPlacer.h>
#include "TMJVolumeRenderingWindow.h"
#include "AxialWindow.h"
#include "TMJDataStruct.h"


class TMJDrawInteractorStyle : public vtkInteractorStyleTrackballCamera
{
public:
	static TMJDrawInteractorStyle* New() { return new TMJDrawInteractorStyle(); };

	void OnLeftButtonDown()override;
	void OnLeftButtonUp()override;
	void OnMouseMove() override;

	void SetTransWindowClass(AxialWindow* window);
	void SetDataStruct(TMJDataStruct* a);

private:
	TMJDataStruct* m_pTMJData;
	AxialWindow* m_tmjTransverseWindow;

	vtkSmartPointer<vtkImageActorPointPlacer> m_tmjImagePointPicker;
	//vtkSmartPointer<vtkPolyDataMapper> m_tmjStrightLineMapper;
	//vtkSmartPointer<vtkActor> m_tmjStrightLineActor;

	int* clickPos;
	const int threshold = 3; // 鼠标点到端点之间的距离
	bool isRotateLine = false;
	bool isRotateMirrorP1 = false;
	bool isRotateMirrorP2 = false;
	bool isLeftButtonUp = false;
	bool isLeftButtonDown = false;
	bool isTranslateCursor = false;  // translate crosshair
	bool isMirrorTranslateCursor = false;

public:
	//bool isLeftButtonUp = false;

};
