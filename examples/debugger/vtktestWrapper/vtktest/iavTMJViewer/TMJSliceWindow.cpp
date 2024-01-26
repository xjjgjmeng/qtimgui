#include <vtkCamera.h>
#include <vtkMatrix4x4.h>
#include <vtkImageChangeInformation.h>
#include <vtkImageFlip.h>
#include <vtkImageReslice.h>
#include <vtkImageSliceMapper.h>
#include <vtkImageProperty.h>
#include <vtkRendererCollection.h>
#include <vtkRenderWindowInteractor.h>
//#include <vtkImageMapToWindowLevelColors.h>
#include <vtkXMLImageDataWriter.h>
#include <vtkImageData.h>
#include "TMJSliceWindow.h"
#include "TMJDrawSliceWindowInteractorStyle.h"
#include <iavGlobalData.h>


TMJSliceWindow::TMJSliceWindow(TMJDataStruct* a, int idx, const bool isSagittal, const bool isLeft)
	: m_sagittal{ isSagittal }
{
	m_pTMJData = a; 
    m_idxSliceWindow = idx;
	m_left = isLeft;

	viewData->SetRenderWindow(vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New());
	this->viewData->GetRenderer()->SetBackground(iavGlobalData::rendererBackground);
	//viewData->GetRenderWindow()->GlobalWarningDisplayOff();

	using namespace std::placeholders;
	MsgCenter::attach(this, std::bind(&TMJSliceWindow::msgHandler, this, _1, _2));
}

void TMJSliceWindow::msgHandler(const MsgCenter::Msg msg, const std::any& arg)
{
	switch (msg)
	{
	case MsgCenter::TMJShowImageViewer:
	{
		this->ComputeImagesViewerTMJ();

		vtkSmartPointer<TMJDrawSliceWindowInteractorStyle> sliceStyle = vtkSmartPointer<TMJDrawSliceWindowInteractorStyle>::New();
		sliceStyle->SetDataStructSliceWindow(m_pTMJData);
		sliceStyle->SetSliceWindowClass(this);
		this->viewData->GetRenderWindow()->GetInteractor()->SetInteractorStyle(sliceStyle);
	}
	break;

	default:
		break;
	}
}

vtkRenderWindow* TMJSliceWindow::GetRenderWindow()
{
    return this->viewData->GetRenderWindow();
}

