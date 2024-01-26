#pragma once

#include <vtkCamera.h>
#include <vtkImageAppend.h>
#include <vtkImageChangeInformation.h>
#include <vtkImageFlip.h>
#include <vtkImageReslice.h>
#include <vtkImageViewer2.h>
#include <vtkMatrix4x4.h>
#include <vtkInteractorStyleTrackballActor.h>
#include <vtkInteractorStyleImage.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkPolyDataMapper.h>
#include <vtkLineSource.h>
#include <vtkProperty.h>
#include <vtkCellPicker.h>
#include <vtkOpenGLActor.h>
#include <vtkRendererCollection.h>
#include <vtkParametricSpline.h>
#include <vtkParametricFunctionSource.h>
#include <vtkTubeFilter.h>
#include <vtkResliceCursor.h>
#include <vtkXMLImageDataWriter.h>
#include <vtkImageMapper3D.h>
#include <vtkImageProperty.h>
#include <vtkLine.h>
#include <vtkCoordinate.h>
#include <vtkActor2D.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkProperty2D.h>
#include <vtkProp3D.h>
#include <vtkCylinderSource.h>

#include "CPRPanWindow.h"
#include "iadAlgoritmCPR.h"
#include "InteractorStyleCprNormal.h"
#include "DrawNerveInteractorStyle.h"
#include "DrawImplantInteractorStyle.h"
#include "iavGlobalData.h"

class CprRoiLine : public vtkOpenGLActor
{
public:
	static CprRoiLine* New();
	vtkTypeMacro(CprRoiLine, vtkOpenGLActor);

	void SetPosition(double pos[3]) override
	{
		pos[0] = this->Position[0];
		pos[2] = this->Position[2];

		const auto minCenterY = CprRoiLine::MinLine->GetCenter()[1];
		const auto maxCenterY = CprRoiLine::MaxLine->GetCenter()[1];
		// ??
		if (std::abs(minCenterY - maxCenterY) < 10)
		{
			if ((this == CprRoiLine::MinLine && pos[1] > this->Position[1]) || (this == CprRoiLine::MaxLine && pos[1] < this->Position[1]))
			{
				pos[1] = this->Position[1];
			}	
		}

		__super::SetPosition(pos);

		m_pData->cprRoi = { CprRoiLine::MinLine->GetCenter()[1],CprRoiLine::MaxLine->GetCenter()[1] };
		MsgCenter::send(MsgCenter::CprRoiChanged);
	}

	CPRDataStruct* m_pData = nullptr;
	static CprRoiLine* MinLine;
	static CprRoiLine* MaxLine;
};
vtkStandardNewMacro(CprRoiLine);
CprRoiLine* CprRoiLine::MinLine = nullptr;
CprRoiLine* CprRoiLine::MaxLine = nullptr;
vtkStandardNewMacro(InteractorStyleCpr);

class CprRioStyle : public vtkInteractorStyleTrackballActor
{
public:
	static CprRioStyle* New();
	vtkTypeMacro(CprRioStyle, vtkInteractorStyleTrackballActor);

	void StartState(int newstate) override
	{
		newstate = VTKIS_PAN;
		__super::StartState(newstate);
	}

