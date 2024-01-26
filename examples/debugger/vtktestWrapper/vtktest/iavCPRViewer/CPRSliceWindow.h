#pragma once
#include <vtkSmartPointer.h>
#include <vtkRenderer.h>
#include <vtkGenericOpenGLRenderWindow.h> 
#include <vtkImageData.h>
#include <vtkDistanceWidget.h>
#include <vtkCornerAnnotation.h>
#include <vtkImageResliceMapper.h>
#include <vtkImageActor.h>
#include <vtkPlane.h>
#include <vtkTubeFilter.h>

#include "CPRDataStruct.h"
#include "MsgCenter.h"

#define M_PI 3.14159265358979323846

class CPRSliceWindow
{
public:
	CPRSliceWindow(CPRDataStruct* a, int idx);
	vtkRenderWindow* GetRenderWindow();

	void ComputeSliceImagesCPR();
	void ShowSliceImages();

	// use reslice mapper;
	void ShowNerveTube();
	void UpdateNerveTubeIntersection();

	void ShowImplantIntersection();
	void UpdateImplantIntersection();

	void UpdateCornerDistanceText();

private:
	int m_idxSliceWindow;
	double reslicePlaneNormal[3] = { 0 , 0, 1};
	double reslicePlaneOrigin[3] = { 0, 0, 0};
	double newCenter[3] = { 0, 0, 0 };

	double rotatedNormal[3] = { 0, 0, 0 };

	void msgHandler(const MsgCenter::Msg msg, const std::any& arg);

private:
	CPRDataStruct* m_pCprData;

	vtkSmartPointer<vtkRenderer>  m_cprSliceRen;
	vtkSmartPointer<vtkGenericOpenGLRenderWindow> m_cprSliceRenW;

	vtkSmartPointer<vtkImageData> m_CprSliceImgData;
	vtkSmartPointer<vtkImageData> m_CprSliceImgDataRotate;
	vtkSmartPointer<vtkImageSlice> m_CprSliceData;
	vtkSmartPointer<vtkImageSlice> m_CprSliceDataRotate;
	vtkSmartPointer<vtkDistanceWidget> m_distanceWidget;

	vtkSmartPointer<vtkCornerAnnotation> leftAnnotation;
	vtkSmartPointer<vtkCornerAnnotation> rightAnnotation;
	vtkSmartPointer<vtkCornerAnnotation> leftUpAnnotation;

	vtkSmartPointer<vtkImageResliceMapper> m_resliceMapper;
	vtkSmartPointer<vtkPlane> m_reslicePlane;
	vtkSmartPointer<vtkImageActor> m_ImgActor;

	vtkSmartPointer<vtkTubeFilter> Tube1;
	vtkSmartPointer<vtkTubeFilter> Tube2;
	vtkSmartPointer<vtkActor> actorNerveTube;
	vtkSmartPointer<vtkActor> actorNerveTube2;
	vtkSmartPointer<vtkActor2D> actorNerveTubeIntersection;

	vtkSmartPointer<vtkActor2D> actorImplantIntersection;
	vtkSmartPointer<vtkActor> actorImplantCtrlineInVrW;

	vtkSmartPointer<vtkTextProperty> textProperty;
};

