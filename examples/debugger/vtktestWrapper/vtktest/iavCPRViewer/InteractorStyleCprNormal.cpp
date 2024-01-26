#include "InteractorStyleCprNormal.h"
#include <vtkPoints.h>
#include <vtkPolyDataMapper2D.h>

#include "MsgCenter.h"

void InteractorStyleCprNormal::OnLeftButtonDown(void)
{
	isLeftButtonDown = true;
	// get the current position of the mouse
	this->GetInteractor()->GetEventPosition(currPos);
	vtkRenderer* currRenderer = this->GetInteractor()->FindPokedRenderer(currPos[0], currPos[1]);
	if (currRenderer == nullptr) {
		return;
	}

	// get the 3 points of Longitudinalline
	double pointsPos[3][3] = { 0 };
	vtkPoints* linePoints = cursorPolyData->GetPoints();

	for (unsigned int i = 0; i < 3; i++) {
		vtkIdType n = i;
		double* point = linePoints->GetPoint(n);
		pointsPos[i][0] = point[0]; pointsPos[i][1] = point[1]; pointsPos[i][2] = point[2];
	}
	double displayLinePoint1[2][3]{ 0.0 };
	currRenderer->SetWorldPoint(pointsPos[1][0], pointsPos[1][1], pointsPos[1][2], 1);
	currRenderer->WorldToDisplay();
	double* pDisplayPoint = currRenderer->GetDisplayPoint();
	displayLinePoint1[0][0] = pDisplayPoint[0];
	displayLinePoint1[0][1] = pDisplayPoint[1];
	displayLinePoint1[0][2] = 0;
	currRenderer->SetWorldPoint(pointsPos[2][0], pointsPos[2][1], pointsPos[2][2], 1);
	currRenderer->WorldToDisplay();
	pDisplayPoint = currRenderer->GetDisplayPoint();
	displayLinePoint1[1][0] = pDisplayPoint[0];
	displayLinePoint1[1][1] = pDisplayPoint[1];
	displayLinePoint1[1][2] = 0;

	double originPos[3]{ currPos[0],currPos[1],0.0 };
	// 鼠标按下的点到竖直线的距离
	moveAxis = false;  // true:move  
	rotateAxis = false; // true:rotate
	double distanceToLine = distancePointToLine(originPos, displayLinePoint1[0], displayLinePoint1[1]);
	double pa[3] = { pointsPos[0][0], pointsPos[0][1], pointsPos[0][2] };  // 这里为原点
	//将世界坐标系中的点pointsPos[0][0], pointsPos[0][1], 0转化为屏幕坐标系中的点pa
	//vtkInteractorObserver::ComputeWorldToDisplay(currRenderer, pointsPos[0][0], pointsPos[0][1], pointsPos[0][2], pa); //pointsPos[0][0], pointsPos[0][1]表示十字线的原点
	
	double display[2];
	display[0] = currPos[0];
	display[1] = currPos[1];
	double pickedImagePos[3];
	double orient[9];
	m_imagePointPicker->ComputeWorldPosition(currRenderer, display, pickedImagePos, orient);
	std::cout << "Picked value (image world): " << pickedImagePos[0] << " " << pickedImagePos[1] << " " << pickedImagePos[2] << std::endl;

	//pa[2] = 0;//z轴坐标置为零
	double pb[3] = { pickedImagePos[0],pickedImagePos[1],pickedImagePos[2]};  // 鼠标当前坐标点
	cout << "鼠标坐标：" << pb[0] << " " << pb[1] << " " << pb[2] << endl;
	cout << "线 坐 标：" << pa[0] << " " << pa[1] << " " << pa[2] << endl;
	double distanceToOrigin = fabs(pb[1]-pa[1]);  // 鼠标到原点的距离
	int* renderWindowSize = currRenderer->GetRenderWindow()->GetSize();
	int renderWindowHeight = renderWindowSize[1];
	int threshold1 = 3;  // threshold for distanceToLine
	int threshold2 = renderWindowHeight * 0.5 * 0.5 * 0.5 * 0.5 * 0.75 ; // threshold for distanceToOrigin
	cout << "windowsize: " << renderWindowHeight << " " << threshold2 << endl;
	cout << "distanceToOrigin: " << distanceToOrigin << endl;
	double disZ = pb[2] - pa[2];
	cout << "disZ: " << disZ << endl;
	if (disZ <= 1 && distanceToOrigin <= threshold2) {
		cout << "moveAxis: " << moveAxis << endl;
		moveAxis = true;  // 平移
		cout << "moveAxis: " << moveAxis << endl;
	}
	else if (disZ <= 1 && distanceToOrigin > threshold2) {
		cout << "rotateAxis: " << rotateAxis << endl;
		rotateAxis = true;  // 旋转
		cout << "rotateAxis: " << rotateAxis << endl;
	}

	//if (distanceToLine <= threshold1 && distanceToOrigin <= threshold2) {
	//	moveAxis = true;  // 平移
	//}
	//if (distanceToLine <= threshold1 && distanceToOrigin > threshold2) {
	//	rotateAxis = true;  // 旋转
	//}
	vtkInteractorStyleTrackballActor::OnLeftButtonDown();
}

