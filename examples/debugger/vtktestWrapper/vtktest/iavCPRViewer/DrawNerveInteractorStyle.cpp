#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRendererCollection.h>
#include <vtkImageActorPointPlacer.h>
#include <vtkImageViewer2.h>
#include <vtkLineSource.h>
#include <vtkProperty.h>
#include <vtkSphereSource.h>

#include "DrawNerveInteractorStyle.h"
#include "MsgCenter.h"
#include "iavGlobalData.h"

DrawNerveInteractorStyle::DrawNerveInteractorStyle()
{
	//m_straightLineActor = vtkSmartPointer<vtkActor>::New();
	//m_straightLineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	m_imagePointPicker = vtkSmartPointer<vtkImageActorPointPlacer>::New();
	//m_FocalPlanePointPlacer = vtkSmartPointer<vtkFocalPlanePointPlacer>::New();
	//m_FocalPlanePointPlacer->SetPointBounds()

	this->PreviousPosition[0] = 0;
	this->PreviousPosition[1] = 0;

	NumberOfClicks = 0;
	ResetPixelDistance = 5;
}

void DrawNerveInteractorStyle::OnLeftButtonDown()
{
	m_IsDrawing = true;
	this->NumberOfClicks++;

	int* eventPos = this->Interactor->GetEventPosition();
	std::cout << "EventPos: " << eventPos[0] << " " << eventPos[1]  << std::endl;

	// For double-click event
	int xdist = eventPos[0] - this->PreviousPosition[0];
	int ydist = eventPos[1] - this->PreviousPosition[1];
	this->PreviousPosition[0] = eventPos[0];
	this->PreviousPosition[1] = eventPos[1];
	int moveDistance = (int)sqrt((double)(xdist * xdist + ydist * ydist));

	// For points drawing
	double display[2];
	display[0] = eventPos[0];
	display[1] = eventPos[1];

	double pickedImagePos[3];
	double orient[9];

	m_imagePointPicker->ComputeWorldPosition(m_panWindow->GetRenderWindow()->GetRenderers()->GetFirstRenderer(), display, pickedImagePos, orient);
	std::cout << "Picked value (image world): " << pickedImagePos[0] << " " << pickedImagePos[1] << " " << pickedImagePos[2] << std::endl;

	int isInImage = m_imagePointPicker->ValidateWorldPosition(pickedImagePos);

	// Reset numClicks - If mouse moved further than resetPixelDistance.
	if (moveDistance > this->ResetPixelDistance)
	{
		this->NumberOfClicks = 1;

		if (isInImage == 0)
		{
			std::cout << "Current pos is not in the image!" << std::endl;
		}
		else if (isInImage == 1)
		{
			//pickedImagePos[2] = std::ceil(pickedImagePos[2]);
			//pickedImagePos[2] += 0.01;

			if (m_panWindow->GetCPRData()->numOfTubes == 0)
			{
				m_panWindow->GetCPRData()->NervePointsSetInPanWin->InsertNextPoint(pickedImagePos);
				std::cout << "NervePointsSet size: " << m_panWindow->GetCPRData()->NervePointsSetInPanWin->GetNumberOfPoints() << std::endl;
			}
			else if (m_panWindow->GetCPRData()->numOfTubes == 1)
			{
				m_panWindow->GetCPRData()->NervePointsSetInPanWin2->InsertNextPoint(pickedImagePos);
				std::cout << "NervePointsSet size: " << m_panWindow->GetCPRData()->NervePointsSetInPanWin2->GetNumberOfPoints() << std::endl;
			}

			//convert pan coords to vr coords here
			double coordsInVrWin[3];
			TransformCoordsPanToVR(pickedImagePos, coordsInVrWin);

			if (m_panWindow->GetCPRData()->numOfTubes == 0)
			{
				m_panWindow->GetCPRData()->NervePointsSetInVrWin->InsertNextPoint(coordsInVrWin);
			}
			else if (m_panWindow->GetCPRData()->numOfTubes == 1)
			{
				m_panWindow->GetCPRData()->NervePointsSetInVrWin2->InsertNextPoint(coordsInVrWin);
			}

			std::cout << "coordsInVrWin: " << coordsInVrWin[0] << " " << coordsInVrWin[1] << " " << coordsInVrWin[2] << std::endl;

			//m_panWindow->GetCPRData()->NervePointsSetInVrWin->InsertNextPoint();

			std::cout << "Current pos is in the image!" << std::endl;
	
			int numOfNervePoints = m_panWindow->GetCPRData()->NervePointsSetInPanWin->GetNumberOfPoints();

			// maximum number of tubes is 2 
			if (m_IsDrawing && numOfNervePoints >= 1 && m_panWindow->GetCPRData()->numOfTubes < 2)
			{
				//Draw point here (panWindow)
				vtkNew<vtkSphereSource> pointSource;
				pointSource->SetCenter(pickedImagePos);
				//pointSource->SetNumberOfPoints(1);
				pointSource->SetRadius(1.0);
				pointSource->Update();

				// Create a mapper and actor.
				vtkNew<vtkPolyDataMapper> mapper;
				mapper->SetInputConnection(pointSource->GetOutputPort());

				vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
				actor->SetMapper(mapper);
				actor->GetProperty()->SetColor(1, 0.5, 0);
				actor->GetProperty()->SetPointSize(5);

				if (m_panWindow->GetCPRData()->numOfTubes == 0)
				{
					m_panWindow->GetCPRData()->nervePointsActorVec1.push_back(actor);
				}
				else if (m_panWindow->GetCPRData()->numOfTubes == 1)
				{
					m_panWindow->GetCPRData()->nervePointsActorVec2.push_back(actor);
				}

				this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->AddActor(actor);
				this->Interactor->GetRenderWindow()->Render();

				if (numOfNervePoints > 1)
				{
					//Draw Nerve here (panWindow, transwindow)
					MsgCenter::send(MsgCenter::CprUpdataNerveTube);
				}
			}
		}
	}

	if (this->Interactor->GetRepeatCount())
	{
		m_panWindow->GetCPRData()->numOfTubes += 1;

		std::cout << "double clicker" << std::endl;
		m_IsDrawing = false;
		this->m_panWindow->GetResliceImageViewer()->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->RemoveActor(m_straightLineActor);
		MsgCenter::send(MsgCenter::CprUpdataNerveTube);
	}

#if 0
	// double clide event - finish drawing
	if (this->NumberOfClicks == 2)
	{
		std::cout << "Double clicked." << std::endl;
		this->NumberOfClicks = 0;

		m_IsDrawing = false;

		// remove straight line and refresh
		this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->RemoveActor(m_straightLineActor);
		this->Interactor->GetRenderWindow()->Render();

		m_transverseWindow->ComputeSplineFrenetArray();
		m_transverseWindow->DrawMarkLineOnTransImage();
		m_transverseWindow->DrawAdjoiningMarkLineOnTransImage();
		m_transverseWindow->DrawThickSplineOnTransImage();

		//ComputeStraightVolumeNerve();

		// Quit drawing mode and change back to default interactor style
		m_transverseWindow->SetDefaultInteractor();
	}
#endif
	// Forward events.
	vtkInteractorStyleTrackballCamera::OnLeftButtonDown();

}

