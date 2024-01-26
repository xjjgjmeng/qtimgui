#pragma once

#include <functional>
#include <memory>
#include <any>

#include <vtkRenderWindow.h>

#include "iavMacrosTMJViewer.h"

namespace MsgCenter
{
	enum Msg : int;
}

enum class ToolType;

class IAVTMJVIEWER_API iavTMJViewer
{
public:
	enum Msg
	{
		AxialSliceIdxChanged,
	};

public:
	iavTMJViewer();
	~iavTMJViewer();

	void useTool(const ToolType v);
	vtkRenderWindow* getAxialWindow();
	vtkRenderWindow* getLeftVRWindow();
	vtkRenderWindow* getRightVRWindow();
	// 参数为slice索引，正负表示相对中心切面的位置
	vtkSmartPointer<vtkRenderWindow> getLeftSagittalSliceWindow(const int idx = 0);
	vtkSmartPointer<vtkRenderWindow> getRightSagittalSliceWindow(const int idx = 0);
	vtkSmartPointer<vtkRenderWindow> getLeftCoronalSliceWindow(const int idx = 0);
	vtkSmartPointer<vtkRenderWindow> getRightCoronalSliceWindow(const int idx = 0);

	std::tuple<int, int, int> getAxialSliceIdx() const;
	void setSlice(const int v);
	void setAxialThickness(const int v);
	void setSliceInterval(const double v);
	void useWindowWidth(const double value) {}
	void useWindowLevel(const double value) {}
	void setSharpenValue(const double value) {}
	void setOnDataChangedCallback(std::function<void(Msg)> v);

private:
	void msgHandler(const MsgCenter::Msg msg, const std::any& arg);

private:
	struct Data;
	Data* m_data = nullptr;
};