// slice size and step
void TMJSliceWindow::ComputeImagesViewerTMJ()
{
	double spacing[3];
	iavGlobalData::getImageData()->GetSpacing(spacing);

	if (m_left)  // left
	{
		viewData->SetInputData(m_pTMJData->appendLeftTMJ->GetOutput());
		viewData->SetColorLevel(2200);
		viewData->SetColorWindow(6500);

		if (m_sagittal)  //冠状面
		{
			switch (m_idxSliceWindow)
			{
			case 0: {
				int idx1 = m_pTMJData->appendLeftTMJ->GetOutput()->GetDimensions()[1] / 2 + m_pTMJData->countOfForwardLeftTop- m_pTMJData->countOfBackLeftTop;
				//std::cout << "idx1: " << idx1 << std::endl;
				viewData->SetSliceOrientationToXZ();
				viewData->SetSlice(idx1);
				break;
			}
			case 1: {
				int idx2 = m_pTMJData->appendLeftTMJ->GetOutput()->GetDimensions()[1] / 2 + lengthOfSlice / spacing[2] + m_pTMJData->countOfForwardLeftTop - m_pTMJData->countOfBackLeftTop;
				//std::cout << "idx2: " << idx2 << std::endl;
				viewData->SetSliceOrientationToXZ();
				viewData->SetSlice(idx2);
				break;
			}
			case -1: {
				int idx3 = m_pTMJData->appendLeftTMJ->GetOutput()->GetDimensions()[1] / 2 - lengthOfSlice / spacing[2] + m_pTMJData->countOfForwardLeftTop - m_pTMJData->countOfBackLeftTop;
				//std::cout << "idx3: " << idx3 << std::endl;
				viewData->SetSliceOrientationToXZ();
				viewData->SetSlice(idx3);
				break;
			}
			default:
				break;
			}
			//viewData->GetRenderer()->GetActiveCamera()->Zoom(1.5);
		}
		else
		{
			switch (m_idxSliceWindow)
			{
			case 0: {
				int idx4 = m_pTMJData->appendLeftTMJ->GetOutput()->GetDimensions()[0] / 2 + m_pTMJData->countOfForwardLeftBottom - m_pTMJData->countOfBackLeftBottom;
				//std::cout << "idx4: " << idx4 << std::endl;
				viewData->SetSliceOrientationToYZ();
				viewData->SetSlice(idx4);
				break;
			}
			case 1: {
				int idx5 = m_pTMJData->appendLeftTMJ->GetOutput()->GetDimensions()[0] / 2 + lengthOfSlice / spacing[2] + m_pTMJData->countOfForwardLeftBottom - m_pTMJData->countOfBackLeftBottom;
				//std::cout << "idx5: " << idx5 << std::endl;
				viewData->SetSliceOrientationToYZ();
				viewData->SetSlice(idx5);
				break;
			}
			case -1: {
				int idx6 = m_pTMJData->appendLeftTMJ->GetOutput()->GetDimensions()[0] / 2 - lengthOfSlice / spacing[2] + m_pTMJData->countOfForwardLeftBottom - m_pTMJData->countOfBackLeftBottom;
				//std::cout << "idx6: " << idx6 << std::endl;
				viewData->SetSliceOrientationToYZ();
				viewData->SetSlice(idx6);
				break;
			}
			default:
				break;
			}
			//viewData->GetRenderer()->GetActiveCamera()->Zoom(1.5);
		}
	}
	else  // right
	{
		viewData->SetInputData(m_pTMJData->appendRightTMJ->GetOutput());
		viewData->SetColorLevel(2200);
		viewData->SetColorWindow(6500);
		//viewData->SetRenderWindow(m_TMJSliceRenW);

		if (m_sagittal)
		{
			switch (m_idxSliceWindow)
			{
			case 0: {
				int idx7 = m_pTMJData->appendRightTMJ->GetOutput()->GetDimensions()[1] / 2 + m_pTMJData->countOfForwardRightTop - m_pTMJData->countOfBackRightTop;
				//std::cout << "idx7: " << idx7 << std::endl;
				viewData->SetSliceOrientationToXZ();
				viewData->SetSlice(idx7);
				break;
			}
			case 1: {
				int idx8 = m_pTMJData->appendRightTMJ->GetOutput()->GetDimensions()[1] / 2 + lengthOfSlice / spacing[2] + m_pTMJData->countOfForwardRightTop - m_pTMJData->countOfBackRightTop;
				//std::cout << "idx8: " << idx8 << std::endl;
				viewData->SetSliceOrientationToXZ();
				viewData->SetSlice(idx8);
				break;
			}
			case -1: {
				int idx9 = m_pTMJData->appendRightTMJ->GetOutput()->GetDimensions()[1] / 2 - lengthOfSlice / spacing[2] + m_pTMJData->countOfForwardRightTop - m_pTMJData->countOfBackRightTop;
				//std::cout << "idx9: " << idx9 << std::endl;
				viewData->SetSliceOrientationToXZ();
				viewData->SetSlice(idx9);
				break;
			}
			default:
				break;
			}
			//viewData->GetRenderer()->GetActiveCamera()->Zoom(1.5);
		}
		else
		{
			switch (m_idxSliceWindow)
			{
			case 0: {
				int idx10 = m_pTMJData->appendRightTMJ->GetOutput()->GetDimensions()[0] / 2 + m_pTMJData->countOfForwardRightBottom - m_pTMJData->countOfBackRightBottom;
				//std::cout << "idx10: " << idx10 << std::endl;
				viewData->SetSliceOrientationToYZ();
				viewData->SetSlice(idx10);
				break;
			}
			case 1: {
				int idx11 = m_pTMJData->appendRightTMJ->GetOutput()->GetDimensions()[0] / 2 + lengthOfSlice / spacing[2] + m_pTMJData->countOfForwardRightBottom - m_pTMJData->countOfBackRightBottom;
				//std::cout << "idx11: " << idx11 << std::endl;
				viewData->SetSliceOrientationToYZ();
				viewData->SetSlice(idx11);
				break;
			}
			case -1: {
				int idx12 = m_pTMJData->appendRightTMJ->GetOutput()->GetDimensions()[0] / 2 - lengthOfSlice / spacing[2] + m_pTMJData->countOfForwardRightBottom - m_pTMJData->countOfBackRightBottom;
				//std::cout << "idx12: " << idx12 << std::endl;
				viewData->SetSliceOrientationToYZ();
				viewData->SetSlice(idx12);
				break;
			}
			default:
				break;
			}
			//viewData->GetRenderer()->GetActiveCamera()->Zoom(1.5);
		}
	}
	viewData->Render();
}

vtkImageViewer2* TMJSliceWindow::GetResliceImageViewer()
{
	return this->viewData;
}

SlicePosition TMJSliceWindow::GetCurrentPostion()
{
	SlicePosition tmjSlicePosition;
	tmjSlicePosition.s_idx = m_idxSliceWindow;
	tmjSlicePosition.s_isSagittal = m_sagittal;
	tmjSlicePosition.s_isLeft = m_left;
	
	return tmjSlicePosition;
}

void TMJSliceWindow::SetSlice(const int v)
{
	this->viewData->SetSlice(v);
}