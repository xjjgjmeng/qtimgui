#include "CPRTab.h"

#include <vtkSmartPointer.h>
#include <vtkRenderer.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkNamedColors.h>
#include <vtkCubeSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <QVTKOpenGLNativeWidget.h>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDebug>
#include <QSlider>
#include <QLabel>
#include <QComboBox>
#include <vtkDICOMImageReader.h>
#include <vtkImageData.h>
#include <vtkImageViewer2.h>
#include <vtkInteractorStyleImage.h>
#include <vtkNew.h>
#include <vtkResliceImageViewer.h>
#include <vtkRenderWindow.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <CPRDataStruct.h>

#include "MsgCenter.h"

CPRTab::CPRTab(QWidget *parent)
    : QWidget(parent)
{
	cprTab = new iavCPRViewer();
	std::function<void()> cprSliceChangedCallback;
	{
		auto h = new QHBoxLayout{ this };

		{
			auto v = new QVBoxLayout{};
			h->addLayout(v);
			auto m_btnNormal = new QPushButton{ "Normal Mode" };
			m_btnCpr = new QPushButton{ "Start CPR" };
			auto btnRoi = new QPushButton{ "ROI" };
			auto btnLine = new QPushButton{ "Line" };
			auto btnNerve = new QPushButton{ "Draw Nerve" };
			auto btnImplant = new QPushButton{ "Draw Implant" };

			v->addWidget(m_btnNormal);
			v->addWidget(m_btnCpr);
			v->addWidget(btnRoi);
			v->addWidget(btnLine);
			v->addWidget(btnNerve);
			v->addWidget(btnImplant);
			connect(m_btnNormal, &QPushButton::clicked, [&] {cprTab->useTool(ToolType::NORMAL); });
			connect(m_btnCpr, &QPushButton::clicked, [&] {cprTab->useTool(ToolType::CPR_DRAW_ARCH); });
			connect(btnRoi, &QPushButton::clicked, [&] {cprTab->useTool(ToolType::CPR_ROI); });
			connect(btnLine, &QPushButton::clicked, [&] {cprTab->useTool(ToolType::LINE); });
			connect(btnNerve, &QPushButton::clicked, [&] {cprTab->useTool(ToolType::CPR_DRAW_Nerve); });
			connect(btnImplant, &QPushButton::clicked, [&] {cprTab->useTool(ToolType::CPR_DRAW_IMPLANT); });
		}

		{
			auto axialView = new QVTKOpenGLNativeWidget{};
			auto pAxialLayout = new QVBoxLayout{ axialView };
			// 层厚调节
			{
				auto combobox = new QComboBox{};
				auto pLayout = new QHBoxLayout{};
				pLayout->addWidget(combobox);
				{
					auto pText = new QLabel{ "thickness" };
					pText->setStyleSheet("color: gray;");
					pLayout->addWidget(pText);
				}
				pLayout->addStretch(1);
				pAxialLayout->addLayout(pLayout);
				for (const auto i : { 1,3,5,7,9 })
				{
					combobox->addItem(QString{ "%1 mm" }.arg(i), i);
				}
				combobox->setCurrentIndex(-1);
				connect(combobox, QOverload<int>::of(&QComboBox::currentIndexChanged), [combobox, this]
					{
						const auto thickness = combobox->currentData().toInt();
						this->cprTab->setAxialThickness(thickness);
					});
			}
			// slice间隔调节
			{
				auto combobox = new QComboBox{};
				auto pLayout = new QHBoxLayout{};
				pLayout->addWidget(combobox);
				{
					auto pText = new QLabel{ "sliceInterval" };
					pText->setStyleSheet("color: gray;");
					pLayout->addWidget(pText);
				}
				pLayout->addStretch(1);
				pAxialLayout->addLayout(pLayout);
				const auto sliceInterval = CPRDataStruct::splineResamplingDistance;
				for (const auto i : { sliceInterval*1,sliceInterval*2,sliceInterval*5,sliceInterval*9,sliceInterval*10,sliceInterval*20,sliceInterval*80,sliceInterval*200,sliceInterval *500 })
				{
					combobox->addItem(QString{ "%1 mm" }.arg(i), i);
				}
				combobox->setCurrentIndex(-1);
				connect(combobox, QOverload<int>::of(&QComboBox::currentIndexChanged), [combobox, this]
					{
						const auto v = combobox->currentData().toDouble();
						this->cprTab->setSliceInterval(v);
					});
			}
			{
				auto pSlider = new QSlider{ Qt::Horizontal };
				pAxialLayout->addStretch(1);
				pAxialLayout->addWidget(pSlider);

				cprSliceChangedCallback = [pSlider, this]
					{
						const auto [sliceMin, sliceMax, sliceIdx] = this->cprTab->getAxialSliceIdx();
						pSlider->setRange(sliceMin, sliceMax);
						pSlider->setValue(sliceIdx);
					};
				//cprSliceChangedCallback();
				connect(pSlider, &QSlider::valueChanged, [this](auto v) {this->cprTab->setSlice(v); });
			}
			axialView->setRenderWindow(cprTab->getAxialWindow());
			//cprTab->SetDefaultInteractorStyle();
			auto expandedView = new QVTKOpenGLNativeWidget{};
			{
				auto combobox = new QComboBox{};
				auto pLayout = new QHBoxLayout{};
				pLayout->addWidget(combobox);
				{
					auto pText = new QLabel{ "thickness" };
					pText->setStyleSheet("color: gray;");
					pLayout->addWidget(pText);
				}
				pLayout->addStretch(1);
				auto pPanLayout = new QVBoxLayout{ expandedView };
				pPanLayout->addLayout(pLayout);
				pPanLayout->addStretch(1);
				for (const auto i : { 1,3,5,7,9 })
				{
					combobox->addItem(QString{ "%1 mm" }.arg(i), i);
				}
				combobox->setCurrentIndex(-1);
				connect(combobox, QOverload<int>::of(&QComboBox::currentIndexChanged), [combobox, this]
					{
						const auto thickness = combobox->currentData().toInt();
						this->cprTab->setPanThickness(thickness);
					});
			}
			expandedView->setRenderWindow(cprTab->getPanWindow());
			auto vrView = new QVTKOpenGLNativeWidget{};
			vrView->setRenderWindow(cprTab->getVRWindow());

			QVTKOpenGLNativeWidget* sliceView[5];
			for (auto& i : sliceView)
			{
				i = new QVTKOpenGLNativeWidget{};
				//i->setRenderWindow(getRenderWindow());
			}
			sliceView[0]->setRenderWindow(cprTab->getSliceWindow(-2));
			sliceView[1]->setRenderWindow(cprTab->getSliceWindow(-1));
			sliceView[2]->setRenderWindow(cprTab->getSliceWindow(0));
			sliceView[3]->setRenderWindow(cprTab->getSliceWindow(1));
			sliceView[4]->setRenderWindow(cprTab->getSliceWindow(2));

			// make layout
			auto myLayout = new QGridLayout{  };
			myLayout->setRowStretch(0, 1);
			myLayout->setRowStretch(1, 1);
			myLayout->addWidget(axialView, 0, 0, 1, 3);
			myLayout->addWidget(expandedView, 0, 3, 1, 5);
			myLayout->addWidget(vrView, 1, 0, 1, 3);
			for (std::size_t i = 0; i < std::size(sliceView); ++i)
			{
				myLayout->addWidget(sliceView[i], 1, 3 + i);
			}
			h->addLayout(myLayout, 1);
		}
	}

	this->cprTab->setOnDataChangedCallback([=](const iavCPRViewer::Msg msg)
		{
			switch (msg)
			{
			case iavCPRViewer::AxialSliceIdxChanged:
				cprSliceChangedCallback();
				break;
			default:
				break;
			}
		});
}

CPRTab::~CPRTab()
{
	delete cprTab;
}

