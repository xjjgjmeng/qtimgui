#include <vtkInteractorStyleTrackballActor.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkCellPicker.h>
#include <vtkCamera.h>
#include <vtkRenderer.h>
#include <vtkSphereSource.h>
#include <vtkRenderWindow.h>
#include <vtkTransform.h>
#include <vtkAssembly.h>
#include <vtkRendererCollection.h>
#include <vtkPoints.h>
#include <vtkLineSource.h>
#include <vtkActor.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkVertexGlyphFilter.h>
#include <cmath>

#include "InteractorStyleCpr.h"
#include "MsgCenter.h"


void InteractorStyleCpr::OnLeftButtonDown()
{
	int x = this->Interactor->GetEventPosition()[0];
	int y = this->Interactor->GetEventPosition()[1];
	int z = this->Interactor->GetEventPosition()[2];
	this->FindPokedRenderer(x, y);
	this->FindPickedActor(x, y);

	// 判断平移or旋转
	// 坐标转换：y坐标位于图像内则平移，否则旋转
	this->InteractionPicker->Pick(x, y, z, this->GetInteractor()->GetRenderWindow()->GetRenderers()->GetFirstRenderer());
	this->InteractionPicker->GetPickPosition(MouseCoords);
	// 定位线的交互
	if (this->InteractionProp == m_lineActor) {
		if (MouseCoords[1] >= Bounds[2] && MouseCoords[1] <= Bounds[3]) {
			this->StartPan();  // 平移
		}
		else {
			this->StartSpin();  // 旋转
		}
	}
	// 种植体的交互
	else if (this->InteractionProp == this->pImplantCtrLineActor && this->InteractionProp!=nullptr)
	{
		cout << "line------------------------" << endl;
		vtkRenderWindowInteractor* rwi = this->Interactor;
		double* obj_center = this->InteractionProp->GetCenter();
		double disp_obj_center[3], old_pick_point[4], new_pick_point[4];
		this->ComputeWorldToDisplay(obj_center[0], obj_center[1], obj_center[2], disp_obj_center);
		this->ComputeDisplayToWorld(rwi->GetEventPosition()[0], rwi->GetEventPosition()[1], disp_obj_center[2], new_pick_point);

		cout << "lineLength: " << lineLength << endl;
		// 计算当前拾取的点到线中心的距离
		double pickPoint[3] = { new_pick_point[0],new_pick_point[1],new_pick_point[2] };
		double distanceToCenter = sqrt(vtkMath::Distance2BetweenPoints(pickPoint, obj_center));
		cout << "distanceToCenter: " << distanceToCenter << endl;
		double threshold = lineLength * 0.5 * 0.8;
		if (distanceToCenter <= threshold)
		{
			std::cout << "Pan" << std::endl;
			this->StartPan();
		}
		else
		{
			std::cout << "Spin" << std::endl;
			this->StartSpin();
			//MsgCenter::send(MsgCenter::CprUpdateImplantPosition);
		}

	}
}

void InteractorStyleCpr::OnLeftButtonUp()
{
	this->StopState();
}

