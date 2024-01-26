#include <vtkCamera.h>
#include <vtkMatrix4x4.h>
#include <vtkImageChangeInformation.h>
#include <vtkImageFlip.h>
#include <vtkImageReslice.h>
#include <vtkImageSliceMapper.h>
#include <vtkImageProperty.h>
#include <vtkRendererCollection.h>
#include <vtkInteractorStyleImage.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkDistanceWidget.h>
#include <vtkProperty2D.h>
#include <vtkXMLImageDataWriter.h>
#include <vtkXMLPolyDataWriter.h>
#include <vtkTextProperty.h>
#include <vtkParametricSpline.h>
#include <vtkParametricFunctionSource.h>
#include <vtkTubeFilter.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkTransform.h>
#include <vtkImageSlice.h>
#include <vtkSphereSource.h>
#include <vtkLineSource.h>
#include <vtkTransformFilter.h>
#include <vtkCutter.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkAppendPolyData.h>

#include <math.h>

#include "CPRSliceWindow.h"
#include "iavGlobalData.h"


CPRSliceWindow::CPRSliceWindow(CPRDataStruct* a, int idx)
{
	m_pCprData = a; 
    m_idxSliceWindow = idx;

    m_cprSliceRen = vtkSmartPointer<vtkRenderer>::New();
    m_cprSliceRenW = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
	this->m_cprSliceRen->SetBackground(iavGlobalData::rendererBackground);

    m_cprSliceRenW->AddRenderer(m_cprSliceRen);

	m_resliceMapper = vtkSmartPointer<vtkImageResliceMapper>::New();
	m_reslicePlane = vtkSmartPointer<vtkPlane>::New();
	m_ImgActor = vtkSmartPointer<vtkImageActor>::New();

	m_CprSliceImgData = vtkSmartPointer<vtkImageData>::New();
	m_CprSliceImgDataRotate = vtkSmartPointer<vtkImageData>::New();
	m_CprSliceData = vtkSmartPointer<vtkImageSlice>::New();
	m_CprSliceDataRotate = vtkSmartPointer<vtkImageSlice>::New();

	Tube1 = vtkSmartPointer<vtkTubeFilter>::New();
	Tube2 = vtkSmartPointer<vtkTubeFilter>::New();
	actorNerveTube = vtkSmartPointer<vtkActor>::New();
	actorNerveTube2 = vtkSmartPointer<vtkActor>::New();
	actorNerveTubeIntersection = vtkSmartPointer<vtkActor2D>::New();

	actorImplantIntersection = vtkSmartPointer<vtkActor2D>::New();
	actorImplantCtrlineInVrW = vtkSmartPointer<vtkActor>::New();

	leftAnnotation = vtkSmartPointer<vtkCornerAnnotation>::New();
	//leftAnnotation->SetMaximumFontSize(20);
	leftAnnotation->SetText(6, " ");
	leftAnnotation->GetProperty()->SetColor(1.0, 0.72, 0.0);
	//leftAnnotation->GetTextProperty()->SetFontSize(20);

	rightAnnotation = vtkSmartPointer<vtkCornerAnnotation>::New();
	//rightAnnotation->SetMaximumFontSize(20);
	rightAnnotation->SetText(5, " ");
	rightAnnotation->GetProperty()->SetColor(1.0, 0.72, 0.0);
	//rightAnnotation->GetTextProperty()->SetFontSize(20);

	leftUpAnnotation = vtkSmartPointer<vtkCornerAnnotation>::New();
	leftUpAnnotation->SetText(2, " ");
	leftUpAnnotation->GetProperty()->SetColor(1.0, 0.72, 0.0);
	leftUpAnnotation->GetTextProperty()->SetFontSize(20);
	textProperty = vtkSmartPointer<vtkTextProperty>::New();
	textProperty = leftUpAnnotation->GetTextProperty();
	cout << *textProperty << endl;

	m_cprSliceRen->AddViewProp(leftAnnotation);
	m_cprSliceRen->AddViewProp(rightAnnotation);
	m_cprSliceRen->AddViewProp(leftUpAnnotation);

	using namespace std::placeholders;
	MsgCenter::attach(this, std::bind(&CPRSliceWindow::msgHandler, this, _1, _2));
}