void DrawNerveInteractorStyle::OnMouseMove()
{
	int numOfNervePoints;
	if (m_panWindow->GetCPRData()->numOfTubes == 0)
	{
		numOfNervePoints = m_panWindow->GetCPRData()->NervePointsSetInPanWin->GetNumberOfPoints();
	}
	else if (m_panWindow->GetCPRData()->numOfTubes == 1)
	{
		numOfNervePoints = m_panWindow->GetCPRData()->NervePointsSetInPanWin2->GetNumberOfPoints();
	}
	else 
	{
		numOfNervePoints = 0;
	}

	if (m_IsDrawing && numOfNervePoints >= 1)
	{
		//iavIteractorStyleNervedrawing::OnMouseMove();
		//m_transverseWindow->DrawStraightLineOnAxialImage();
		// Get last point
		double lastPt[3];
		//m_panWindow->GetCPRData()->NervePointsSetInPanWin->GetPoint(m_panWindow->GetCPRData()->NervePointsSetInPanWin->GetNumberOfPoints() - 1, lastPt);

		if (m_panWindow->GetCPRData()->numOfTubes == 0)
		{
			m_panWindow->GetCPRData()->NervePointsSetInPanWin->GetPoint(m_panWindow->GetCPRData()->NervePointsSetInPanWin->GetNumberOfPoints() - 1, lastPt);
		}
		else if (m_panWindow->GetCPRData()->numOfTubes == 1)
		{
			m_panWindow->GetCPRData()->NervePointsSetInPanWin2->GetPoint(m_panWindow->GetCPRData()->NervePointsSetInPanWin2->GetNumberOfPoints() - 1, lastPt);
		}

		// Get event world position
		int* eventPos = this->Interactor->GetEventPosition();
		//std::cout << "eventPos: " << eventPos[0] << " " << eventPos[1] << " " << eventPos[2] << std::endl;

		double display[2];
		display[0] = eventPos[0];
		display[1] = eventPos[1];

		double pickedImagePos[3];
		double orient[9];
		m_imagePointPicker->ComputeWorldPosition(m_panWindow->GetResliceImageViewer()->GetRenderer(), display, pickedImagePos, orient);

		pickedImagePos[0] += 0.01;
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
			this->m_panWindow->GetResliceImageViewer()->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->AddActor(m_straightLineActor);
		}

		m_straightLineMapper->SetInputData(line->GetOutput());
		m_straightLineMapper->Update();

		this->m_panWindow->GetResliceImageViewer()->GetRenderWindow()->Render();
	}
}

void DrawNerveInteractorStyle::OnRightButtonUp()
{
	m_IsDrawing = false;

	this->m_panWindow->GetResliceImageViewer()->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->RemoveActor(m_straightLineActor);
	//m_transverseWindow->SetDefaultInteractor();

}

void DrawNerveInteractorStyle::OnMouseWheelForward()
{
	int sliceNum = this->m_panWindow->GetResliceImageViewer()->GetSlice();
	this->m_panWindow->SetSlice(sliceNum + 1);
	//m_transverseWindow->view2->Render();
}

