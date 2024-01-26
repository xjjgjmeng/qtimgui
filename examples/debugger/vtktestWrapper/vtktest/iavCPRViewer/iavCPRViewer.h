#pragma once

#include <functional>
#include <memory>
#include <any>

#include <vtkRenderWindow.h>

#include <iavToolType.h>
#include <iavMacrosCPRViewer.h>

namespace MsgCenter
{
	enum Msg : int;
}

class IAVCPRVIEWER_API iavCPRViewer
{
public:
	enum Msg
	{
		AxialSliceIdxChanged, // 横断面的slice索引发生切换
	};

public:
	iavCPRViewer();
	~iavCPRViewer();

	void useTool(const ToolType v);
	vtkRenderWindow* getAxialWindow();
	vtkRenderWindow* getPanWindow();
	vtkRenderWindow* getVRWindow();
	// 参数为slice索引，正负表示相对中心切面的位置
	vtkRenderWindow* getSliceWindow(const int idx = 0);
	std::tuple<int, int, int> getAxialSliceIdx() const;
	void setSlice(const int v);
	void setAxialThickness(const int v);
	void setPanThickness(const int v);
	void setSliceInterval(const double v);
	void useWindowWidth(const double value) {}
	void useWindowLevel(const double value) {}
	void setSharpenValue(const double value) {}

	//std::function<void(Msg)> onCprDataChanged;
	void setOnDataChangedCallback(std::function<void(Msg)> v);

private:
	void msgHandler(const MsgCenter::Msg msg, const std::any& arg);

private:
	struct Data;
	Data* m_data = nullptr;
};