void CPRSliceWindow::msgHandler(const MsgCenter::Msg msg, const std::any& arg)
{
	switch (msg)
	{
	case MsgCenter::CprSplineFinishEditing:
	case MsgCenter::CprSliceIntervalChanged:
		this->ComputeSliceImagesCPR();
		this->ShowSliceImages();
		this->ShowImplantIntersection();
		//this->ShowResliceImage();
		break;
	case MsgCenter::CprRoiChanged:
		break;
	case MsgCenter::CprToolDrawArch:
	{
		vtkNew<vtkInteractorStyleImage> imageStyle;
		imageStyle->SetKeyPressActivation(false);
		this->m_cprSliceRenW->GetInteractor()->SetInteractorStyle(imageStyle);
	}
	break;
	case MsgCenter::CprToolLine:
		this->m_distanceWidget = vtkSmartPointer<vtkDistanceWidget>::New();
		this->m_distanceWidget->SetInteractor(this->m_cprSliceRenW->GetInteractor());
		this->m_distanceWidget->CreateDefaultRepresentation();
		this->m_distanceWidget->On();
		break;
	case MsgCenter::CprUpdataNerveTube:
		this->ShowNerveTube();
		this->UpdateNerveTubeIntersection();
		break;
	case MsgCenter::CprUpdatePanCursorLineTranslate:
		this->ComputeSliceImagesCPR();
		this->ShowSliceImages();
		this->UpdateImplantIntersection();
		this->UpdateNerveTubeIntersection();
		break;
	//case MsgCenter::CprUpdatePanCursorLineRotate:
	//	this->ComputeSliceImagesCPR();
	//	this->ShowSliceImages();
	//	this->ShowImplant();
	//	break;
	case MsgCenter::CprUpdateImplant:
		this->UpdateImplantIntersection();
		//this->ShowImplantCenterline();
		this->UpdateNerveTubeIntersection();
		break;
	case MsgCenter::CprUpdateImplantPosition:
	{
		this->UpdateImplantIntersection();
		this->UpdateNerveTubeIntersection();
	}
	break;
	default:
		break;
	}
}

vtkRenderWindow* CPRSliceWindow::GetRenderWindow()
{
    return this->m_cprSliceRenW;
}

