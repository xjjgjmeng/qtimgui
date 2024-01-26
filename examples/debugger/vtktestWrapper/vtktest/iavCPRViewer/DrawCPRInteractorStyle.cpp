#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRendererCollection.h>
#include <vtkImageActorPointPlacer.h>
#include <vtkImageViewer2.h>
#include <vtkLineSource.h>
#include <vtkProperty.h>
#include <vtkInteractorStyleImage.h>

#include "DrawCPRInteractorStyle.h"
#include "MsgCenter.h"

DrawCPRInteractorStyle::DrawCPRInteractorStyle()
{
	ResetPixelDistance = 5;
}

void DrawCPRInteractorStyle::OnLeftButtonDown()
{
	m_IsDrawing = true;

	if (this->Interactor->GetRepeatCount())
	{
		std::cout << "double clicked" << std::endl;
		m_IsDrawing = false;

		// remove straight line and refresh
		this->m_transverseWindow->view2->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->RemoveActor(m_straightLineActor);
		this->m_transverseWindow->view2->GetRenderWindow()->Render();

		vtkNew<vtkPoints> splinePoints;
		splinePoints->DeepCopy(m_transverseWindow->m_pCprData->spline->GetPoints());

		int numOfPoints = splinePoints->GetNumberOfPoints();
		double centerPoint[3];

		if (numOfPoints % 2 != 0)
		{
			splinePoints->GetPoint((numOfPoints + 1) / 2 - 1, centerPoint);
			m_transverseWindow->m_pCprData->currentPickedPointID = (numOfPoints + 1) / 2 - 1;
		}
		else
		{
			splinePoints->GetPoint(numOfPoints / 2, centerPoint);
			m_transverseWindow->m_pCprData->currentPickedPointID = numOfPoints / 2;
		}


		//this->m_RendererWindowVR->Render();
		m_transverseWindow->ComputeSplineFrenetArray();
		m_transverseWindow->DrawMarkLineOnTransImage();
		m_transverseWindow->DrawAdjoiningMarkLineOnTransImage();
		m_transverseWindow->DrawThickSplineOnTransImage();

		MsgCenter::send(MsgCenter::CprSplineFinishEditing);

		// Quit drawing mode and change back to default interactor style
		this->Interactor->SetInteractorStyle(vtkSmartPointer<vtkInteractorStyleImage>::New());
		return;
	}

	int* eventPos = this->Interactor->GetEventPosition();
	std::cout << "EventPos: " << eventPos[0] << " " << eventPos[1]  << std::endl;

	// For points drawing
	double display[2];
	display[0] = eventPos[0];
	display[1] = eventPos[1];

	double pickedImagePos[3];
	double orient[9];

	m_imagePointPicker->ComputeWorldPosition(m_transverseWindow->view2->GetRenderer(), display, pickedImagePos, orient);
	std::cout << "Picked value (image world): " << pickedImagePos[0] << " " << pickedImagePos[1] << " " << pickedImagePos[2] << std::endl;

	int isInImage = m_imagePointPicker->ValidateWorldPosition(pickedImagePos);

	{
		if (isInImage == 0)
		{
			std::cout << "Current pos is not in the image!" << std::endl;
		}
		else if (isInImage == 1)
		{
			pickedImagePos[2] += 0.01;
			m_transverseWindow->m_pCprData->splinePointsSet->InsertNextPoint(pickedImagePos);

			std::cout << "Current pos is in the image!" << std::endl;
			std::cout << "m_SplinePoints size: " << m_transverseWindow->m_pCprData->splinePointsSet->GetNumberOfPoints() << std::endl;

			int numOfSplineContorPoints = m_transverseWindow->m_pCprData->splinePointsSet->GetNumberOfPoints();

			if (m_IsDrawing && numOfSplineContorPoints >= 1)
			{
				m_transverseWindow->DrawSplineOnTransImage();
				m_transverseWindow->ComputeSplineFrenetArray();

				MsgCenter::send(MsgCenter::CprUpdateSpline);

				if (numOfSplineContorPoints > 1)
				{
					int numOfPoints = m_transverseWindow->m_pCprData->spline->GetNumberOfPoints();
					double centerPoint[3];

					if (numOfPoints % 2 != 0)
					{
						m_transverseWindow->m_pCprData->spline->GetPoints()->GetPoint((numOfPoints + 1) / 2 - 1, centerPoint);
						m_transverseWindow->m_pCprData->currentPickedPointID = (numOfPoints + 1) / 2 - 1;
					}
					else
					{
						m_transverseWindow->m_pCprData->spline->GetPoints()->GetPoint(numOfPoints / 2, centerPoint);
						m_transverseWindow->m_pCprData->currentPickedPointID = numOfPoints / 2;
					}

					MsgCenter::send(MsgCenter::CprUpdateCurvePlane);
				}
			}
		}
	}
}