void InteractorStyleCpr::Spin()
{

	if (this->CurrentRenderer == nullptr || this->InteractionProp == nullptr)
	{
		return;
	}

	vtkRenderWindowInteractor* rwi = this->Interactor;

	// 定位线交互
	if (this->InteractionProp == m_lineActor)
	{
		vtkCamera* cam = this->CurrentRenderer->GetActiveCamera();

		// Get the axis to rotate around = vector from eye to origin

		double* obj_center = this->InteractionProp->GetCenter();
		double motion_vector[3];
		double view_point[3];

		if (cam->GetParallelProjection())
		{
			// If parallel projection, want to get the view plane normal...
			cam->ComputeViewPlaneNormal();
			cam->GetViewPlaneNormal(motion_vector);
		}
		else
		{
			// Perspective projection, get vector from eye to center of actor
			// 透视投影
			cam->GetPosition(view_point);
			motion_vector[0] = view_point[0] - obj_center[0];
			motion_vector[1] = view_point[1] - obj_center[1];
			motion_vector[2] = view_point[2] - obj_center[2];
			vtkMath::Normalize(motion_vector);
		}

		double disp_obj_center[3];

		this->ComputeWorldToDisplay(obj_center[0], obj_center[1], obj_center[2], disp_obj_center);

		double newAngle =
			vtkMath::DegreesFromRadians(atan2(rwi->GetEventPosition()[1] - disp_obj_center[1],
				rwi->GetEventPosition()[0] - disp_obj_center[0]));

		double oldAngle =
			vtkMath::DegreesFromRadians(atan2(rwi->GetLastEventPosition()[1] - disp_obj_center[1],
				rwi->GetLastEventPosition()[0] - disp_obj_center[0]));

		double scale[3];
		scale[0] = scale[1] = scale[2] = 1.0;

		double** rotate = new double* [1];
		rotate[0] = new double[4];

		rotate[0][0] = newAngle - oldAngle;
		rotate[0][1] = motion_vector[0];
		rotate[0][2] = motion_vector[1];
		rotate[0][3] = 0; // 旋转

		// 旋转角度限制
		if (newAngle >= 45 && newAngle <= 135 || newAngle >= -135 && newAngle <= -45) {
			if (newAngle >= 45 && newAngle <= 135) angleRadian = 90 - newAngle;
			if (newAngle >= -135 && newAngle <= -45) angleRadian = -90 - newAngle;
			if (angleRadian >= 45) angleRadian = 45;
			if (angleRadian <= -45) angleRadian = -45;
			m_Data->panCursorLineRoateAngleRadian = angleRadian * M_PI / 180.0;

			this->Prop3DTransform(this->InteractionProp, obj_center, 1, rotate, scale);
		}

		delete[] rotate[0];
		delete[] rotate;

		if (this->AutoAdjustCameraClippingRange)
		{
			this->CurrentRenderer->ResetCameraClippingRange();
		}

		MsgCenter::send(MsgCenter::CprUpdatePanCursorLineTranslate);

	}
	// 种植体交互
	else if (this->InteractionProp == this->pImplantCtrLineActor)
	{
		vtkCamera* cam = this->CurrentRenderer->GetActiveCamera();

		// Get the axis to rotate around = vector from eye to origin

		double* obj_center = this->InteractionProp->GetCenter();
		double motion_vector[3];
		double view_point[3];

		if (cam->GetParallelProjection())
		{
			// If parallel projection, want to get the view plane normal...
			cam->ComputeViewPlaneNormal();
			cam->GetViewPlaneNormal(motion_vector);
		}
		else
		{
			// Perspective projection, get vector from eye to center of actor
			// 透视投影
			cam->GetPosition(view_point);
			motion_vector[0] = view_point[0] - obj_center[0];
			motion_vector[1] = view_point[1] - obj_center[1];
			motion_vector[2] = view_point[2] - obj_center[2];
			vtkMath::Normalize(motion_vector);
		}

		double disp_obj_center[3];

		this->ComputeWorldToDisplay(obj_center[0], obj_center[1], obj_center[2], disp_obj_center);

		double newAngle =
			vtkMath::DegreesFromRadians(atan2(rwi->GetEventPosition()[1] - disp_obj_center[1],
				rwi->GetEventPosition()[0] - disp_obj_center[0]));

		double oldAngle =
			vtkMath::DegreesFromRadians(atan2(rwi->GetLastEventPosition()[1] - disp_obj_center[1],
				rwi->GetLastEventPosition()[0] - disp_obj_center[0]));

		double scale[3];
		scale[0] = scale[1] = scale[2] = 1.0;

		double** rotate = new double* [1];
		rotate[0] = new double[4];

		rotate[0][0] = newAngle - oldAngle;
		rotate[0][1] = 1;
		rotate[0][2] = 0;
		rotate[0][3] = 0; // 旋转

		this->Prop3DTransform(this->InteractionProp, obj_center, 1, rotate, scale);

		delete[] rotate[0];
		delete[] rotate;

		if (this->AutoAdjustCameraClippingRange)
		{
			this->CurrentRenderer->ResetCameraClippingRange();
		}

		// 端点更新
		double rotateVector[3] = { 0,0,-1 };
		double wxyz[4] = { newAngle * M_PI / 180, 1,0,0 };
		double rotatedVector[3];
		vtkMath::RotateVectorByWXYZ(rotateVector, wxyz, rotatedVector);
		vtkMath::Normalize(rotatedVector);
		double point0[3];
		double point1[3];
		point0[0] = obj_center[0] + 0.5 * lineLength * rotatedVector[0];
		point0[1] = obj_center[1] + 0.5 * lineLength * rotatedVector[1];
		point0[2] = obj_center[2] + 0.5 * lineLength * rotatedVector[2];

		point1[0] = obj_center[0] - 0.5 * lineLength * rotatedVector[0];
		point1[1] = obj_center[1] - 0.5 * lineLength * rotatedVector[1];
		point1[2] = obj_center[2] - 0.5 * lineLength * rotatedVector[2];


		m_Data->point0[0] = point0[0];
		m_Data->point0[1] = point0[1];
		m_Data->point0[2] = point0[2];
		m_Data->point1[0] = point1[0];
		m_Data->point1[1] = point1[1];
		m_Data->point1[2] = point1[2];

		//m_Data->ImplantPointsSetInPanWin->SetPoint(0, point0);
		//m_Data->ImplantPointsSetInPanWin->SetPoint(1, point1);
		MsgCenter::send(MsgCenter::CprUpdateImplantPosition);

		cout << " m_Data->point0 " << m_Data->point0[0] << " " << m_Data->point0[1] << " " << m_Data->point0[2] << endl;
		cout << " m_Data->point1 " << m_Data->point1[0] << " " << m_Data->point1[1] << " " << m_Data->point1[2] << endl;
	}
	rwi->Render();

}

