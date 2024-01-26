#pragma once
#include <vtkSmartPointer.h>
#include <vtkRenderer.h>
#include <vtkGenericOpenGLRenderWindow.h> 
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>

#include "CPRDataStruct.h"
#include "MsgCenter.h"

class CPRVolumeRenderingWindow
{
public:

	CPRVolumeRenderingWindow(CPRDataStruct* a);

	vtkRenderWindow* GetRenderWindow();
	void DrawSplineInVRWindow();
	void DrawSplineCurvePlane();
	void DrawSplineTangentPlane();

	void DrawTransSlicePlane(); // yellow
	void loadImage();

private:

	void DrawNerveTube();
	void DrawImplant();
	void UpdateImplantPosition();

	//helpers 
	void TransformPanWinImplantCoordsToVrWin(double panCoords[3], double vrCoords[3]);

private:

	void msgHandler(const MsgCenter::Msg msg, const std::any& arg);

	CPRDataStruct* m_pCprData;

	vtkSmartPointer<vtkRenderer> m_vrRen;
	vtkSmartPointer<vtkGenericOpenGLRenderWindow> m_vrRenW;
	
	vtkSmartPointer<vtkPolyDataMapper> m_splineMapper;
	vtkSmartPointer<vtkPolyDataMapper> m_splineCurvePlaneMapper;
	vtkSmartPointer<vtkPolyDataMapper> m_cprSlicePlaneMapper;
	vtkSmartPointer<vtkPolyDataMapper> m_TransSlicePlaneMapper;

	vtkSmartPointer<vtkPolyData> m_transSlicePlane; //yellow
	vtkSmartPointer<vtkPolyData> m_curvePlane; //red
	vtkSmartPointer<vtkPolyData> m_cprSlicePlane; // blue
	
	vtkSmartPointer<vtkActor> m_splineActor;
	vtkSmartPointer<vtkActor> m_transSlicePlaneActor;
	vtkSmartPointer<vtkActor> m_curvePlaneActor;
	vtkSmartPointer<vtkActor> m_cprSlicePlaneActor;

	vtkSmartPointer<vtkActor> m_imageOutlineActorInVRView;

	vtkSmartPointer<vtkActor> actorNerveTube;
	vtkSmartPointer<vtkActor> actorNerveTube2;

	//implant data 
	vtkSmartPointer<vtkActor> implantActor;

	friend class DrawCPRInteractorStyle;
};

