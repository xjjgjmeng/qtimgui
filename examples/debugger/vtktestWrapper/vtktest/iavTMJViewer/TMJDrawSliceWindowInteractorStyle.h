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
#include "TMJSliceWindow.h"
#include "AxialWindow.h"


class TMJDrawSliceWindowInteractorStyle : public vtkInteractorStyleTrackballCamera
{
public:
	static TMJDrawSliceWindowInteractorStyle* New() { return new TMJDrawSliceWindowInteractorStyle(); };

	void OnMouseWheelForward() override;
	void OnMouseWheelBackward() override;
	void SetSliceWindowClass(TMJSliceWindow* window);
	void SetDataStructSliceWindow(TMJDataStruct* a);

private:
	TMJSliceWindow* m_pTMJSliceWindow;
	TMJDataStruct* m_pTMJData;
	vtkSmartPointer<vtkImageActorPointPlacer> m_imagePointPicker;

	AxialWindow* m_tmjTransverseWindow;
};