	void Init(const double bounds[6], CPRDataStruct* pData)
	{
		this->InteractionPicker->SetTolerance(0.01);
		this->m_ren = this->GetInteractor()->GetRenderWindow()->GetRenderers()->GetFirstRenderer();
		vtkNew<vtkLineSource> lineSourceMin;
		lineSourceMin->SetPoint1(bounds[0] + 0.1, bounds[2], bounds[4]);
		lineSourceMin->SetPoint2(bounds[0] + 0.1, bounds[2], bounds[5]);
		lineSourceMin->Update();
		vtkNew<vtkLineSource> lineSourceMax;
		lineSourceMax->SetPoint1(bounds[0] + 0.1, bounds[3], bounds[4]);
		lineSourceMax->SetPoint2(bounds[0] + 0.1, bounds[3], bounds[5]);
		lineSourceMax->Update();
		this->m_lineMin = vtkSmartPointer<CprRoiLine>::New();
		this->m_lineMax = vtkSmartPointer<CprRoiLine>::New();
		this->m_lineMin->m_pData = pData;
		this->m_lineMax->m_pData = pData;
		this->InteractionPicker->AddPickList(this->m_lineMin);
		this->InteractionPicker->AddPickList(this->m_lineMax);
		this->InteractionPicker->PickFromListOn();

		for (const auto& [pSrc, pActor] : std::list<std::pair<vtkLineSource*, CprRoiLine*>>{{lineSourceMin, this->m_lineMin}, {lineSourceMax, this->m_lineMax}})
		{
			vtkNew<vtkPolyDataMapper> lineMapper;
			lineMapper->SetInputConnection(pSrc->GetOutputPort());
			pActor->SetMapper(lineMapper);
			pActor->GetProperty()->SetLineWidth(10);
			pActor->GetProperty()->SetColor(1, 0, 0);
			this->GetInteractor()->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->AddActor(pActor);
		}

		CprRoiLine::MinLine = this->m_lineMin;
		CprRoiLine::MaxLine = this->m_lineMax;
		this->GetInteractor()->GetRenderWindow()->Render();
	}

protected:
	~CprRioStyle()
	{
		if (auto rw = this->m_ren->GetRenderWindow())
		{
			this->m_ren->RemoveActor(this->m_lineMin);
			this->m_ren->RemoveActor(this->m_lineMax);
			rw->Render();
		}
	}

private:
	vtkSmartPointer<CprRoiLine> m_lineMin;
	vtkSmartPointer<CprRoiLine> m_lineMax;
	vtkSmartPointer<vtkRenderer> m_ren;
};
vtkStandardNewMacro(CprRioStyle);

CPRPanWindow::CPRPanWindow(CPRDataStruct* a)
{
    m_pCprData = a; 

	m_viewer->SetRenderWindow(vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New());
	m_viewer->GetRenderWindow()->GlobalWarningDisplayOff();
	m_viewer->GetRenderer()->SetBackground(iavGlobalData::rendererBackground);

	m_straightVoumeCPR = vtkSmartPointer<vtkImageData>::New();
	actorNerveTube = vtkSmartPointer<vtkActor>::New();
	actorNerveTube2 = vtkSmartPointer<vtkActor>::New();

	m_resliceMapper = vtkSmartPointer<vtkImageResliceMapper>::New();
	normalStyle = vtkSmartPointer<InteractorStyleCpr>::New();
	m_implantActor = vtkSmartPointer<vtkActor2D>::New();
	implantCenterLine = vtkSmartPointer<vtkActor>::New();

	using namespace std::placeholders;
	MsgCenter::attach(this, std::bind(&CPRPanWindow::msgHandler, this, _1, _2));
}