void InteractorStyleCpr::Pan()
{
	if (this->CurrentRenderer == nullptr || this->InteractionProp == nullptr)
	{
		return;
	}

	vtkRenderWindowInteractor* rwi = this->Interactor;

	// 定位线交互
	if (this->InteractionProp == m_lineActor) {
		// Use initial center as the origin from which to pan
		double* obj_center = this->InteractionProp->GetCenter();
		double disp_obj_center[3], new_pick_point[4];
		double old_pick_point[4], motion_vector[3];

		this->ComputeWorldToDisplay(obj_center[0], obj_center[1], obj_center[2], disp_obj_center);
		this->ComputeDisplayToWorld(rwi->GetEventPosition()[0], rwi->GetEventPosition()[1], disp_obj_center[2], new_pick_point);
		this->ComputeDisplayToWorld(rwi->GetLastEventPosition()[0], rwi->GetLastEventPosition()[1], disp_obj_center[2], old_pick_point);
		// 计算平移向量
		motion_vector[0] = new_pick_point[0] - old_pick_point[0];
		motion_vector[1] = 0; // 固定y坐标不变
		motion_vector[2] = new_pick_point[2] - old_pick_point[2];

		if (this->InteractionProp->GetUserMatrix() != nullptr)
		{
			vtkTransform* t = vtkTransform::New();
			t->PostMultiply();
			t->SetMatrix(this->InteractionProp->GetUserMatrix());
			t->Translate(motion_vector[0], motion_vector[1], motion_vector[2]);
			this->InteractionProp->GetUserMatrix()->DeepCopy(t->GetMatrix());
			t->Delete();
		}
		else
		{
			// 左右边界限制
			// bug修复：如果当前坐标+移动矢量超出边界,会导致此次更新的坐标在边界之外，而导致后续无法移动
			// 所以此时需要将移动矢量置为0
			if (obj_center[2] + motion_vector[2] >= Bounds[4] && obj_center[2] + motion_vector[2] <= Bounds[5])
			{
				this->InteractionProp->AddPosition(motion_vector[0], motion_vector[1], motion_vector[2]);
			}
			else
			{
				motion_vector[2] = 0;
				this->InteractionProp->AddPosition(motion_vector[0], motion_vector[1], motion_vector[2]);
			}
		}

		if (this->AutoAdjustCameraClippingRange)
		{
			this->CurrentRenderer->ResetCameraClippingRange();
		}

		m_Data->CursorLineXCoords = obj_center[2];

		int newIdx = floor(obj_center[2] / m_Data->splineResamplingDistance);

		m_Data->currentPickedPointID = m_Data->spline->GetNumberOfPoints() - newIdx;

		MsgCenter::send(MsgCenter::CprUpdatePanCursorLineTranslate);
	}
	// 种植体交互
	else if (this->InteractionProp == this->pImplantCtrLineActor) {

		double* obj_center = this->InteractionProp->GetCenter();
		double disp_obj_center[3], new_pick_point[4];
		double old_pick_point[4], motion_vector[3];

		this->ComputeWorldToDisplay(obj_center[0], obj_center[1], obj_center[2], disp_obj_center);

		this->ComputeDisplayToWorld(rwi->GetEventPosition()[0], rwi->GetEventPosition()[1], disp_obj_center[2], new_pick_point);

		this->ComputeDisplayToWorld(rwi->GetLastEventPosition()[0], rwi->GetLastEventPosition()[1], disp_obj_center[2], old_pick_point);

		motion_vector[0] = new_pick_point[0] - old_pick_point[0];
		motion_vector[1] = new_pick_point[1] - old_pick_point[1];
		motion_vector[2] = new_pick_point[2] - old_pick_point[2];

		if (this->InteractionProp->GetUserMatrix() != nullptr)
		{
			vtkTransform* t = vtkTransform::New();
			t->PostMultiply();
			t->SetMatrix(this->InteractionProp->GetUserMatrix());
			t->Translate(motion_vector[0], motion_vector[1], motion_vector[2]);
			this->InteractionProp->GetUserMatrix()->DeepCopy(t->GetMatrix());
			t->Delete();
		}
		else
		{
			this->InteractionProp->AddPosition(motion_vector[0], motion_vector[1], motion_vector[2]);
			// 更新端点
			m_Data->point0[0] = m_Data->point0[0] + motion_vector[0];
			m_Data->point0[1] = m_Data->point0[1] + motion_vector[1];
			m_Data->point0[2] = m_Data->point0[2] + motion_vector[2];

			m_Data->point1[0] = m_Data->point1[0] + motion_vector[0];
			m_Data->point1[1] = m_Data->point1[1] + motion_vector[1];
			m_Data->point1[2] = m_Data->point1[2] + motion_vector[2];
			
			cout << m_Data->point0[0] << " " << m_Data->point0[1] << " " << m_Data->point0[2] << endl;
			cout << m_Data->point1[0] << " " << m_Data->point1[1] << " " << m_Data->point1[2] << endl;

			//m_Data->ImplantPointsSetInPanWin->SetPoint(0, m_Data->point0);
			//m_Data->ImplantPointsSetInPanWin->SetPoint(1, m_Data->point1);
			MsgCenter::send(MsgCenter::CprUpdateImplantPosition);

		}
		if (this->AutoAdjustCameraClippingRange)
		{
			this->CurrentRenderer->ResetCameraClippingRange();
		}

	}
	rwi->Render();
}