// slice size and step
void CPRSliceWindow::ComputeSliceImagesCPR()
{
	double sliceSizeMm[2] = { 150, 400 }; // 1:3; 1:3.5
	double spacing[3];
	iavGlobalData::getImageData()->GetSpacing(spacing);

	const int steps = std::round((m_idxSliceWindow - 3)*(m_pCprData->sliceInterval/CPRDataStruct::splineResamplingDistance)); // the third window is in the middel so -3

	double point[3];
	m_pCprData->spline->GetPoint(m_pCprData->currentPickedPointID + steps, point);

	vtkNew<vtkMatrix4x4> resliceMatrix;

	double tangent[3];
	tangent[0] = m_pCprData->ctrlineTangent->GetTuple3(m_pCprData->currentPickedPointID + steps)[0];
	tangent[1] = m_pCprData->ctrlineTangent->GetTuple3(m_pCprData->currentPickedPointID + steps)[1];
	tangent[2] = m_pCprData->ctrlineTangent->GetTuple3(m_pCprData->currentPickedPointID + steps)[2];

	double binormal[3];
	binormal[0] = m_pCprData->ctrlineBinormal->GetTuple3(m_pCprData->currentPickedPointID + steps)[0];
	binormal[1] = m_pCprData->ctrlineBinormal->GetTuple3(m_pCprData->currentPickedPointID + steps)[1];
	binormal[2] = m_pCprData->ctrlineBinormal->GetTuple3(m_pCprData->currentPickedPointID + steps)[2];

	double rotateW[4];
	rotateW[0] = m_pCprData->panCursorLineRoateAngleRadian;
	rotateW[1] = m_pCprData->ctrlineNormal->GetTuple3(m_pCprData->currentPickedPointID + steps)[0];
	rotateW[2] = m_pCprData->ctrlineNormal->GetTuple3(m_pCprData->currentPickedPointID + steps)[1];
	rotateW[3] = m_pCprData->ctrlineNormal->GetTuple3(m_pCprData->currentPickedPointID + steps)[2];

	double rotatedTangent[3];
	double rotatedBinormal[3];

	vtkMath::RotateVectorByWXYZ(tangent, rotateW, rotatedTangent);
	vtkMath::Normalize(rotatedTangent);

	vtkMath::RotateVectorByWXYZ(binormal, rotateW, rotatedBinormal);
	vtkMath::Normalize(rotatedBinormal);

	// Reslice Matrix Definition
	//{
	//	resliceMatrix->Identity();
	//	resliceMatrix->SetElement(0, 0, m_pCprData->ctrlineNormal->GetTuple3(m_pCprData->currentPickedPointID - steps)[0]);
	//	resliceMatrix->SetElement(1, 0, m_pCprData->ctrlineNormal->GetTuple3(m_pCprData->currentPickedPointID - steps)[1]);
	//	resliceMatrix->SetElement(2, 0, m_pCprData->ctrlineNormal->GetTuple3(m_pCprData->currentPickedPointID - steps)[2]);
	//	resliceMatrix->SetElement(0, 1, m_pCprData->ctrlineBinormal->GetTuple3(m_pCprData->currentPickedPointID - steps)[0]);
	//	resliceMatrix->SetElement(1, 1, m_pCprData->ctrlineBinormal->GetTuple3(m_pCprData->currentPickedPointID - steps)[1]);
	//	resliceMatrix->SetElement(2, 1, m_pCprData->ctrlineBinormal->GetTuple3(m_pCprData->currentPickedPointID - steps)[2]);
	//	resliceMatrix->SetElement(0, 2, m_pCprData->ctrlineTangent->GetTuple3(m_pCprData->currentPickedPointID - steps)[0]);
	//	resliceMatrix->SetElement(1, 2, m_pCprData->ctrlineTangent->GetTuple3(m_pCprData->currentPickedPointID - steps)[1]);
	//	resliceMatrix->SetElement(2, 2, m_pCprData->ctrlineTangent->GetTuple3(m_pCprData->currentPickedPointID - steps)[2]);
	//	resliceMatrix->SetElement(0, 3, point[0]);
	//	resliceMatrix->SetElement(1, 3, point[1]);
	//	resliceMatrix->SetElement(2, 3, point[2]);
	//}

	{
		resliceMatrix->Identity();
		resliceMatrix->SetElement(0, 0, m_pCprData->ctrlineNormal->GetTuple3(m_pCprData->currentPickedPointID + steps)[0]);
		resliceMatrix->SetElement(1, 0, m_pCprData->ctrlineNormal->GetTuple3(m_pCprData->currentPickedPointID + steps)[1]);
		resliceMatrix->SetElement(2, 0, m_pCprData->ctrlineNormal->GetTuple3(m_pCprData->currentPickedPointID + steps)[2]);
		resliceMatrix->SetElement(0, 1, rotatedBinormal[0]);
		resliceMatrix->SetElement(1, 1, rotatedBinormal[1]);
		resliceMatrix->SetElement(2, 1, rotatedBinormal[2]);
		resliceMatrix->SetElement(0, 2, rotatedTangent[0]);
		resliceMatrix->SetElement(1, 2, rotatedTangent[1]);
		resliceMatrix->SetElement(2, 2, rotatedTangent[2]);
		resliceMatrix->SetElement(0, 3, point[0]);
		resliceMatrix->SetElement(1, 3, point[1]);
		resliceMatrix->SetElement(2, 3, point[2]);
	}

	vtkNew<vtkImageReslice> reslice;

	// Image Reslice
	{
		reslice->SetInputData(iavGlobalData::getImageData());
		reslice->TransformInputSamplingOff();
		reslice->SetOutputDimensionality(2);
		reslice->SetInterpolationModeToLinear();
		reslice->SetResliceAxes(resliceMatrix);
		reslice->SetOutputExtent(0, sliceSizeMm[0] - 1, 0, sliceSizeMm[1] - 1, 0, 0);
		reslice->SetOutputOrigin(-(sliceSizeMm[0] - 1) / 2 * spacing[0], -(sliceSizeMm[1] - 1) / 2 * spacing[1], 0);
		reslice->SetEnableSMP(1);
		reslice->Update();
	}

	vtkNew<vtkImageChangeInformation> changer;

	// Change Image Info (origin & extent etc.)

	double zCoords = m_pCprData->straightVolume->GetCenter()[2] - steps * CPRDataStruct::splineResamplingDistance;

	changer->SetInputData(reslice->GetOutput());
	changer->SetOutputExtentStart(0, 0, 0);
	changer->SetOutputOrigin(0, 0, 0);
	changer->SetOutputSpacing(spacing[0], spacing[1], 0);
	changer->Update();


	vtkNew<vtkImageFlip> flipFilter1;
	flipFilter1->SetInputData(changer->GetOutput());
	flipFilter1->SetFilteredAxis(0);
	flipFilter1->Update();

	m_CprSliceImgData->ShallowCopy(flipFilter1->GetOutput());
	m_CprSliceImgDataRotate->DeepCopy(flipFilter1->GetOutput());

	//vtkNew<vtkXMLImageDataWriter> writer1;
	//writer1->SetInputData(m_CprSliceImgData);
	//writer1->SetFileName("C:\\Users\\DELL\\Desktop\\m_CprSliceData.vti");
	//writer1->Write();
}