void CPRPanWindow::msgHandler(const MsgCenter::Msg msg, const std::any& arg)
{
	switch (msg)
	{
	case MsgCenter::CprSplineFinishEditing:
	{
		this->ComputeStraightVolumeCPR();
		this->ShowStraightImage();
		/*this->ShowCursorLine();
		auto normalStyle = vtkSmartPointer<InteractorStyleCprNormal>::New();
		normalStyle->SetLinePolyData(cursorPolyData);
		normalStyle->SetLineActor(cursorActor);
		normalStyle->SetImageActor(this->m_viewer->GetImageActor());
		normalStyle->SetPanWindowClass(this);*/
		//auto normalStyle = vtkSmartPointer<CprNewLineStyle>::New();
		this->m_viewer->GetRenderWindow()->GetInteractor()->SetInteractorStyle(normalStyle);
		normalStyle->Init(this->m_viewer->GetImageActor()->GetBounds(), m_pCprData);
	}
		break;
	case MsgCenter::CprRoiAdjust:
		{
			auto pStyle = vtkSmartPointer<CprRioStyle>::New();
			this->m_viewer->GetRenderWindow()->GetInteractor()->SetInteractorStyle(pStyle);
			pStyle->Init(this->m_viewer->GetImageActor()->GetBounds(), m_pCprData);
		}
		break;
	case MsgCenter::CprToolDrawArch:
		{
			//vtkNew<vtkInteractorStyleImage> imageStyle;
			//imageStyle->SetKeyPressActivation(false);
			//this->m_viewer->GetRenderWindow()->GetInteractor()->SetInteractorStyle(imageStyle);

			vtkNew<vtkInteractorStyleTrackballCamera> trackball;
			trackball->SetKeyPressActivation(false);
			this->m_viewer->GetRenderWindow()->GetInteractor()->SetInteractorStyle(trackball);
		}
		break;
	case MsgCenter::CprRoiChanged:
		break;
	case MsgCenter::CprToolLine:
		this->m_distanceWidget = vtkSmartPointer<vtkDistanceWidget>::New();
		this->m_distanceWidget->SetInteractor(this->m_viewer->GetRenderWindow()->GetInteractor());
		this->m_distanceWidget->CreateDefaultRepresentation();
		this->m_distanceWidget->On();
		break;
	case MsgCenter::CprDrawNerve:
		{
			std::cout << "Drawing Nerve Start! " << std::endl;
			auto drawNerveStyle = vtkSmartPointer<DrawNerveInteractorStyle>::New();
			drawNerveStyle->SetKeyPressActivation(false);
			drawNerveStyle->SetPanWindowClass(this);
			this->m_viewer->GetRenderWindow()->GetInteractor()->SetInteractorStyle(drawNerveStyle);
		}
		break;
	case MsgCenter::CprUpdataNerveTube:
		this->ShowNerveTube();
		break;
	case MsgCenter::CprDrawImplant:
		{
			std::cout << "Drawing Implant Start! " << std::endl;
			auto drawImplantStyle = vtkSmartPointer<DrawImplantInteractorStyle>::New();
			drawImplantStyle->SetKeyPressActivation(false);
			drawImplantStyle->SetPanWindowClass(this);
			this->m_viewer->GetRenderWindow()->GetInteractor()->SetInteractorStyle(drawImplantStyle);
		}
		break;
	case MsgCenter::CprUpdateImplant:
		{
			this->ShowImplant();
			normalStyle->SetImplantActors(m_pCprData->ImplantActors);
			normalStyle->Modified();
		}
		break;
	case MsgCenter::CprUpdateImplantPosition:
		{
			this->UpdateImplantPosition();
		}
		break;
	case MsgCenter::CprToolNormal:
		{
		//auto normalStyle = vtkSmartPointer<CprNewLineStyle>::New();
		this->m_viewer->GetRenderWindow()->GetInteractor()->SetInteractorStyle(normalStyle);
		//normalStyle->Init(this->m_viewer->GetImageActor()->GetBounds(), m_pCprData);
		//this->m_viewer->GetRenderWindow()->GetInteractor()->SetInteractorStyle(normalStyle);
		}
		break;
	default:
		break;
	}
}

void CPRPanWindow::setThickness(const int v)
{
	int numOfSlabRange = floor(0.5 * v / m_straightVoumeCPR->GetSpacing()[2]);

	//slab
	vtkNew<vtkImageSlab> slab;
	slab->SetMultiSliceOutput(1);
	slab->SetInputData(m_straightVoumeCPR);
	slab->SetOrientationToX();
	slab->SetOperationToMean();
	//slab->SetTrapezoidIntegration(1);
	slab->SetSliceRange(m_viewer->GetSlice() - numOfSlabRange, m_viewer->GetSlice() + numOfSlabRange);
	slab->SetEnableSMP(1);
	slab->Update();

	m_viewer->SetInputData(slab->GetOutput());
	//m_viewer->SetSlice(slab->GetOutput()->GetDimensions()[0] / 2);
	m_viewer->GetRenderWindow()->Render();

	//vtkNew<vtkXMLImageDataWriter> writer;
	//writer->SetInputData(slab->GetOutput());
	//writer->SetFileName("slab.vti");
	//writer->Update();
}

vtkRenderWindow* CPRPanWindow::GetRenderWindow()
{
    return this->m_viewer->GetRenderWindow();
}

vtkImageViewer2* CPRPanWindow::GetResliceImageViewer()
{
	return this->m_viewer;
}

void CPRPanWindow::SetSlice(const int v)
{
	this->m_viewer->SetSlice(v);
}