void InteractorStyleCpr::Init(const double bounds[6], CPRDataStruct* pData)
{
	m_Data = pData;
	for (int i = 0; i < 6; i++)
	{
		this->Bounds[i] = bounds[i];
	}
	this->InteractionPicker->SetTolerance(0.01);
	this->m_ren = this->GetInteractor()->GetRenderWindow()->GetRenderers()->GetFirstRenderer();
	lineSource = vtkSmartPointer<vtkLineSource>::New();
	lineSource->SetPoint1(bounds[0] + 0.1, bounds[2] - 200, bounds[5] / 2);
	lineSource->SetPoint2(bounds[0] + 0.1, bounds[3] + 200, bounds[5] / 2);
	lineSource->Update();
	m_lineActor = vtkSmartPointer<vtkActor>::New();
	this->InteractionPicker->AddPickList(m_lineActor);
	this->InteractionPicker->PickFromListOn();

	vtkSmartPointer<vtkPolyDataMapper> lineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	lineMapper->SetInputConnection(lineSource->GetOutputPort());
	m_lineActor->SetMapper(lineMapper);
	m_lineActor->GetProperty()->SetLineWidth(5);
	m_lineActor->GetProperty()->SetColor(0, 153.0 / 255, 1);
	this->GetInteractor()->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->AddActor(m_lineActor);

	this->GetInteractor()->GetRenderWindow()->Render();
}