void DrawNerveInteractorStyle::OnMouseWheelBackward()
{
	int sliceNum = this->m_panWindow->GetResliceImageViewer()->GetSlice();
	this->m_panWindow->SetSlice(sliceNum - 1);
	//m_transverseWindow->view2->Render();
}

void DrawNerveInteractorStyle::SetVolumeRenderingWindowClass(CPRVolumeRenderingWindow* window)
{
	m_VolumeRenderingWindow = window;
}

void DrawNerveInteractorStyle::SetPanWindowClass(CPRPanWindow* window)
{
	m_panWindow = window;

	m_imagePointPicker->SetImageActor(window->GetResliceImageViewer()->GetImageActor());
	m_imagePointPicker->Modified();
}

void DrawNerveInteractorStyle::TransformCoordsPanToVR(double panCoords[3], double vrCoords[3])
{
	std::cout << "TransformCoordsPanToVR" << std::endl;
	std::cout << "Image Bounds: " << m_panWindow->GetResliceImageViewer()->GetInput()->GetBounds()[1]
		<< " " << m_panWindow->GetResliceImageViewer()->GetInput()->GetBounds()[3]
		<< " " << m_panWindow->GetResliceImageViewer()->GetInput()->GetBounds()[5] << std::endl;
	std::cout << "Image Extents: " << m_panWindow->GetResliceImageViewer()->GetInput()->GetExtent()[1]
		<< " " << m_panWindow->GetResliceImageViewer()->GetInput()->GetExtent()[3]
		<< " " << m_panWindow->GetResliceImageViewer()->GetInput()->GetExtent()[5] << std::endl;

	double spacing[3];
	iavGlobalData::getImageData()->GetSpacing(spacing);

	double imgIJK[3];
	m_panWindow->GetResliceImageViewer()->GetInput()->TransformPhysicalPointToContinuousIndex(panCoords, imgIJK);

	std::cout << "imgIJK: " << imgIJK[0] << " " << imgIJK[1] << " " << imgIJK[2] << std::endl;

	int xIdx = floor(imgIJK[0]);
	int yIdx = floor(imgIJK[1]);
	int zIdx = m_panWindow->GetCPRData()->spline->GetNumberOfPoints() - 1 - floor(imgIJK[2]);
	std::cout << "final yIdx: " << yIdx << std::endl;
	std::cout << "final zIdx: " << zIdx << std::endl;


	double xfactor = 0;
	xfactor = fabs(xIdx - ((m_panWindow->GetResliceImageViewer()->GetInput()->GetExtent()[1] + 1) / 2));

	double yfactor = 0;
	yfactor = fabs(yIdx - ((m_panWindow->GetResliceImageViewer()->GetInput()->GetExtent()[3] + 1) / 2));

	double correspondingSplinePt[3];
	m_panWindow->GetCPRData()->spline->GetPoint(zIdx, correspondingSplinePt);

	// pan slice 
	if (xIdx > (m_panWindow->GetResliceImageViewer()->GetInput()->GetExtent()[1] + 1) / 2)
	{
		// outward 
		double* normal;
		normal = m_panWindow->GetCPRData()->ctrlineNormal->GetTuple3(zIdx);

		vrCoords[0] = correspondingSplinePt[0] + xfactor * spacing[0] * normal[0];
		vrCoords[1] = correspondingSplinePt[1] + xfactor * spacing[0] * normal[1];
		vrCoords[2] = correspondingSplinePt[2] + xfactor * spacing[0] * normal[2];
	}
	else
	{
		// inward
		double* normal;
		normal = m_panWindow->GetCPRData()->ctrlineNormal->GetTuple3(zIdx);
		normal[0] = -1 * normal[0];
		normal[1] = -1 * normal[1];
		normal[2] = -1 * normal[2];

		vrCoords[0] = correspondingSplinePt[0] + xfactor * spacing[0] * normal[0];
		vrCoords[1] = correspondingSplinePt[1] + xfactor * spacing[0] * normal[1];
		vrCoords[2] = correspondingSplinePt[2] + xfactor * spacing[0] * normal[2];
	}

	// pan height
	if (yIdx > (m_panWindow->GetResliceImageViewer()->GetInput()->GetExtent()[3] + 1) / 2)
	{
		// above spline 
		double binormal[3] = { 0, 0, -1 };

		vrCoords[0] = vrCoords[0] + yfactor * spacing[1] * binormal[0];
		vrCoords[1] = vrCoords[1] + yfactor * spacing[1] * binormal[1];
		vrCoords[2] = vrCoords[2] + yfactor * spacing[1] * binormal[2];
	}
	else
	{
		// below spline
		double binormal[3] = { 0, 0, 1 };

		vrCoords[0] = vrCoords[0] + yfactor * spacing[1] * binormal[0];
		vrCoords[1] = vrCoords[1] + yfactor * spacing[1] * binormal[1];
		vrCoords[2] = vrCoords[2] + yfactor * spacing[1] * binormal[2];
	}
}