void CPRSliceWindow::ShowSliceImages()
{
	vtkNew<vtkImageProperty> property;
	property->SetColorLevel(2200);
	property->SetColorWindow(6500);

	vtkNew<vtkImageSliceMapper> imageResliceMapper;
	imageResliceMapper->SetInputData(m_CprSliceImgData);

	vtkNew<vtkImageSliceMapper> imageResliceMapperRotate;
	imageResliceMapperRotate->SetInputData(m_CprSliceImgDataRotate);

	m_CprSliceData->SetMapper(imageResliceMapper);
	m_CprSliceData->SetProperty(property);

	m_CprSliceDataRotate->SetMapper(imageResliceMapperRotate);
	m_CprSliceDataRotate->SetProperty(property);

	double center[3];
	center[0] = m_CprSliceData->GetCenter()[0];
	center[1] = m_CprSliceData->GetCenter()[1];
	center[2] = m_CprSliceData->GetCenter()[2];

	const int steps = std::round((m_idxSliceWindow - 3) * (m_pCprData->sliceInterval / CPRDataStruct::splineResamplingDistance)); // the third window is in the middel so -3

	double point[3];
	m_pCprData->spline->GetPoint(m_pCprData->currentPickedPointID + steps, point);

	double zCoords = (m_pCprData->spline->GetNumberOfPoints() - m_pCprData->currentPickedPointID) * m_pCprData->splineResamplingDistance - steps * CPRDataStruct::splineResamplingDistance;

	vtkNew<vtkTransform> transform;
	transform->RotateX(-m_pCprData->panCursorLineRoateAngleRadian * 180 / M_PI);
	transform->Update();
	m_CprSliceData->SetUserTransform(transform);
	m_CprSliceData->Update();

	double newCenter[3];
	newCenter[0] = m_CprSliceData->GetCenter()[0];
	newCenter[1] = m_CprSliceData->GetCenter()[1];
	newCenter[2] = m_CprSliceData->GetCenter()[2];

	vtkNew<vtkTransform> transform2;
	transform2->Translate(m_pCprData->straightVolume->GetCenter()[0], m_pCprData->straightVolume->GetCenter()[1], zCoords);
	transform2->Translate(-newCenter[0], -newCenter[1], -newCenter[2]);
	transform2->RotateX(-m_pCprData->panCursorLineRoateAngleRadian * 180 / M_PI);
	transform2->Update();

	m_CprSliceDataRotate->SetUserTransform(transform2);
	m_CprSliceDataRotate->Update();

	//std::cout << "imageslice GetOrigin: " << m_CprSliceData->GetOrigin()[0] << " " << m_CprSliceData->GetOrigin()[1] << " " << m_CprSliceData->GetOrigin()[2] << std::endl;
	// for coordinate verification
	{
		//vtkNew<vtkSphereSource> pointSource;
		//pointSource->SetCenter(m_pCprData->straightVolume->GetCenter());
		////pointSource->SetNumberOfPoints(1);
		//pointSource->SetRadius(5.0);
		//pointSource->Update();

		//// Create a mapper and actor.
		//vtkNew<vtkPolyDataMapper> mapper;
		//mapper->SetInputConnection(pointSource->GetOutputPort());

		//vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
		//actor->SetMapper(mapper);
		//actor->GetProperty()->SetColor(1, 0.5, 0);
		//actor->GetProperty()->SetPointSize(5);


		//vtkNew<vtkSphereSource> pointSource2;
		//pointSource2->SetCenter(0, 0, 0);
		////pointSource->SetNumberOfPoints(1);
		//pointSource2->SetRadius(5.0);
		//pointSource2->Update();

		//// Create a mapper and actor.
		//vtkNew<vtkPolyDataMapper> mapper2;
		//mapper2->SetInputConnection(pointSource2->GetOutputPort());

		//vtkSmartPointer<vtkActor> actor2 = vtkSmartPointer<vtkActor>::New();
		//actor2->SetMapper(mapper2);
		//actor2->GetProperty()->SetColor(1, 0.5, 0);
		//actor2->GetProperty()->SetPointSize(5);


		//vtkNew<vtkLineSource> LineSourceX;
		//LineSourceX->SetPoint1(0, 0, 0);
		////pointSource->SetNumberOfPoints(1);
		//LineSourceX->SetPoint2(10, 0, 0);
		//LineSourceX->Update();

		//// Create a mapper and actor.
		//vtkNew<vtkPolyDataMapper> mapperLineSourceX;
		//mapperLineSourceX->SetInputConnection(LineSourceX->GetOutputPort());

		//vtkSmartPointer<vtkActor> actorLineSourceX = vtkSmartPointer<vtkActor>::New();
		//actorLineSourceX->SetMapper(mapperLineSourceX);
		//actorLineSourceX->GetProperty()->SetColor(1, 0.5, 0);
		//actorLineSourceX->GetProperty()->SetPointSize(5);

		//this->m_cprSliceRenW->GetRenderers()->GetFirstRenderer()->AddActor(actor);
		//this->m_cprSliceRenW->GetRenderers()->GetFirstRenderer()->AddActor(actor2);
		//this->m_cprSliceRenW->GetRenderers()->GetFirstRenderer()->AddActor(actorLineSourceX);
	}

	// camera direction calculation

	double rotateW[4];
	rotateW[0] = m_pCprData->panCursorLineRoateAngleRadian;
	rotateW[1] = -1;
	rotateW[2] = 0;
	rotateW[3] = 0;

	double binormal[3] = { 0 , 0, 1 };

	//double rotatedTangent[3];
	double rotatedBinormal[3];

	vtkMath::RotateVectorByWXYZ(binormal, rotateW, rotatedBinormal);
	vtkMath::Normalize(rotatedBinormal);

	rotatedNormal[0] = rotatedBinormal[0];
	rotatedNormal[1] = rotatedBinormal[1];
	rotatedNormal[2] = rotatedBinormal[2];

	double viewUpDirection[3];
	double xDirection[3] = { -1, 0, 0 };
	vtkMath::Cross(rotatedBinormal, xDirection, viewUpDirection);
	vtkMath::Normalize(viewUpDirection);

	// calculation camera postion
	double distanceToImageCenter = 10;
	double postionCemera[3];
	postionCemera[0] = m_CprSliceDataRotate->GetCenter()[0] + distanceToImageCenter * rotatedBinormal[0];
	postionCemera[1] = m_CprSliceDataRotate->GetCenter()[1] + distanceToImageCenter * rotatedBinormal[1];
	postionCemera[2] = m_CprSliceDataRotate->GetCenter()[2] + distanceToImageCenter * rotatedBinormal[2];

	// for camera position verification
	{
		//vtkNew<vtkSphereSource> pointSource;
		//pointSource->SetCenter(postionCemera);
		////pointSource->SetNumberOfPoints(1);
		//pointSource->SetRadius(5.0);
		//pointSource->Update();

		//// Create a mapper and actor.
		//vtkNew<vtkPolyDataMapper> mapper;
		//mapper->SetInputConnection(pointSource->GetOutputPort());

		//vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
		//actor->SetMapper(mapper);
		//actor->GetProperty()->SetColor(1, 0.5, 0);
		//actor->GetProperty()->SetPointSize(5);

		//vtkNew<vtkLineSource> LineSourceX;
		//LineSourceX->SetPoint1(0, 0, 0);
		////pointSource->SetNumberOfPoints(1);
		//LineSourceX->SetPoint2(10, 0, 0);
		//LineSourceX->Update();

		//// Create a mapper and actor.
		//vtkNew<vtkPolyDataMapper> mapperLineSourceX;
		//mapperLineSourceX->SetInputConnection(LineSourceX->GetOutputPort());

		//vtkSmartPointer<vtkActor> actorLineSourceX = vtkSmartPointer<vtkActor>::New();
		//actorLineSourceX->SetMapper(mapperLineSourceX);
		//actorLineSourceX->GetProperty()->SetColor(1, 0.5, 0);
		//actorLineSourceX->GetProperty()->SetPointSize(5);

		//this->m_cprSliceRenW->GetRenderers()->GetFirstRenderer()->AddActor(actor);;
		////this->m_cprSliceRenW->GetRenderers()->GetFirstRenderer()->AddActor(actorLineSourceX);
	}

	double bounds[6];
	m_CprSliceDataRotate->GetBounds(bounds);
	std::cout << "imageslice bounds: " << bounds[0] << " " << bounds[1] << " " << bounds[2] << " " << bounds[3] << " " << bounds[4] << " "  << bounds[5] << std::endl;

	this->m_cprSliceRenW->GetRenderers()->GetFirstRenderer()->AddActor(m_CprSliceDataRotate);
	this->m_cprSliceRenW->GetRenderers()->GetFirstRenderer()->GetActiveCamera()->SetViewUp(viewUpDirection);
	this->m_cprSliceRenW->GetRenderers()->GetFirstRenderer()->GetActiveCamera()->SetFocalPoint(m_CprSliceDataRotate->GetCenter());
	this->m_cprSliceRenW->GetRenderers()->GetFirstRenderer()->GetActiveCamera()->SetPosition(postionCemera[0], postionCemera[1], postionCemera[2]);
	this->m_cprSliceRenW->GetRenderers()->GetFirstRenderer()->ResetCamera();
	this->m_cprSliceRenW->GetRenderers()->GetFirstRenderer()->GetActiveCamera()->Roll(180);
	this->m_cprSliceRenW->GetRenderers()->GetFirstRenderer()->GetActiveCamera()->Zoom(1.8);

	this->m_cprSliceRenW->Render();

	double directionProjection[3];
	this->m_cprSliceRenW->GetRenderers()->GetFirstRenderer()->GetActiveCamera()->GetDirectionOfProjection(directionProjection);

	//std::cout << "imageslice rotatedBinormal: " << rotatedBinormal[0] << " " << rotatedBinormal[1] << " " << rotatedBinormal[2] << std::endl;
	//std::cout << "imageslice directionProjection: " << directionProjection[0] << " " << directionProjection[1] << " " << directionProjection[2] << std::endl;
	leftAnnotation->SetText(6, "B");
	rightAnnotation->SetText(5, "R");
	UpdateCornerDistanceText();
	

}


