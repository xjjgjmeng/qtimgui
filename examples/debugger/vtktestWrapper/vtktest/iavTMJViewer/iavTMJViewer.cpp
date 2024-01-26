#include "iavTMJViewer.h"

#include <MsgCenter.h>
#include <iavToolType.h>

#include "TMJDataStruct.h"
#include "AxialWindow.h"
#include "TMJVolumeRenderingWindow.h"
#include "TMJSliceWindow.h"

struct iavTMJViewer::Data
{
	AxialWindow* axial;
	TMJVolumeRenderingWindow* vrLeft;
	TMJVolumeRenderingWindow* vrRight;
	TMJSliceWindow* leftSagittalSlice_minus;
	TMJSliceWindow* leftSagittalSlice;
	TMJSliceWindow* leftSagittalSlice_plus;
	TMJSliceWindow* rightSagittalSlice_minus;
	TMJSliceWindow* rightSagittalSlice;
	TMJSliceWindow* rightSagittalSlice_plus;
	TMJSliceWindow* leftCoronalSlice_minus;
	TMJSliceWindow* leftCoronalSlice;
	TMJSliceWindow* leftCoronalSlice_plus;
	TMJSliceWindow* rightCoronalSlice_minus;
	TMJSliceWindow* rightCoronalSlice;
	TMJSliceWindow* rightCoronalSlice_plus;
	std::function<void(iavTMJViewer::Msg)> onDataChanged;
	TMJDataStruct tmjData;
};

iavTMJViewer::iavTMJViewer()
	: m_data{ new Data{} }
{
	this->m_data->axial = new AxialWindow(&this->m_data->tmjData);
	this->m_data->vrLeft = new TMJVolumeRenderingWindow(&this->m_data->tmjData, true);
	this->m_data->vrRight = new TMJVolumeRenderingWindow(&this->m_data->tmjData, false);

	this->m_data->leftSagittalSlice_minus = new TMJSliceWindow(&this->m_data->tmjData, -1, true, true);
	this->m_data->leftSagittalSlice = new TMJSliceWindow(&this->m_data->tmjData, 0, true, true);
	this->m_data->leftSagittalSlice_plus = new TMJSliceWindow(&this->m_data->tmjData, 1, true, true);

	this->m_data->rightSagittalSlice_minus = new TMJSliceWindow(&this->m_data->tmjData, -1, true, false);
	this->m_data->rightSagittalSlice = new TMJSliceWindow(&this->m_data->tmjData, 0, true, false);
	this->m_data->rightSagittalSlice_plus = new TMJSliceWindow(&this->m_data->tmjData, 1, true, false);

	this->m_data->leftCoronalSlice_minus = new TMJSliceWindow(&this->m_data->tmjData, -1, false, true);
	this->m_data->leftCoronalSlice = new TMJSliceWindow(&this->m_data->tmjData, 0, false, true);
	this->m_data->leftCoronalSlice_plus = new TMJSliceWindow(&this->m_data->tmjData, 1, false, true);

	this->m_data->rightCoronalSlice_minus = new TMJSliceWindow(&this->m_data->tmjData, -1, false, false);
	this->m_data->rightCoronalSlice = new TMJSliceWindow(&this->m_data->tmjData, 0, false, false);
	this->m_data->rightCoronalSlice_plus = new TMJSliceWindow(&this->m_data->tmjData, 1, false, false);

	using namespace std::placeholders;
	MsgCenter::attach(this, std::bind(&iavTMJViewer::msgHandler, this, _1, _2));
}

void iavTMJViewer::useTool(const ToolType v)
{
	switch (v)
	{
	case ToolType::TMJ_DRAW:
		MsgCenter::send(MsgCenter::TMJToolDrawLine);
		break;
	case ToolType::TMJ_ROI:
		MsgCenter::send(MsgCenter::TMJROI);
		break;
	default:
		break;
	}
}

inline void iavTMJViewer::setOnDataChangedCallback(std::function<void(Msg)> v)
{
	this->m_data->onDataChanged = v;
}

void iavTMJViewer::msgHandler(const MsgCenter::Msg msg, const std::any& arg)
{
	switch (msg)
	{
	case MsgCenter::TMJAxialSliceChanged:
		if (this->m_data->onDataChanged)
		{
			this->m_data->onDataChanged(AxialSliceIdxChanged);
		}
		break;
	case MsgCenter::GlobalImageLoaded:
		this->m_data->axial->loadImage();
		break;
	default:
		break;
	}
}

iavTMJViewer::~iavTMJViewer()
{
	delete m_data;
}

vtkRenderWindow* iavTMJViewer::getAxialWindow()
{
	return this->m_data->axial->GetRenderWindow();
}

vtkRenderWindow* iavTMJViewer::getLeftVRWindow()
{
	return this->m_data->vrLeft->GetRenderWindow();
}

vtkRenderWindow* iavTMJViewer::getRightVRWindow()
{
	return this->m_data->vrRight->GetRenderWindow();
}

vtkSmartPointer<vtkRenderWindow> iavTMJViewer::getLeftSagittalSliceWindow(const int idx)
{
	switch (idx)
	{
	case -1:
		return this->m_data->leftSagittalSlice_minus->GetRenderWindow();
	case 1:
		return this->m_data->leftSagittalSlice_plus->GetRenderWindow();
	default:
		return this->m_data->leftSagittalSlice->GetRenderWindow();
	}
}

vtkSmartPointer<vtkRenderWindow> iavTMJViewer::getRightSagittalSliceWindow(const int idx)
{
	switch (idx)
	{
	case -1:
		return this->m_data->rightSagittalSlice_minus->GetRenderWindow();
	case 1:
		return this->m_data->rightSagittalSlice_plus->GetRenderWindow();
	default:
		return this->m_data->rightSagittalSlice->GetRenderWindow();
	}
}

vtkSmartPointer<vtkRenderWindow> iavTMJViewer::getLeftCoronalSliceWindow(const int idx)
{
	switch (idx)
	{
	case -1:
		return this->m_data->leftCoronalSlice_minus->GetRenderWindow();
	case 1:
		return this->m_data->leftCoronalSlice_plus->GetRenderWindow();
	default:
		return this->m_data->leftCoronalSlice->GetRenderWindow();
	}
}

vtkSmartPointer<vtkRenderWindow> iavTMJViewer::getRightCoronalSliceWindow(const int idx)
{
	switch (idx)
	{
	case -1:
		return this->m_data->rightCoronalSlice_minus->GetRenderWindow();
	case 1:
		return this->m_data->rightCoronalSlice_plus->GetRenderWindow();
	default:
		return this->m_data->rightCoronalSlice->GetRenderWindow();
	}
}

std::tuple<int, int, int> iavTMJViewer::getAxialSliceIdx() const
{
	return this->m_data->axial->getTransverseSliceIdx();
}

void iavTMJViewer::setSlice(const int v)
{
	this->m_data->axial->SetSlice(v);
}

void iavTMJViewer::setAxialThickness(const int v)
{
	this->m_data->axial->setThickness(v);
}

void iavTMJViewer::setSliceInterval(const double v)
{
	this->m_data->tmjData.sliceInterval = v;
	MsgCenter::send(MsgCenter::TMJSliceIntervalChanged);
}