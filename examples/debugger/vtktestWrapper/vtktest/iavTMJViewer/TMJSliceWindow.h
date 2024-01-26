#pragma once
#include <vtkSmartPointer.h>
#include <vtkRenderer.h>
#include <vtkGenericOpenGLRenderWindow.h> 
#include <vtkImageData.h>
#include <vtkImageAppend.h>
#include <vtkImageViewer2.h>
#include "TMJDataStruct.h"
#include "MsgCenter.h"

class TMJSliceWindow
{
public:
	TMJSliceWindow(TMJDataStruct* a, int idx, const bool isSagittal, const bool isLeft);
	vtkRenderWindow* GetRenderWindow();
	void ComputeImagesViewerTMJ();
	vtkImageViewer2* GetResliceImageViewer();
	void SetSlice(const int v);
	SlicePosition GetCurrentPostion();

private:
	int m_idxSliceWindow;
	void msgHandler(const MsgCenter::Msg msg, const std::any& arg);

private:
	TMJDataStruct* m_pTMJData;
	vtkNew<vtkImageViewer2> viewData;

	bool m_sagittal = false; // ¥Àslice «∑Ò «sagittal
	bool m_left = false;
	int lengthOfSlice = 2;  // 2mm
};