void CPRPanWindow::ComputeStraightVolumeCPR()
{
	double sliceSizeMm[2] = { 150, 400 }; // 1:3; 1:3.5
	double spacing[3];
	iavGlobalData::getImageData()->GetSpacing(spacing);

	//std::cout << "m_lastClickedSplinePtID: " << m_lastClickedSplinePtID << std::endl;
	vtkSmartPointer<vtkImageAppend> appendCPR = vtkSmartPointer<vtkImageAppend>::New();
	appendCPR->SetAppendAxis(2);

	//m_straightVoumeCPR->SetAppendAxis(2);

	for (vtkIdType i = 0; i < m_pCprData->spline->GetNumberOfPoints(); i++)
	{
		double curPt[3];
		m_pCprData->spline->GetPoint(i, curPt);

		vtkNew<vtkMatrix4x4> resliceMatrix;
		resliceMatrix->Identity();
		resliceMatrix->SetElement(0, 0, m_pCprData->ctrlineNormal->GetTuple3(i)[0]);
		resliceMatrix->SetElement(1, 0, m_pCprData->ctrlineNormal->GetTuple3(i)[1]);
		resliceMatrix->SetElement(2, 0, m_pCprData->ctrlineNormal->GetTuple3(i)[2]);
		resliceMatrix->SetElement(0, 1, m_pCprData->ctrlineBinormal->GetTuple3(i)[0]);
		resliceMatrix->SetElement(1, 1, m_pCprData->ctrlineBinormal->GetTuple3(i)[1]);
		resliceMatrix->SetElement(2, 1, m_pCprData->ctrlineBinormal->GetTuple3(i)[2]);
		resliceMatrix->SetElement(0, 2, m_pCprData->ctrlineTangent->GetTuple3(i)[0]);
		resliceMatrix->SetElement(1, 2, m_pCprData->ctrlineTangent->GetTuple3(i)[1]);
		resliceMatrix->SetElement(2, 2, m_pCprData->ctrlineTangent->GetTuple3(i)[2]);
		resliceMatrix->SetElement(0, 3, curPt[0]);
		resliceMatrix->SetElement(1, 3, curPt[1]);
		resliceMatrix->SetElement(2, 3, curPt[2]);

		vtkNew<vtkImageReslice> reslice;
		reslice->SetInputData(iavGlobalData::getImageData());
		reslice->TransformInputSamplingOff();
		reslice->SetOutputDimensionality(2);
		reslice->SetInterpolationModeToCubic();
		//reslice->SetInterpolationModeToLinear();
		reslice->SetResliceAxes(resliceMatrix);
		reslice->SetOutputExtent(0, sliceSizeMm[0] - 1, 0, sliceSizeMm[1] - 1, 0, 0);
		reslice->SetOutputOrigin(-(sliceSizeMm[0] - 1) / 2 * spacing[0], -(sliceSizeMm[1] - 1) / 2 * spacing[1], 0);
		reslice->Update();

		//vtkNew<vtkImageChangeInformation> changer;
		//changer->SetInputData(reslice->GetOutput());
		//changer->SetOutputExtentStart(0, 0, 0);
		//changer->SetOutputOrigin(0, 0, 0);
		//changer->SetOutputSpacing(0.5, 0.5, 0.5);
		//changer->Update();

		appendCPR->AddInputData(reslice->GetOutput());
		//m_straightVoumeCPR->AddInputData(reslice->GetOutput());
	}
	appendCPR->Update();
	//m_straightVoumeCPR->Update();

	vtkNew<vtkImageFlip> flipFilter;
	flipFilter->SetInputData(appendCPR->GetOutput());
	flipFilter->SetFilteredAxes(2);
	flipFilter->Update();

	vtkNew<vtkImageChangeInformation> changer;
	changer->SetInputData(flipFilter->GetOutput());
	changer->SetOutputExtentStart(0, 0, 0);
	changer->SetOutputOrigin(0, 0, 0);
	changer->SetOutputSpacing(spacing[0], spacing[1], m_pCprData->splineResamplingDistance);
	changer->Update();

	m_straightVoumeCPR->ShallowCopy(changer->GetOutput());

	double splineLength;
	iadAlgorithmCPR alg;
	splineLength = alg.GetCurveLength(m_pCprData->spline->GetPoints(), 0, 0, m_pCprData->spline->GetNumberOfPoints());
	std::cout << "splineLength: " << splineLength << std::endl;

	//vtkNew<vtkXMLImageDataWriter> writer1;
	//writer1->SetInputData(changer->GetOutput());
	//writer1->SetFileName("C:\\Users\\DELL\\Desktop\\straightCPR.vti");
	//writer1->Write();

	m_pCprData->straightVolume->ShallowCopy(m_straightVoumeCPR);

}

