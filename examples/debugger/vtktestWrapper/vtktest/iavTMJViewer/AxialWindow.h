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
#include "TMJDataStruct.h"
#include "MsgCenter.h"
#include "TMJSliceWindow.h"

//class TMJDrawInteractorStyle;  //类的前向声明

class AxialWindow : public vtkInteractorStyleTrackballCamera
{
public:
	AxialWindow(TMJDataStruct* a);
	vtkRenderWindow* GetRenderWindow();
	// min max cur
	std::tuple<int, int, int> getTransverseSliceIdx() const;
	void SetSlice(const int v);
	void setThickness(const int v);
	void StartBidirectionalDrawing();

	void VerticalLineRenderer();
	void MirrorCross();
	void LineRenderer();
	void MirrorLineRenderer();
	void MirrorVerticalLine();
	vtkSmartPointer<vtkImageAppend> getImageData(const bool isLeft);
	void GetLeftRectangleWorldPoints();
	void GetRightRectangleWorldPoints();
	void TranslateCrosshair();
	void TranslateMirrorCrosshair();
	double GetDistance(double* p2, double* p1);
	void loadImage();

	// mouse wheel->line render
	void LeftTopLineRender();
	void RightTopLineRender();
	void LeftBottomLineRender();
	void RightBottomLineRender();

public:
	vtkSmartPointer<vtkImageViewer2> m_view;

private:
	// Put helper functions here
	void msgHandler(const MsgCenter::Msg msg, const std::any& arg);

private:
	// Put data here
	TMJDataStruct* m_pTMJData;
	//TMJDrawInteractorStyle* drawW;
	TMJSliceWindow* m_tmjSliceWindow;

	//vtkSmartPointer<vtkPolyData> m_biWidthLineLeft;   //left
	//vtkSmartPointer<vtkPolyData> m_biHightLineLeft; 
	//vtkSmartPointer<vtkPolyData> m_biWidthLineRight;  //right
	//vtkSmartPointer<vtkPolyData> m_biHightLineRight;

	vtkSmartPointer<vtkLineSource> m_biWidthLineLeftSource;  //left
    vtkSmartPointer<vtkLineSource> m_biHightLineLeftSource;
	vtkSmartPointer<vtkLineSource> m_biWidthLineRightSource;  //right
	vtkSmartPointer<vtkLineSource> m_biHightLineRightSource;

	vtkSmartPointer<vtkPolyDataMapper> m_biWidthLineLeftMapper; //left
	vtkSmartPointer<vtkPolyDataMapper> m_biHightLineLeftMapper;
	vtkSmartPointer<vtkPolyDataMapper> m_biWidthLineRightMapper; //right
	vtkSmartPointer<vtkPolyDataMapper> m_biHightLineRightMapper;

	vtkSmartPointer<vtkActor> m_biWidthLineLeftActor;  //left
	vtkSmartPointer<vtkActor> m_biHightLineLeftActor;
	vtkSmartPointer<vtkActor> m_biWidthLineRightActor;  //right
	vtkSmartPointer<vtkActor> m_biHightLineRightActor;

	// left top
	vtkSmartPointer<vtkLineSource> m_biWidthLineLeftTopSourceBak;
	vtkSmartPointer<vtkPolyDataMapper> m_biWidthLineLeftTopMapperBak;
	vtkSmartPointer<vtkActor> m_biWidthLineLeftTopActorBak;
	// left bottom
	vtkSmartPointer<vtkLineSource> m_biWidthLineLeftBottomSourceBak;
	vtkSmartPointer<vtkPolyDataMapper> m_biWidthLineLeftBottomMapperBak;
	vtkSmartPointer<vtkActor> m_biWidthLineLeftBottomActorBak;
	// right top
	vtkSmartPointer<vtkLineSource> m_biWidthLineRightTopSourceBak;
	vtkSmartPointer<vtkPolyDataMapper> m_biWidthLineRightTopMapperBak;
	vtkSmartPointer<vtkActor> m_biWidthLineRightTopActorBak;
	// right bottom
	vtkSmartPointer<vtkLineSource> m_biWidthLineRightBottomSourceBak;
	vtkSmartPointer<vtkPolyDataMapper> m_biWidthLineRightBottomMapperBak;
	vtkSmartPointer<vtkActor> m_biWidthLineRightBottomActorBak;

	vtkSmartPointer<vtkPolyDataMapper> splineMapperInTransView;

	//friend class TMJDrawInteractorStyle;

private:
	// line data
	int halfDistanceLine = 80; // distance line=100垂线长度
	int numberOfSlice = 20;
	double vPoint1[2];
	double vPoint2[2];
	double vMirrorPoint1[2]; //mirror
	double vMirrorPoint2[2];

};