void InteractorStyleCprNormal::OnLeftButtonUp(void)
{
	isLeftButtonDown = false;
	vtkInteractorStyleTrackballActor::OnLeftButtonUp();
}

void InteractorStyleCprNormal::OnMouseMove(void)
{
	this->Interactor->GetRenderWindow()->SetCurrentCursor(VTK_CURSOR_DEFAULT);
	//this->Interactor->GetRenderWindow()->SetCurrentCursor(VTK_CURSOR_HAND);

	this->GetInteractor()->GetEventPosition(currPos);
	const double currDisplayPos[3] = { currPos[0], currPos[1], 0 };
	vtkRenderer* currRenderer = this->GetInteractor()->FindPokedRenderer(currPos[0], currPos[1]);
	currRenderer->SetDisplayPoint(currDisplayPos);
	currRenderer->DisplayToWorld();
	currRenderer->GetWorldPoint(m_panWindow->GetCPRData()->MoveRotateAxisPoint);
	//MoveRotateAxisPoint = currRenderer->GetWorldPoint();

	vtkSmartPointer<vtkPoints> initialPoints = cursorPolyData->GetPoints();
	double p0[3] = { initialPoints->GetPoint(0)[0],initialPoints->GetPoint(0)[1],initialPoints->GetPoint(0)[2] };
	double p1[3] = { initialPoints->GetPoint(1)[0],initialPoints->GetPoint(1)[1],initialPoints->GetPoint(1)[2] };
	double p2[3] = { initialPoints->GetPoint(2)[0],initialPoints->GetPoint(2)[1],initialPoints->GetPoint(2)[2] };
	double bounds[6];
	imageActor->GetBounds(bounds);
	boundx = bounds[1];

	if (isLeftButtonDown == true)
	{
		if (moveAxis == true)
		{
			this->Interactor->GetRenderWindow()->SetCurrentCursor(VTK_CURSOR_DEFAULT);

			int newIdx = floor(cursorPolyData->GetPoint(0)[2] / m_panWindow->GetCPRData()->splineResamplingDistance);

			// z aixs inversed
			m_panWindow->GetCPRData()->currentPickedPointID = m_panWindow->GetCPRData()->spline->GetNumberOfPoints() - newIdx;
			MsgCenter::send(MsgCenter::CprUpdatePanCursorLineTranslate);

		}
		else if (rotateAxis == true)
		{
			this->Interactor->GetRenderWindow()->SetCurrentCursor(VTK_CURSOR_HAND);
			MsgCenter::send(MsgCenter::CprUpdatePanCursorLineRotate);

		}
	}
}

