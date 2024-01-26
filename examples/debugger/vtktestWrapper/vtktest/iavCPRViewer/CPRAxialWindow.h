#pragma once
#include <vtkSmartPointer.h>
#include <vtkRenderer.h>
#include <vtkGenericOpenGLRenderWindow.h> 
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkImageActor.h>
#include <vtkImageViewer2.h>
#include <vtkDistanceWidget.h>
#include <vtkResliceImageViewer.h>
#include <vtkResliceImageViewer.h>
#include "CPRDataStruct.h"
#include <MsgCenter.h>

class DrawCPRInteractorStyle;

class CPRAxialWindow
{
public:

	CPRAxialWindow(CPRDataStruct* a);

	vtkRenderWindow* GetRenderWindow();
	// min max cur
	std::tuple<int, int, int> getTransverseSliceIdx() const;
	void SetSlice(const int v);
	void setThickness(const int v);
	void loadImage();

private:
	void DrawSplineOnTransImage();
	void DrawMarkLineOnTransImage();
	void DrawAdjoiningMarkLineOnTransImage();
	void DrawThickSplineOnTransImage();

	void ComputeSplineFrenetArray();
	void msgHandler(const MsgCenter::Msg msg, const std::any& arg);



private:

	CPRDataStruct* m_pCprData;

	vtkSmartPointer<vtkPolyData> m_thickSpline;  //red 
	vtkSmartPointer<vtkPolyData> m_markerLine; //blue
	vtkSmartPointer<vtkPolyData> m_adjoiningMarkerLine; //blue (maybe dashed)

	vtkSmartPointer<vtkPolyDataMapper> m_splinePointsMapper; 
	vtkSmartPointer<vtkPolyDataMapper> m_splineMapper;
	vtkSmartPointer<vtkPolyDataMapper> m_thickSplineMapper;
	vtkSmartPointer<vtkPolyDataMapper> m_markerLineMapper;
	vtkSmartPointer<vtkPolyDataMapper> m_adjoiningMarkerLineMapperUpstream;
	vtkSmartPointer<vtkPolyDataMapper> m_adjoiningMarkerLineMapperDownstream;
	vtkSmartPointer<vtkPolyDataMapper> m_thickSplineUpMapper;
	vtkSmartPointer<vtkPolyDataMapper> m_thickSplineDownMapper;
	vtkSmartPointer<vtkPolyDataMapper> m_closeLineLeftMapper;
	vtkSmartPointer<vtkPolyDataMapper> m_closeLineRightMapper;

	vtkSmartPointer<vtkActor> m_splinePointsActor;
	vtkSmartPointer<vtkActor> m_splineActor;
	vtkSmartPointer<vtkActor> m_thickSplineActor;
	vtkSmartPointer<vtkActor> m_markerLineActor;
	vtkSmartPointer<vtkActor> m_adjoiningMarkerLineActorUpstream;
	vtkSmartPointer<vtkActor> m_adjoiningMarkerLineActorDownstream;
	vtkSmartPointer<vtkActor> m_thickSplineUpActor;
	vtkSmartPointer<vtkActor> m_thickSplineDownActor;
	vtkSmartPointer<vtkActor> m_closeLineLeftActor;
	vtkSmartPointer<vtkActor> m_closeLineRightActor;

	vtkSmartPointer<vtkPolyDataMapper> splineMapperInTransView;

	vtkSmartPointer<vtkImageViewer2> view2;
	vtkSmartPointer<vtkDistanceWidget> m_distanceWidget;
	
	friend class DrawCPRInteractorStyle;
};

