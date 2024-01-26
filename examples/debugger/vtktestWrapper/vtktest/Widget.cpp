#include "Widget.h"

#include <QVBoxLayout>

#include <vtkDICOMImageReader.h>

#include <iavGlobalData.h>

#include "TMJTab.h"
#include "CPRTab.h"

Widget::Widget(QWidget* parent)
	: QFrame{parent}
{
	auto pLayout = new QVBoxLayout{ this };
	this->m_tab = new QTabWidget{};
	pLayout->addWidget(this->m_tab);
	this->m_tab->addTab(new CPRTab{}, "CPR");
	this->m_tab->addTab(new TMJTab{}, "TMJ");

	vtkNew<vtkDICOMImageReader> pReader;
	pReader->SetDirectoryName("D:\\test_data\\series");
	pReader->Update();
	this->pImage = pReader->GetOutput();
	iavGlobalData::setImageData(this->pImage);
}

Widget::~Widget()
{
}