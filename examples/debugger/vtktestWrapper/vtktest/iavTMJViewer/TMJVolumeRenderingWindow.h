#pragma once
#include <vtkSmartPointer.h>
#include <vtkRenderer.h>
#include <vtkGenericOpenGLRenderWindow.h> 
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkImageAppend.h>
#include <vtkMatrix4x4.h>
#include <vtkVolume.h>
#include <vtkClipVolume.h>
#include <vtkPlanes.h>
#include <vtkImageAppend.h>

#include "TMJDataStruct.h"
#include "MsgCenter.h"
#include "TMJDrawInteractorStyle.h"

class TMJVolumeRenderingWindow
{
public:
	TMJVolumeRenderingWindow(TMJDataStruct* a, const bool isLeft);
	vtkRenderWindow* GetRenderWindow();


private: 
	void ShowRenderingVolume();
	void msgHandler(const MsgCenter::Msg msg, const std::any& arg);
	

private:
	TMJDataStruct* m_pTMJData;
	vtkSmartPointer<vtkRenderer> m_vrRen;
	vtkSmartPointer<vtkGenericOpenGLRenderWindow> m_vrRenW;
	//vtkSmartPointer<vtkVolume> m_Volume;
	//vtkSmartPointer<vtkPlanes> clipLeftPlanes;  // box

	bool m_left = false;  //≈–∂œ «∑Ò «left
};