double InteractorStyleCprNormal::distancePointToLine(const double origin[3], const double linePoint1[3], const double linePoint2[3]) {
	// 根据余弦定理求点到直线的距离
	double b = std::sqrt(std::pow(origin[0] - linePoint1[0], 2) + std::pow(origin[1] - linePoint1[1], 2) + std::pow(origin[2] - linePoint1[2], 2));
	double c = std::sqrt(std::pow(linePoint2[0] - linePoint1[0], 2) + std::pow(linePoint2[1] - linePoint1[1], 2) + std::pow(linePoint2[2] - linePoint1[2], 2));
	double a = std::sqrt(std::pow(origin[0] - linePoint2[0], 2) + std::pow(origin[1] - linePoint2[1], 2) + std::pow(origin[2] - linePoint2[2], 2));
	double value = (b * b + c * c - a * a) / (2 * b * c);
	double A = std::acos(value);
	double distance = std::abs(b * std::sin(A));
	return distance;
}

void InteractorStyleCprNormal::SetLinePolyData(vtkSmartPointer<vtkPolyData> polydata)
{
	cursorPolyData = polydata;
}

void InteractorStyleCprNormal::SetLineActor(vtkSmartPointer<vtkActor2D> lineActor)
{
	cursorActor = lineActor;
}

void InteractorStyleCprNormal::SetImageActor(vtkSmartPointer<vtkImageActor> imageactor)
{
	imageActor = imageactor;
	m_imagePointPicker = vtkSmartPointer<vtkImageActorPointPlacer>::New();
	m_imagePointPicker->SetImageActor(imageActor);

}

void InteractorStyleCprNormal::SetPanWindowClass(CPRPanWindow* window)
{
	m_panWindow = window;
}

//void InteractorStyleCprNormal::SetRenSliceViewer(vtkSmartPointer<vtkRenderer> ren[], vtkSmartPointer<vtkImageReslice> slice[], vtkSmartPointer<vtkResliceImageViewer> viewer[])
//{
//	for (int i = 0; i < 5; i++) {
//		resliceRen[i] = ren[i];
//		reslice[i] = slice[i];
//		imageViewer[i] = viewer[i];
//	}
//}

double InteractorStyleCprNormal::GetRotateRadian()
{
	return angleRadian;
}

double InteractorStyleCprNormal::GetImageResliceAxis()
{
	double bounds[6];
	imageActor->GetBounds(bounds);
	boundx = bounds[1];
	imageResliceAxes = boundx + cursorPolyData->GetPoint(0)[0];
	return imageResliceAxes;
}


void InteractorStyleCprNormal::ResliceUpdate(double XToZ, double origin0)
{
	for (int i = 0; i < 5; i++) {
		resliceMatrix1 = vtkSmartPointer<vtkMatrix4x4>::New();
		resliceMatrix1->Identity();
		resliceMatrix1->SetElement(0, 0, 1);  //X
		resliceMatrix1->SetElement(1, 0, 0);
		resliceMatrix1->SetElement(2, 0, 0);

		resliceMatrix1->SetElement(0, 1, 0);  //Y
		resliceMatrix1->SetElement(1, 1, cos(angleRadian));
		resliceMatrix1->SetElement(2, 1, sin(angleRadian));

		resliceMatrix1->SetElement(0, 2, 0);  //Z
		resliceMatrix1->SetElement(1, 2, -sin(angleRadian));
		resliceMatrix1->SetElement(2, 2, cos(angleRadian));

		resliceMatrix1->SetElement(0, 3, reslice[i]->GetResliceAxes()->GetElement(0, 3));  //origin
		resliceMatrix1->SetElement(1, 3, reslice[i]->GetResliceAxes()->GetElement(1, 3));

		switch (i) {
		case 0:
			resliceMatrix1->SetElement(2, 3, origin0 + XToZ);
			break;

		case 1:
			resliceMatrix1->SetElement(2, 3, origin0 + XToZ - 1);
			break;

		case 2:
			resliceMatrix1->SetElement(2, 3, origin0 + XToZ - 2);
			break;

		case 3:
			resliceMatrix1->SetElement(2, 3, origin0 + XToZ + 1);
			break;

		case 4:
			resliceMatrix1->SetElement(2, 3, origin0 + XToZ + 2);
			break;
		}
		reslice[i]->SetResliceAxes(resliceMatrix1);
		reslice[i]->SetInterpolationModeToLinear();
		reslice[i]->Update();
	}
}