#pragma once

#include <QFrame>
#include <QTabWidget>

#include <vtkImageData.h>
#include <vtkSmartPointer.h>

class Widget : public QFrame
{
	Q_OBJECT

public:
	Widget(QWidget* parent = nullptr);
	~Widget();

private:
	QTabWidget* m_tab = nullptr;
	vtkSmartPointer<vtkImageData> pImage;
};