void CPRSliceWindow::ShowNerveTube()
{
	vtkNew<vtkParametricSpline> spline;
	vtkNew<vtkParametricFunctionSource> functionSource;

	vtkNew<vtkPolyDataMapper> mapper;
	
	if (m_pCprData->numOfTubes == 0)
	{
		spline->SetPoints(m_pCprData->NervePointsSetInPanWin);

		functionSource->SetParametricFunction(spline);
		functionSource->Update();

		//vtkNew<vtkTubeFilter> Tube;
		Tube1->SetInputConnection(functionSource->GetOutputPort());
		Tube1->SetRadius(m_pCprData->tubeRadius);
		Tube1->SetNumberOfSides(20);
		Tube1->CappingOn();
		Tube1->SidesShareVerticesOn();
		Tube1->Update();

		mapper->SetInputData(Tube1->GetOutput());
	}
	else if (m_pCprData->numOfTubes == 1)
	{
		spline->SetPoints(m_pCprData->NervePointsSetInPanWin2);

		functionSource->SetParametricFunction(spline);
		functionSource->Update();

		Tube2->SetInputConnection(functionSource->GetOutputPort());
		Tube2->SetRadius(m_pCprData->tubeRadius);
		Tube2->SetNumberOfSides(20);
		Tube2->CappingOn();
		Tube2->SidesShareVerticesOn();
		Tube2->Update();

		mapper->SetInputData(Tube2->GetOutput());
	}

	//vtkNew<vtkTubeFilter> Tube;
	//Tube->SetInputConnection(functionSource->GetOutputPort());
	//Tube->SetRadius(m_pCprData->tubeRadius);
	//Tube->SetNumberOfSides(20);
	//Tube->CappingOn();
	//Tube->SidesShareVerticesOn();
	//Tube->Update();

	if (m_pCprData->numOfTubes == 0)
	{
		actorNerveTube->SetMapper(mapper);
		actorNerveTube->GetProperty()->SetColor(1, 0.5, 0);
		actorNerveTube->GetProperty()->SetOpacity(0.3);

		//m_cprSliceRen->AddActor(actorNerveTube);
	}
	else if (m_pCprData->numOfTubes == 1)
	{
		actorNerveTube2->SetMapper(mapper);
		actorNerveTube2->GetProperty()->SetColor(1, 0.5, 0);
		actorNerveTube2->GetProperty()->SetOpacity(0.3);

		//m_cprSliceRen->AddActor(actorNerveTube2);
	}

	//m_cprSliceRenW->Render();
}