void InteractorStyleCpr::SetImplantActors(vtkSmartPointer<vtkActor> implantActors[])
{
	//pImplantPointActor0 = vtkSmartPointer<vtkActor>::New();  // 端点0

	pImplantPointActor0 = implantActors[0];
	this->InteractionPicker->AddPickList(pImplantPointActor0);

	//pImplantPointActor1 = vtkSmartPointer<vtkActor>::New();  // 端点1
	pImplantPointActor1 = implantActors[1];
	this->InteractionPicker->AddPickList(pImplantPointActor1);

	//pImplantCtrLineActor = vtkSmartPointer<vtkActor>::New();  // 线 
	pImplantCtrLineActor = implantActors[2];
	this->InteractionPicker->AddPickList(pImplantCtrLineActor);

	// 获取初始端点并计算线的长度
	double pointX0 = this->pImplantCtrLineActor->GetMapper()->GetInput()->GetPoint(0)[0];
	double pointY0 = this->pImplantCtrLineActor->GetMapper()->GetInput()->GetPoint(0)[1];
	double pointZ0 = this->pImplantCtrLineActor->GetMapper()->GetInput()->GetPoint(0)[2];
	/*this->Point0[0] = pointX0;
	this->Point0[1] = pointY0;
	this->Point0[2] = pointZ0;*/
	m_Data->point0[0] = pointX0;
	m_Data->point0[1] = pointY0;
	m_Data->point0[2] = pointZ0;

	double pointX1 = this->pImplantCtrLineActor->GetMapper()->GetInput()->GetPoint(1)[0];
	double pointY1 = this->pImplantCtrLineActor->GetMapper()->GetInput()->GetPoint(1)[1];
	double pointZ1 = this->pImplantCtrLineActor->GetMapper()->GetInput()->GetPoint(1)[2];
	/*this->Point1[0] = pointX1;
	this->Point1[1] = pointY1;
	this->Point1[2] = pointZ1;*/
	m_Data->point1[0] = pointX1;
	m_Data->point1[1] = pointY1;
	m_Data->point1[2] = pointZ1;
	lineLength = sqrt(vtkMath::Distance2BetweenPoints(m_Data->point0, m_Data->point1));
	m_Data->implantLength = lineLength;

}

//void InteractorStyleCpr::SetImplantActor(vtkSmartPointer<vtkAssembly> implantActor)
//{
//	pImplantCtrLineActor = vtkSmartPointer<vtkAssembly>::New();
//	pImplantCtrLineActor = implantActor; 
//	this->InteractionPicker->AddPickList(pImplantCtrLineActor);
//	this->InteractionPicker->PickFromListOn();
//	this->InteractionPicker->Modified();
//}
