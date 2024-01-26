#pragma once
#include <vtkSmartPointer.h>
#include <vtkRenderer.h>
#include <vtkGenericOpenGLRenderWindow.h> 
#include <vtkImageData.h>
#include <vtkResliceImageViewer.h>
#include <vtkImageViewer2.h>
#include <vtkDistanceWidget.h>
#include <vtkImageResliceMapper.h>
#include <vtkImageSlab.h>
#include <vtkPolyDataMapper2D.h>

#include "CPRDataStruct.h"
#include "InteractorStyleCpr.h"
#include <MsgCenter.h>

class CPRPanWindow
{
public:
	CPRPanWindow(CPRDataStruct* a);
	void SetCPRDataStruct(CPRDataStruct* pCprData) { m_pCprData = pCprData; };
	void setThickness(const int v);
	vtkRenderWindow* GetRenderWindow();
	vtkImageViewer2* GetResliceImageViewer();
	CPRDataStruct* GetCPRData() { return m_pCprData; };
	void SetSlice(const int v);

private:
	void ComputeStraightVolumeCPR();
	void ShowNerveTube();
	void ShowStraightImage();

	void ShowCursorLine();
	void UpdateCursorLineTranslation();
	void UpdataCursorLineRotate();

	void ShowImplant();
	void UpdateImplantPosition();

	void msgHandler(const MsgCenter::Msg msg, const std::any& arg);

private:
	CPRDataStruct* m_pCprData;
	vtkNew<vtkImageViewer2> m_viewer;
	vtkSmartPointer<vtkImageData> m_straightVoumeCPR;
	vtkSmartPointer<vtkDistanceWidget> m_distanceWidget;
	vtkSmartPointer<vtkActor> actorNerveTube;
	vtkSmartPointer<vtkActor> actorNerveTube2;

	vtkSmartPointer<vtkRenderer> m_panRen;
	vtkSmartPointer<vtkImageResliceMapper> m_resliceMapper;

	// blue cursor line data
	vtkSmartPointer<vtkPolyData> cursorPolyData;
	vtkSmartPointer<vtkActor2D> cursorActor;
	vtkSmartPointer<vtkPolyDataMapper2D> cursorMapper;

	//normalInteractorStyle
	vtkSmartPointer<InteractorStyleCpr> normalStyle;

	//implant related data 
	vtkSmartPointer<vtkActor2D> m_implantActor;
	vtkSmartPointer<vtkActor> implantCenterLine;

};