void CPRPanWindow::ShowNerveTube()
{
	vtkNew<vtkParametricSpline> spline;
	if (m_pCprData->numOfTubes == 0)
	{
		spline->SetPoints(m_pCprData->NervePointsSetInPanWin);
	}
	else if (m_pCprData->numOfTubes == 1)
	{
		spline->SetPoints(m_pCprData->NervePointsSetInPanWin2);
	}
	
	vtkNew<vtkParametricFunctionSource> functionSource;
	functionSource->SetParametricFunction(spline);
	functionSource->Update();

	vtkNew<vtkTubeFilter> Tube;
	Tube->SetInputConnection(functionSource->GetOutputPort());
	Tube->SetRadius(m_pCprData->tubeRadius);
	Tube->SetNumberOfSides(20);
	Tube->CappingOn();
	Tube->SidesShareVerticesOn();
	Tube->Update();

	vtkNew<vtkPolyDataMapper> mapper;
	mapper->SetInputData(Tube->GetOutput());


	if (m_pCprData->numOfTubes == 0)
	{
		actorNerveTube->SetMapper(mapper);
		actorNerveTube->GetProperty()->SetColor(1, 0.5, 0);
		actorNerveTube->GetProperty()->SetOpacity(0.3);

		this->m_viewer->GetRenderer()->AddActor(actorNerveTube);
	}
	else if (m_pCprData->numOfTubes == 1)
	{
		actorNerveTube2->SetMapper(mapper);
		actorNerveTube2->GetProperty()->SetColor(1, 0.5, 0);
		actorNerveTube2->GetProperty()->SetOpacity(0.3);

		this->m_viewer->GetRenderer()->AddActor(actorNerveTube2);
	}

}

void CPRPanWindow::ShowStraightImage()
{
	// maybe changed to Reslice cursor widget in future?
	m_viewer->SetInputData(m_straightVoumeCPR);
	m_viewer->SetSliceOrientationToYZ();
	m_viewer->SetSlice(m_straightVoumeCPR->GetDimensions()[0]/2);
	m_viewer->SetColorLevel(2200);
	m_viewer->SetColorWindow(6500);

	vtkNew<vtkInteractorStyleImage> imageStyle;
	imageStyle->SetKeyPressActivation(false);
	this->m_viewer->GetRenderWindow()->GetInteractor()->SetInteractorStyle(imageStyle);

	m_viewer->GetRenderer()->ResetCamera();
	m_viewer->GetRenderer()->GetActiveCamera()->SetViewUp(0,1,0);
	m_viewer->GetRenderer()->GetActiveCamera()->Zoom(1.8);
	m_viewer->Render();	

}

void CPRPanWindow::ShowCursorLine()
{
	double currPointX = m_straightVoumeCPR->GetBounds()[1]/2;
	double currPointY = m_straightVoumeCPR->GetBounds()[3]/2;
	double currPointZ = (m_pCprData->currentPickedPointID + 1) * m_straightVoumeCPR->GetSpacing()[2];
	
	double point0[3] = { 0, currPointY, currPointZ};
	double point3[3] = { 0, currPointY-30000, currPointZ };
	double point4[3] = { 0, currPointY+30000.0, currPointZ };
	vtkSmartPointer<vtkPoints> Points = vtkSmartPointer<vtkPoints>::New();
	Points->InsertNextPoint(point0);
	Points->InsertNextPoint(point3);
	Points->InsertNextPoint(point4);

	cout << "point0: " << point0[1] << endl;
	cout << "point3: " << point3[1] << endl;
	cout << "point4: " << point4[1] << endl;

	//Longitudinalline1
	vtkSmartPointer<vtkLine> axisLine21 = vtkSmartPointer<vtkLine>::New();
	axisLine21->GetPointIds()->SetNumberOfIds(2);
	axisLine21->GetPointIds()->SetId(0, 0);
	axisLine21->GetPointIds()->SetId(1, 1);

	//Longitudinalline2
	vtkSmartPointer<vtkLine> axisLine22 = vtkSmartPointer<vtkLine>::New();
	axisLine22->GetPointIds()->SetNumberOfIds(2);
	axisLine22->GetPointIds()->SetId(0, 0);
	axisLine22->GetPointIds()->SetId(1, 2);

	vtkSmartPointer<vtkCellArray> lineCells2 = vtkSmartPointer<vtkCellArray>::New();
	lineCells2->InsertNextCell(axisLine21);
	lineCells2->InsertNextCell(axisLine22);

	cursorPolyData = vtkSmartPointer<vtkPolyData>::New();
	cursorPolyData->SetPoints(Points);
	cursorPolyData->SetLines(lineCells2);

	vtkSmartPointer<vtkCoordinate> cursorCoordinate = vtkSmartPointer<vtkCoordinate>::New();
	cursorCoordinate->SetCoordinateSystemToWorld();
	cursorMapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
	cursorMapper->SetInputData(cursorPolyData);
	cursorMapper->SetTransformCoordinate(cursorCoordinate);

	cursorActor = vtkSmartPointer<vtkActor2D>::New();
	cursorActor->SetMapper(cursorMapper);
	cursorActor->GetProperty()->SetColor(0, 153.0 / 255, 1);
	cursorActor->GetProperty()->SetLineWidth(2.5);

	this->m_viewer->GetRenderer()->AddActor(cursorActor);
	this->m_viewer->Render();

	m_pCprData->lineCursorCpy->ShallowCopy(cursorPolyData);
}