void CPRSliceWindow::UpdateNerveTubeIntersection()
{
	vtkNew<vtkPlane> cuttingPlane;
	cuttingPlane->SetNormal(rotatedNormal); // re-calculate it 
	cuttingPlane->SetOrigin(m_CprSliceDataRotate->GetCenter());
	cuttingPlane->Modified();

	std::cout << "m_CprSliceDataRotate->GetCenter(): " << m_CprSliceDataRotate->GetCenter()[0] << " " << m_CprSliceDataRotate->GetCenter()[1] << " " << m_CprSliceDataRotate->GetCenter()[2] << std::endl;

	vtkNew<vtkCutter> cutter;
	cutter->SetInputData(Tube1->GetOutput());
	cutter->SetCutFunction(cuttingPlane);
	cutter->SetGenerateTriangles(0);
	cutter->SetGenerateCutScalars(0);
	cutter->Update();

	vtkNew<vtkCutter> cutter2;
	cutter2->SetInputData(Tube2->GetOutput());
	cutter2->SetCutFunction(cuttingPlane);
	cutter2->SetGenerateTriangles(0);
	cutter2->SetGenerateCutScalars(0);
	cutter2->Update();

	////cutter->GetOutput()->GetPoints();
	//vtkNew<vtkXMLPolyDataWriter> writer;
	//writer->SetInputData(cutter->GetOutput());
	//writer->SetFileName("C:\\Users\\DELL\\Desktop\\implantCutter.vtp");
	//writer->Write();

	vtkNew<vtkAppendPolyData> appendPoly;
	appendPoly->AddInputData(cutter->GetOutput());
	appendPoly->AddInputData(cutter2->GetOutput());
	appendPoly->Update();

	vtkSmartPointer<vtkCoordinate> implantCoordinate = vtkSmartPointer<vtkCoordinate>::New();
	implantCoordinate->SetCoordinateSystemToWorld();

	vtkNew<vtkPolyDataMapper2D> mapper;
	mapper->SetInputData(appendPoly->GetOutput());
	mapper->SetTransformCoordinate(implantCoordinate);

	actorNerveTubeIntersection->SetMapper(mapper);
	actorNerveTubeIntersection->GetProperty()->SetLineWidth(2);
	actorNerveTubeIntersection->GetProperty()->SetColor(1.0, 0.5, 0);
	actorNerveTubeIntersection->Modified();

	this->m_cprSliceRen->AddActor(actorNerveTubeIntersection);
	this->m_cprSliceRenW->Render();
}


