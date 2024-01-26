#pragma once
#include <vtkSmartPointer.h>
#include <vtkImageData.h>
#include <vtkPolyData.h>
#include <vtkPolyLine.h>
#include <vtkPoints.h>
#include <vtkRenderer.h>


class iadAlgorithmCPR
{

public:
	//iadAlgorithmCPR();
	//~iadAlgorithmCPR();

	bool ResamplePoints(vtkPoints* originalPoints, vtkPoints* sampledPoints, double samplingDistance, bool closedCurve);
	vtkSmartPointer<vtkImageData> computeStraightVolumeCPR(vtkPolyData* curve, vtkImageData* originalDcm);

	vtkSmartPointer<vtkVolume> dentalVR(vtkImageData* dcm);

	double GetCurveLength(vtkPoints* curvePoints, bool closedCurve, vtkIdType startCurvePointIndex, vtkIdType numberOfCurvePoints);

private:

	//Helper functions
	bool GetPositionAndClosestPointIndexAlongCurve(double foundCurvePosition[3], vtkIdType& foundClosestPointIndex, vtkIdType startCurvePointId, double distanceFromStartPoint, vtkPoints* curvePoints, bool closedCurve);

	void convertPointsToPolyline(vtkPoints* inputPoints, vtkPolyData* outputPolyline);

};