void CPRPanWindow::UpdateCursorLineTranslation()
{
	double bounds[6];
	m_straightVoumeCPR->GetBounds(bounds);
	double p0[3] = { cursorPolyData->GetPoint(0)[0],cursorPolyData->GetPoint(0)[1],cursorPolyData->GetPoint(0)[2] };
	double p1[3] = { cursorPolyData->GetPoint(1)[0],cursorPolyData->GetPoint(1)[1],cursorPolyData->GetPoint(1)[2] };
	double p2[3] = { cursorPolyData->GetPoint(2)[0],cursorPolyData->GetPoint(2)[1],cursorPolyData->GetPoint(2)[2] };

	vtkSmartPointer<vtkPoints> changedMovePoints = vtkSmartPointer<vtkPoints>::New();
	// 限定线的移动范围不能超出图像边界
	if (m_pCprData->MoveRotateAxisPoint[2] >= bounds[4] && m_pCprData->MoveRotateAxisPoint[2] <= bounds[5]) {
		double d = m_pCprData->MoveRotateAxisPoint[2] - p0[2];  // Z方向移动距离
		p0[0] = cursorPolyData->GetPoint(0)[0];
		p0[1] = cursorPolyData->GetPoint(0)[1];
		p0[2] = m_pCprData->MoveRotateAxisPoint[2];
		changedMovePoints->InsertNextPoint(p0);

		p1[0] = cursorPolyData->GetPoint(1)[0];
		p1[1] = cursorPolyData->GetPoint(1)[1];
		p1[2] = cursorPolyData->GetPoint(1)[2] + d;
		changedMovePoints->InsertNextPoint(p1);

		p2[0] = cursorPolyData->GetPoint(2)[0];
		p2[1] = cursorPolyData->GetPoint(2)[1];
		p2[2] = cursorPolyData->GetPoint(2)[2] + d;
		changedMovePoints->InsertNextPoint(p2);

		cursorPolyData->SetPoints(changedMovePoints);
		cursorPolyData->Modified();
		//ResliceUpdate(boundx, p0[0]);
		m_viewer->Render();
	}

	m_pCprData->CursorLineXCoords = p0[2];

}

