#include "iavCPRViewer.h"

#include "CPRDataStruct.h"
#include "CPRAxialWindow.h"
#include "CPRPanWindow.h"
#include "CPRVolumeRenderingWindow.h"
#include "CPRSliceWindow.h"

struct iavCPRViewer::Data
{
	CPRAxialWindow* transW;
	CPRPanWindow* panW;
	CPRVolumeRenderingWindow* vrW;
	CPRSliceWindow* sliceW1;
	CPRSliceWindow* sliceW2;
	CPRSliceWindow* sliceW3;
	CPRSliceWindow* sliceW4;
	CPRSliceWindow* sliceW5;
	CPRDataStruct m_data{};
	std::function<void(iavCPRViewer::Msg)> onDataChangedCallback;
};

iavCPRViewer::iavCPRViewer()
	: m_data{ new Data{} }
{
	//this->m_data->m_data.dcmReader->SetDirectoryName("D:\\test_data\\series");
	////a->dcmReader->SetDirectoryName("D:\\test_data\\202110020082000");
	//this->m_data->m_data.dcmReader->Update();

	this->m_data->transW = new CPRAxialWindow(&this->m_data->m_data);
	this->m_data->panW = new CPRPanWindow(&this->m_data->m_data);
	this->m_data->vrW  = new CPRVolumeRenderingWindow(&this->m_data->m_data);

	this->m_data->sliceW1 = new CPRSliceWindow(&this->m_data->m_data, 1);
	this->m_data->sliceW2 = new CPRSliceWindow(&this->m_data->m_data, 2);
	this->m_data->sliceW3 = new CPRSliceWindow(&this->m_data->m_data, 3);
	this->m_data->sliceW4 = new CPRSliceWindow(&this->m_data->m_data, 4);
	this->m_data->sliceW5 = new CPRSliceWindow(&this->m_data->m_data, 5);

	using namespace std::placeholders;
	MsgCenter::attach(this, std::bind(&iavCPRViewer::msgHandler, this, _1, _2));
}

void iavCPRViewer::msgHandler(const MsgCenter::Msg msg, const std::any& arg)
{
	switch (msg)
	{
	case MsgCenter::CprAxialSliceChanged:
		if (this->m_data->onDataChangedCallback)
		{
			this->m_data->onDataChangedCallback(AxialSliceIdxChanged);
		}
		break;
	case MsgCenter::GlobalImageLoaded:
		this->m_data->transW->loadImage();
		this->m_data->vrW->loadImage();
		break;
	default:
		break;
	}
}

iavCPRViewer::~iavCPRViewer()
{
	delete m_data;
}

void iavCPRViewer::useTool(const ToolType v)
{
	switch (v)
	{
	case ToolType::NORMAL:
		MsgCenter::send(MsgCenter::CprToolNormal);
		break;
	case ToolType::CPR_DRAW_ARCH:
		MsgCenter::send(MsgCenter::CprToolDrawArch);
		break;
	case ToolType::CPR_ROI:
		MsgCenter::send(MsgCenter::CprRoiAdjust);
		break;
	case ToolType::LINE:
		MsgCenter::send(MsgCenter::CprToolLine);
		break;
	case ToolType::CPR_DRAW_Nerve:
		MsgCenter::send(MsgCenter::CprDrawNerve);
		break;
	case ToolType::CPR_DRAW_IMPLANT:
		MsgCenter::send(MsgCenter::CprDrawImplant);
		break;
	default:
		break;
	}
}

vtkRenderWindow* iavCPRViewer::getAxialWindow()
{
	return this->m_data->transW->GetRenderWindow();
}

vtkRenderWindow* iavCPRViewer::getPanWindow()
{
	return this->m_data->panW->GetRenderWindow();
}

vtkRenderWindow* iavCPRViewer::getVRWindow()
{
	return this->m_data->vrW->GetRenderWindow();
}

vtkRenderWindow* iavCPRViewer::getSliceWindow(int idx)
{
	if (idx == -2) { return this->m_data->sliceW1->GetRenderWindow(); };
	if (idx == -1) { return this->m_data->sliceW2->GetRenderWindow(); };
	if (idx == 0) { return this->m_data->sliceW3->GetRenderWindow(); };
	if (idx == 1) { return this->m_data->sliceW4->GetRenderWindow(); };
	if (idx == 2) { return this->m_data->sliceW5->GetRenderWindow(); };

	return nullptr;
}

std::tuple<int, int, int> iavCPRViewer::getAxialSliceIdx() const
{
	return this->m_data->transW->getTransverseSliceIdx();
}

void iavCPRViewer::setSlice(const int v)
{
	this->m_data->transW->SetSlice(v);
}

void iavCPRViewer::setAxialThickness(const int v)
{
	this->m_data->transW->setThickness(v);
}

void iavCPRViewer::setPanThickness(const int v)
{
	this->m_data->panW->setThickness(v);
}

void iavCPRViewer::setSliceInterval(const double v)
{
	this->m_data->m_data.sliceInterval = v;
	MsgCenter::send(MsgCenter::CprSliceIntervalChanged);
}

void iavCPRViewer::setOnDataChangedCallback(std::function<void(Msg)> v)
{
	this->m_data->onDataChangedCallback = v;
}