void CPRSliceWindow::ShowImplantIntersection()
{
	std::cout << "ShowImplant" << std::endl;;
	vtkNew<vtkLineSource> lineSource;
	lineSource->SetPoints(m_pCprData->ImplantPointsSetInPanWin);
	lineSource->Update();

	vtkNew<vtkTubeFilter> tubeFilter;
	tubeFilter->SetInputConnection(lineSource->GetOutputPort());
	tubeFilter->SetRadius(2.5);
	tubeFilter->SetNumberOfSides(50);
	tubeFilter->CappingOn();
	tubeFilter->SidesShareVerticesOn();
	tubeFilter->Update();

	vtkNew<vtkPlane> cuttingPlane;
	cuttingPlane->SetNormal(rotatedNormal); // re-calculate it 
	cuttingPlane->SetOrigin(m_CprSliceDataRotate->GetCenter());
	cuttingPlane->Modified();

	std::cout << "m_CprSliceDataRotate->GetCenter() " << m_idxSliceWindow << ":" << m_CprSliceDataRotate->GetCenter()[0] << " " << m_CprSliceDataRotate->GetCenter()[1] << " " << m_CprSliceDataRotate->GetCenter()[2] << std::endl;

	vtkNew<vtkCutter> cutter;
	cutter->SetInputData(tubeFilter->GetOutput());
	cutter->SetCutFunction(cuttingPlane);
	cutter->SetGenerateTriangles(0);
	cutter->SetGenerateCutScalars(0);
	cutter->Update();

	////cutter->GetOutput()->GetPoints();
	//vtkNew<vtkXMLPolyDataWriter> writer;
	//writer->SetInputData(cutter->GetOutput());
	//writer->SetFileName("C:\\Users\\DELL\\Desktop\\implantCutter.vtp");
	//writer->Write();

	vtkSmartPointer<vtkCoordinate> implantCoordinate = vtkSmartPointer<vtkCoordinate>::New();
	implantCoordinate->SetCoordinateSystemToWorld();

	vtkNew<vtkPolyDataMapper2D> mapper;
	mapper->SetInputData(cutter->GetOutput());
	mapper->SetTransformCoordinate(implantCoordinate);

	actorImplantIntersection->SetMapper(mapper);
	actorImplantIntersection->GetProperty()->SetLineWidth(2);
	actorImplantIntersection->GetProperty()->SetColor(255.0, 255.0, 0);
	actorImplantIntersection->Modified();

	this->m_cprSliceRen->AddActor(actorImplantIntersection);
	this->m_cprSliceRenW->Render();
}