void CPRPanWindow::UpdataCursorLineRotate()
{
	double p0[3] = { cursorPolyData->GetPoint(0)[0],cursorPolyData->GetPoint(0)[1],cursorPolyData->GetPoint(0)[2] };
	double p1[3] = { cursorPolyData->GetPoint(1)[0],cursorPolyData->GetPoint(1)[1],cursorPolyData->GetPoint(1)[2] };
	double p2[3] = { cursorPolyData->GetPoint(2)[0],cursorPolyData->GetPoint(2)[1],cursorPolyData->GetPoint(2)[2] };
	vtkSmartPointer<vtkPoints> changedRotatePoints = vtkSmartPointer<vtkPoints>::New();
	//// 旋转时的原点坐标、鼠标坐标
	double rotateOrigin[3] = { 0, cursorPolyData->GetPoint(0)[1], cursorPolyData->GetPoint(0)[2] };
	double rotatePoint3[3] = { 0, cursorPolyData->GetPoint(1)[1], cursorPolyData->GetPoint(1)[2] };
	double rotatePoint4[3] = { 0, cursorPolyData->GetPoint(2)[1], cursorPolyData->GetPoint(2)[2] };
	double rotateMouse[3] = { 0 ,m_pCprData->MoveRotateAxisPoint[1] ,m_pCprData->MoveRotateAxisPoint[2] };
	// 向量(原点，point3)  向量(原点，point4) 向量(原点，当前鼠标点) 初始垂直向量
	double pointVector3[3] = { 0,0,0 };
	double pointVector4[3] = { 0,0,0 };
	double mouseVector[3] = { 0,0,0 };
	pointVector3[2] = cursorPolyData->GetPoint(1)[2] - rotateOrigin[2];
	pointVector3[1] = cursorPolyData->GetPoint(1)[1] - rotateOrigin[1];
	pointVector4[2] = cursorPolyData->GetPoint(2)[2] - rotateOrigin[2];
	pointVector4[1] = cursorPolyData->GetPoint(2)[1] - rotateOrigin[1];
	mouseVector[2] = m_pCprData->MoveRotateAxisPoint[2] - rotateOrigin[2];
	mouseVector[1] = m_pCprData->MoveRotateAxisPoint[1] - rotateOrigin[1];
	// 根据向量之间夹角的弧度判断鼠标位于那条线上
	vtkMath::Normalize(pointVector3);
	vtkMath::Normalize(pointVector4);
	vtkMath::Normalize(mouseVector);
	double angleRadian3 = acos(vtkMath::Dot2D(pointVector3, mouseVector));
	double angleRadian4 = acos(vtkMath::Dot2D(pointVector4, mouseVector));
	changedRotatePoints->InsertNextPoint(p0);
	double initVector[3] = { 0,1,0 };
	if (angleRadian3 <= angleRadian4) {
		initVector[1] = -1;
		// 计算第一条线端点到原点的距离、鼠标点到原点的距离
		double d1 = sqrt(vtkMath::Distance2BetweenPoints(rotatePoint3, rotateOrigin));
		double d2 = sqrt(vtkMath::Distance2BetweenPoints(rotateMouse, rotateOrigin));
		// 计算坐标
		double z = d1 * (rotateMouse[2] - rotateOrigin[2]) / d2 + rotateOrigin[2];
		double y = d1 * (rotateMouse[1] - rotateOrigin[1]) / d2 + rotateOrigin[1];
		// 更新坐标,按照对称性计算
		p1[0] = 0;
		p1[1] = y;
		p1[2] = z;
		changedRotatePoints->InsertNextPoint(p1);
		p2[0] = 0;
		p2[1] = 2 * rotateOrigin[1] - y;
		p2[2] = 2 * rotateOrigin[2] - z;
		changedRotatePoints->InsertNextPoint(p2);
	}
	else {

		initVector[1] = 1;
		// 计算第二条线端点到原点的距离、鼠标点到原点的距离
		double d1 = sqrt(vtkMath::Distance2BetweenPoints(rotatePoint4, rotateOrigin));
		double d2 = sqrt(vtkMath::Distance2BetweenPoints(rotateMouse, rotateOrigin));
		// 根据相似三角形计算坐标
		double z = d1 * (rotateMouse[2] - rotateOrigin[2]) / d2 + rotateOrigin[2];
		double y = d1 * (rotateMouse[1] - rotateOrigin[1]) / d2 + rotateOrigin[1];
		// 更新坐标,按照对称性计算
		p1[0] = 0;
		p1[1] = 2 * rotateOrigin[1] - y;
		p1[2] = 2 * rotateOrigin[2] - z;
		changedRotatePoints->InsertNextPoint(p1);
		p2[0] = 0;
		p2[1] = y;
		p2[2] = z;
		changedRotatePoints->InsertNextPoint(p2);
	}
	m_pCprData->panCursorLineRoateAngleRadian = acos(vtkMath::Dot2D(initVector, mouseVector));
	// 限定旋转角度45度
	if (m_pCprData->panCursorLineRoateAngleRadian < M_PI * 0.25) {
		cursorPolyData->SetPoints(changedRotatePoints);
		cursorPolyData->Modified();
		//ResliceUpdate(boundx, p0[0]);
		m_viewer->Render();
	}
	else {
		m_pCprData->panCursorLineRoateAngleRadian = M_PI * 0.25;
	}
	// 判断方向
	if (p1[2] < p0[2])
	{
		m_pCprData->panCursorLineRoateAngleRadian = -1 * m_pCprData->panCursorLineRoateAngleRadian;
	}
	cout << " radian: " << m_pCprData->panCursorLineRoateAngleRadian << endl;

}