void DrawCPRInteractorStyle::OnMouseMove()
{
	int numOfSplineContorPoints = m_transverseWindow->m_pCprData->splinePointsSet->GetNumberOfPoints();
	if (m_IsDrawing && numOfSplineContorPoints >= 1)
	{
		//iavIteractorStyleCPRdrawing::OnMouseMove();
		//m_transverseWindow->DrawStraightLineOnAxialImage();
		// Get last point
		double lastPt[3];
		m_transverseWindow->m_pCprData->splinePointsSet->GetPoint(m_transverseWindow->m_pCprData->splinePointsSet->GetNumberOfPoints() - 1, lastPt);

		// Get event world position
		int* eventPos = this->Interactor->GetEventPosition();
		//std::cout << "eventPos: " << eventPos[0] << " " << eventPos[1] << " " << eventPos[2] << std::endl;

		//this->Interactor->GetPicker()->Pick(eventPos[0], eventPos[1], 0, // always zero.
		//								this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer());

		//double picked[3];
		//this->Interactor->GetPicker()->GetPickPosition(picked);
		////std::cout << "Picked value (world): " << picked[0] << " " << picked[1] << " " << picked[2] << std::endl;

		//picked[2] = 45;

		double display[2];
		display[0] = eventPos[0];
		display[1] = eventPos[1];

		double pickedImagePos[3];
		double orient[9];
		m_imagePointPicker->ComputeWorldPosition(m_transverseWindow->view2->GetRenderer(), display, pickedImagePos, orient);

		pickedImagePos[2] += 0.01;
		vtkSmartPointer<vtkLineSource> line = vtkSmartPointer<vtkLineSource>::New();
		line->SetPoint1(lastPt);
		line->SetPoint2(pickedImagePos);
		line->Update();
		//std::cout << "lastPt: " << lastPt[0] << " " << lastPt[1] << " " << lastPt[2] << std::endl;
		//std::cout << "pickedImagePos: " << pickedImagePos[0] << " " << pickedImagePos[1] << " " << pickedImagePos[2] << std::endl;

		if (!this->m_straightLineMapper.Get())
		{
			m_straightLineActor = vtkSmartPointer<vtkActor>::New();
			m_straightLineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
			m_straightLineActor->SetMapper(m_straightLineMapper);
			m_straightLineActor->GetProperty()->SetColor(1, 1, 1);
			m_straightLineActor->GetProperty()->SetLineWidth(3);
			m_straightLineActor->Modified();
			this->m_transverseWindow->view2->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->AddActor(m_straightLineActor);
		}

		m_straightLineMapper->SetInputData(line->GetOutput());
		m_straightLineMapper->Update();

		this->m_transverseWindow->view2->GetRenderWindow()->Render();
	}
}

void DrawCPRInteractorStyle::OnMouseWheelForward()
{
	int sliceNum = m_transverseWindow->view2->GetSlice();
	m_transverseWindow->SetSlice(sliceNum + 1);
	//m_transverseWindow->view2->Render();
}

void DrawCPRInteractorStyle::OnMouseWheelBackward()
{
	int sliceNum = m_transverseWindow->view2->GetSlice();
	m_transverseWindow->SetSlice(sliceNum - 1);
	//m_transverseWindow->view2->Render();
}

void DrawCPRInteractorStyle::SetTransWindowClass(CPRAxialWindow* window) 
{
	m_transverseWindow = window;
	m_imagePointPicker = vtkSmartPointer<vtkImageActorPointPlacer>::New();
	m_imagePointPicker->SetImageActor(window->view2->GetImageActor());
}

void DrawCPRInteractorStyle::SetVolumeRenderingWindowClass(CPRVolumeRenderingWindow* window)
{
	m_VolumeRenderingWindow = window;
}