void CPRSliceWindow::UpdateImplantIntersection()
{
	std::cout << "ShowImplant" << std::endl;;
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

	vtkNew<vtkPlane> cuttingPlane;
	cuttingPlane->SetNormal(rotatedNormal); // re-calculate it 
	cuttingPlane->SetOrigin(m_CprSliceDataRotate->GetCenter());
	cuttingPlane->Modified();

	std::cout << "m_CprSliceDataRotate->GetCenter(): " << m_CprSliceDataRotate->GetCenter()[0] << " " << m_CprSliceDataRotate->GetCenter()[1] << " " << m_CprSliceDataRotate->GetCenter()[2] << std::endl;

	vtkNew<vtkCutter> cutter;
	cutter->SetInputData(tubeFilter->GetOutput());
	cutter->SetCutFunction(cuttingPlane);
	cutter->SetGenerateTriangles(0);
	cutter->SetGenerateCutScalars(0);
	cutter->Update();

	////cutter->GetOutput()->GetPoints();
	//vtkNew<vtkXMLPolyDataWriter> writer;
	//writer->SetInputData(cutter->GetOutput());
	//writer->SetFileName("C:\\Users\\DELL\\Desktop\\implantCutter.vtp");
	//writer->Write();

	vtkSmartPointer<vtkCoordinate> implantCoordinate = vtkSmartPointer<vtkCoordinate>::New();
	implantCoordinate->SetCoordinateSystemToWorld();

	vtkNew<vtkPolyDataMapper2D> mapper;
	mapper->SetInputData(cutter->GetOutput());
	mapper->SetTransformCoordinate(implantCoordinate);

	actorImplantIntersection->SetMapper(mapper);
	actorImplantIntersection->GetProperty()->SetLineWidth(2);
	actorImplantIntersection->GetProperty()->SetColor(255.0, 255.0, 0);
	actorImplantIntersection->Modified();

	this->m_cprSliceRen->AddActor(actorImplantIntersection);
	this->m_cprSliceRenW->Render();
}

void CPRSliceWindow::UpdateCornerDistanceText()
{
	double d = (m_pCprData->currentPickedPointID * m_pCprData->splineResamplingDistance)
		+ (m_idxSliceWindow-3)* m_pCprData->sliceInterval;
	std::string stringDistance = std::to_string(d);
	size_t dotPosition = stringDistance.find('.');
	stringDistance = stringDistance.substr(0, 5) + "mm";
	leftUpAnnotation->SetText(2, stringDistance.c_str());
	
}