void CPRPanWindow::ShowImplant()
{
	//vtkNew<vtkPoints> points;
	//points->InsertNextPoint(m_pCprData->point0);
	//points->InsertNextPoint(m_pCprData->point1);

	vtkNew<vtkLineSource> lineSource;
	lineSource->SetPoints(m_pCprData->ImplantPointsSetInPanWin);
	lineSource->Update();

	vtkNew<vtkPolyDataMapper> lineMapper; 
	lineMapper->SetInputData(lineSource->GetOutput());

	implantCenterLine->SetMapper(lineMapper);
	implantCenterLine->GetProperty()->SetColor(0, 1, 0);
	implantCenterLine->GetProperty()->SetLineWidth(5);
	implantCenterLine->GetProperty()->VertexVisibilityOn();

	vtkNew<vtkTubeFilter> tubeFilter;
	tubeFilter->SetInputConnection(lineSource->GetOutputPort());
	tubeFilter->SetRadius(2.5);
	tubeFilter->SetNumberOfSides(50);
	tubeFilter->CappingOn();
	tubeFilter->SidesShareVerticesOn();
	tubeFilter->Update();

	vtkSmartPointer<vtkCoordinate> implantCoordinate = vtkSmartPointer<vtkCoordinate>::New();
	implantCoordinate->SetCoordinateSystemToWorld();
	//vtkNew<vtkPolyDataMapper> mapper;
	vtkNew<vtkPolyDataMapper2D> mapper;
	mapper->SetInputData(tubeFilter->GetOutput());
	mapper->SetTransformCoordinate(implantCoordinate);

	//vtkNew<vtkPolyDataMapper> mapper;
	//mapper->SetInputData(tubeFilter->GetOutput());

	m_implantActor->SetMapper(mapper);
	m_implantActor->GetProperty()->SetOpacity(0.5);
	m_implantActor->GetProperty()->SetColor(1, 1, 0);

	this->m_viewer->GetRenderer()->AddActor(m_implantActor);
	//this->m_viewer->GetRenderer()->AddActor(implantCenterLine);

	this->m_viewer->GetRenderWindow()->Render();

	/*m_pCprData->ImplantAssembly->AddPart(implantCenterLine);
	m_pCprData->ImplantAssembly->Modified();*/
	m_pCprData->ImplantActors[2] = implantCenterLine;

}

void CPRPanWindow::UpdateImplantPosition()
{
	vtkNew<vtkPoints> points;
	points->InsertNextPoint(m_pCprData->point0);
	points->InsertNextPoint(m_pCprData->point1);

	vtkNew<vtkLineSource> lineSource;
	lineSource->SetPoints(points);
	lineSource->Update();

	vtkNew<vtkTubeFilter> tubeFilter;
	tubeFilter->SetInputConnection(lineSource->GetOutputPort());
	tubeFilter->SetRadius(2.5);
	tubeFilter->SetNumberOfSides(50);
	tubeFilter->CappingOn();
	tubeFilter->SidesShareVerticesOn();
	tubeFilter->Update();

	vtkSmartPointer<vtkCoordinate> implantCoordinate = vtkSmartPointer<vtkCoordinate>::New();
	implantCoordinate->SetCoordinateSystemToWorld();

	vtkNew<vtkPolyDataMapper2D> mapper;
	mapper->SetInputData(tubeFilter->GetOutput());
	mapper->SetTransformCoordinate(implantCoordinate);

	m_implantActor->SetMapper(mapper);
	m_implantActor->GetProperty()->SetOpacity(0.5);
	m_implantActor->GetProperty()->SetColor(1, 1, 0);

	this->m_viewer->GetRenderer()->AddActor(m_implantActor);

	this->m_viewer->GetRenderWindow()->Render();
}

