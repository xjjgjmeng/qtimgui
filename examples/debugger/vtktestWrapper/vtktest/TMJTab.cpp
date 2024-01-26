#include "TMJTab.h"
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
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <iavTMJViewer.h>
#include <TMJDataStruct.h>
#include <iavToolType.h>

TMJTab::TMJTab(QWidget *parent)
{
	tmjTab = new iavTMJViewer();
	std::function<void()> cprSliceChangedCallback;

	auto getRenderWindow = []
		{
			auto renderer = vtkSmartPointer<vtkRenderer>::New();
			auto renw = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
			vtkNew<vtkNamedColors> colors;
			vtkNew<vtkCubeSource> cubeSource;
			cubeSource->SetXLength(0.8);
			cubeSource->SetYLength(0.8);
			cubeSource->SetZLength(0.8);
			vtkNew<vtkPolyDataMapper> cubeMapper;
			cubeMapper->SetInputConnection(cubeSource->GetOutputPort());
			vtkNew<vtkActor> cubeActor;
			cubeActor->SetMapper(cubeMapper);
			cubeActor->GetProperty()->SetColor(colors->GetColor4d("MediumSeaGreen").GetData());
			renderer->AddActor(cubeActor);
			renw->AddRenderer(renderer);
			return renw;
		};

	auto pMainLayout = new QHBoxLayout{this};

	{
		auto pLayout = new QVBoxLayout{};
		pMainLayout->addLayout(pLayout, 1);

		// draw crosshair
		auto drawLine = new QPushButton{ "CrossHair" };
		pLayout->addWidget(drawLine);
		connect(drawLine, &QPushButton::clicked, [this]
			{
				//tmjTab->drawCrossHair();
				tmjTab->useTool(ToolType::TMJ_DRAW);
			});

		// image viewer
		auto imageSlice = new QPushButton{ "ROI" };
		pLayout->addWidget(imageSlice);
		connect(imageSlice, &QPushButton::clicked, [this]
			{
				//tmjTab->imageSliceShow();
				tmjTab->useTool(ToolType::TMJ_ROI);
			});
	}

	{
		QVTKOpenGLNativeWidget* topLeftSlice[3];
		QVTKOpenGLNativeWidget* topRightSlice[3];
		QVTKOpenGLNativeWidget* btmLeftSlice[3];
		QVTKOpenGLNativeWidget* btmRightSlice[3];

		QHBoxLayout *pLayoutTopLeft, *pLayoutBottomLeft, *pLayoutTopRight, *pLayoutBottomRight;

		{
			pLayoutTopLeft = new QHBoxLayout{};

			for (auto& i : topLeftSlice)
			{
				i = new QVTKOpenGLNativeWidget{};
				pLayoutTopLeft->addWidget(i);
			}
			topLeftSlice[0]->setRenderWindow(tmjTab->getLeftSagittalSliceWindow(-1));
			topLeftSlice[1]->setRenderWindow(tmjTab->getLeftSagittalSliceWindow(0));
			topLeftSlice[2]->setRenderWindow(tmjTab->getLeftSagittalSliceWindow(1));
		}

		{
			pLayoutBottomLeft = new QHBoxLayout{};
			for (auto& i : btmLeftSlice)
			{
				i = new QVTKOpenGLNativeWidget{};
				pLayoutBottomLeft->addWidget(i);
			}
			btmLeftSlice[0]->setRenderWindow(tmjTab->getLeftCoronalSliceWindow(-1));
			btmLeftSlice[1]->setRenderWindow(tmjTab->getLeftCoronalSliceWindow(0));
			btmLeftSlice[2]->setRenderWindow(tmjTab->getLeftCoronalSliceWindow(1));
		}

		{
			pLayoutTopRight = new QHBoxLayout{};
			for (auto& i : topRightSlice)
			{
				i = new QVTKOpenGLNativeWidget{};
				pLayoutTopRight->addWidget(i);
			}
			topRightSlice[0]->setRenderWindow(tmjTab->getRightSagittalSliceWindow(-1));
			topRightSlice[1]->setRenderWindow(tmjTab->getRightSagittalSliceWindow(0));
			topRightSlice[2]->setRenderWindow(tmjTab->getRightSagittalSliceWindow(1));
		}

		{
			pLayoutBottomRight = new QHBoxLayout{};
			for (auto& i : btmRightSlice)
			{
				i = new QVTKOpenGLNativeWidget{};
				pLayoutBottomRight->addWidget(i);
			}
			btmRightSlice[0]->setRenderWindow(tmjTab->getRightCoronalSliceWindow(-1));
			btmRightSlice[1]->setRenderWindow(tmjTab->getRightCoronalSliceWindow(0));
			btmRightSlice[2]->setRenderWindow(tmjTab->getRightCoronalSliceWindow(1));
		}

		//auto setupSliceArray = [getRenderWindow](auto& sliceArray)
		//	{
		//		auto pLayout = new QHBoxLayout{};
		//		for (auto& i : sliceArray)
		//		{
		//			auto w = new QVTKOpenGLNativeWidget{};
		//			w->setRenderWindow(getRenderWindow());
		//			i = w;
		//			pLayout->addWidget(w);
		//		}
		//		return pLayout;
		//	};

		// Layout of transverseWindow and VRWindow
		auto axialView = new QVTKOpenGLNativeWidget{};
		axialView->setRenderWindow(tmjTab->getAxialWindow());
#if 1
		{
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
						this->tmjTab->setAxialThickness(thickness);
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
				const auto sliceInterval = TMJDataStruct::splineResamplingDistance;
				for (const auto i : { sliceInterval * 1,sliceInterval * 2,sliceInterval * 5,sliceInterval * 9,sliceInterval * 10,sliceInterval * 20,sliceInterval * 80,sliceInterval * 200,sliceInterval * 500 })
				{
					combobox->addItem(QString{ "%1 mm" }.arg(i), i);
				}
				combobox->setCurrentIndex(-1);
				connect(combobox, QOverload<int>::of(&QComboBox::currentIndexChanged), [combobox, this]
					{
						const auto v = combobox->currentData().toDouble();
						this->tmjTab->setSliceInterval(v);
					});
			}
			{
				auto pSlider = new QSlider{ Qt::Horizontal };
				pAxialLayout->addStretch(1);
				pAxialLayout->addWidget(pSlider);

				cprSliceChangedCallback = [pSlider, this]
					{
						const auto [sliceMin, sliceMax, sliceIdx] = this->tmjTab->getAxialSliceIdx();
						pSlider->setRange(sliceMin, sliceMax);
						pSlider->setValue(sliceIdx);
					};
				//cprSliceChangedCallback();
				connect(pSlider, &QSlider::valueChanged, [this](auto v) {this->tmjTab->setSlice(v); });
			}
		}
#endif
		auto vrViewLeft = new QVTKOpenGLNativeWidget{};
		vrViewLeft->setRenderWindow(tmjTab->getLeftVRWindow());
		auto vrViewRight = new QVTKOpenGLNativeWidget{};
		vrViewRight->setRenderWindow(tmjTab->getRightVRWindow());
		auto vrLayout = new QHBoxLayout{};
		vrLayout->addWidget(vrViewLeft);
		vrLayout->addWidget(vrViewRight);

		// QT layout
		auto myLayout = new QGridLayout{};
		myLayout->setRowStretch(0, 1);
		myLayout->setRowStretch(1, 1);
		myLayout->setColumnStretch(0, 1);
		myLayout->setColumnStretch(1, 1);
		myLayout->setColumnStretch(2, 1);
		pMainLayout->addLayout(myLayout, 9);
		myLayout->addLayout(pLayoutTopLeft, 0, 0);
		myLayout->addWidget(axialView, 0, 1);
		myLayout->addLayout(pLayoutTopRight, 0, 2);
		myLayout->addLayout(pLayoutBottomLeft, 1, 0);
		myLayout->addLayout(vrLayout, 1, 1);
		myLayout->addLayout(pLayoutBottomRight, 1, 2);
	}

	this->tmjTab->setOnDataChangedCallback([=](const iavTMJViewer::Msg msg)
		{
			switch (msg)
			{
			case iavTMJViewer::AxialSliceIdxChanged:
				cprSliceChangedCallback();
				break;
			default:
				break;
			}
		});
}

TMJTab::~TMJTab()
{

}

