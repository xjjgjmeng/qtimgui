#include <vtkDoubleArray.h>
#include <vtkPointData.h>
#include <vtkImageAppend.h>
#include <vtkMatrix4x4.h>
#include <vtkImageReslice.h>
#include <vtkImageChangeInformation.h>
#include <vtkImageFlip.h>

#include <vtkGPUVolumeRayCastMapper.h>
#include <vtkPiecewiseFunction.h>
#include <vtkColorTransferFunction.h>
#include <vtkVolumeProperty.h>

#include "iadAlgoritmCPR.h"

#include "vtkParallelTransportFrame.h"


bool iadAlgorithmCPR::ResamplePoints(vtkPoints* originalPoints, vtkPoints* sampledPoints, double samplingDistance, bool closedCurve)
{
    if (!originalPoints || !sampledPoints || samplingDistance <= 0)
    {
        vtkGenericWarningMacro("CurvedPlanarReformatLogic::ResamplePoints failed: invalid inputs");

        return false;
    }

    if (originalPoints->GetNumberOfPoints() < 2)
    {
        sampledPoints->DeepCopy(originalPoints);

        return true;
    }

    double distanceFromLastSampledPoint = 0;
    double remainingSegmentLength = 0;
    double previousCurvePoint[3] = { 0.0 };
    originalPoints->GetPoint(0, previousCurvePoint);
    sampledPoints->Reset();
    sampledPoints->InsertNextPoint(previousCurvePoint);

    vtkIdType numberOfOriginalPoints = originalPoints->GetNumberOfPoints();
    bool addClosingSegment = closedCurve; // for closed curves, add a closing segment that connects last and first points
    double* currentCurvePoint = nullptr;
    for (vtkIdType originalPointIndex = 0; originalPointIndex < numberOfOriginalPoints || addClosingSegment; originalPointIndex++)
    {
        if (originalPointIndex >= numberOfOriginalPoints)
        {
            // this is the closing segment
            addClosingSegment = false;
            currentCurvePoint = originalPoints->GetPoint(0);
        }
        else
        {
            currentCurvePoint = originalPoints->GetPoint(originalPointIndex);
        }

        double segmentLength = sqrt(vtkMath::Distance2BetweenPoints(currentCurvePoint, previousCurvePoint));
        if (segmentLength <= 0.0)
        {
            continue;
        }
        remainingSegmentLength = distanceFromLastSampledPoint + segmentLength;
        if (remainingSegmentLength >= samplingDistance)
        {
            double segmentDirectionVector[3] =
            {
                (currentCurvePoint[0] - previousCurvePoint[0]) / segmentLength,
                (currentCurvePoint[1] - previousCurvePoint[1]) / segmentLength,
                (currentCurvePoint[2] - previousCurvePoint[2]) / segmentLength
            };
            // distance of new sampled point from previous curve point
            double distanceFromLastInterpolatedPoint = samplingDistance - distanceFromLastSampledPoint;
            while (remainingSegmentLength >= samplingDistance)
            {
                double newSampledPoint[3] =
                {
                    previousCurvePoint[0] + segmentDirectionVector[0] * distanceFromLastInterpolatedPoint,
                    previousCurvePoint[1] + segmentDirectionVector[1] * distanceFromLastInterpolatedPoint,
                    previousCurvePoint[2] + segmentDirectionVector[2] * distanceFromLastInterpolatedPoint
                };
                sampledPoints->InsertNextPoint(newSampledPoint);
                distanceFromLastSampledPoint = 0;
                distanceFromLastInterpolatedPoint += samplingDistance;

                remainingSegmentLength -= samplingDistance;
            }
            distanceFromLastSampledPoint = remainingSegmentLength;
        }
        else
        {
            distanceFromLastSampledPoint += segmentLength;
        }
        previousCurvePoint[0] = currentCurvePoint[0];
        previousCurvePoint[1] = currentCurvePoint[1];
        previousCurvePoint[2] = currentCurvePoint[2];
    }

    // Make sure the resampled curve has the same size as the original
    // but avoid having very long or very short line segments at the end.
    if (closedCurve)
    {
        // Closed curve
        // Ideally, remainingSegmentLength would be equal to samplingDistance.
        if (remainingSegmentLength < samplingDistance * 0.5)
        {
            // last segment would be too short, so remove the last point and adjust position of second last point
            double lastPointPosition[3] = { 0.0 };
            vtkIdType lastPointOriginalPointIndex = 0;
            if (GetPositionAndClosestPointIndexAlongCurve(lastPointPosition, lastPointOriginalPointIndex,
                0, -(2.0 * samplingDistance + remainingSegmentLength) / 2.0, originalPoints, closedCurve))
            {
                sampledPoints->SetNumberOfPoints(sampledPoints->GetNumberOfPoints() - 1);
                sampledPoints->SetPoint(sampledPoints->GetNumberOfPoints() - 1, lastPointPosition);
            }
            else
            {
                // something went wrong, we could not add a point, therefore just remove the last point
                sampledPoints->SetNumberOfPoints(sampledPoints->GetNumberOfPoints() - 1);
            }
        }
        else
        {
            // last segment is only slightly shorter than the sampling distance
            // so just adjust the position of the last point
            double lastPointPosition[3] = { 0.0 };
            vtkIdType lastPointOriginalPointIndex = 0;
            if (GetPositionAndClosestPointIndexAlongCurve(lastPointPosition, lastPointOriginalPointIndex,
                0, -(samplingDistance + remainingSegmentLength) / 2.0, originalPoints, closedCurve))
            {
                sampledPoints->SetPoint(sampledPoints->GetNumberOfPoints() - 1, lastPointPosition);
            }
        }
    }
    else
    {
        // Open curve
        // Ideally, remainingSegmentLength would be equal to 0.
        if (remainingSegmentLength > samplingDistance * 0.5)
        {
            // last segment would be much longer than the sampling distance, so add an extra point
            double secondLastPointPosition[3] = { 0.0 };
            vtkIdType secondLastPointOriginalPointIndex = 0;
            if (GetPositionAndClosestPointIndexAlongCurve(secondLastPointPosition, secondLastPointOriginalPointIndex,
                originalPoints->GetNumberOfPoints() - 1, -(samplingDistance + remainingSegmentLength) / 2.0, originalPoints, closedCurve))
            {
                sampledPoints->SetPoint(sampledPoints->GetNumberOfPoints() - 1, secondLastPointPosition);
                sampledPoints->InsertNextPoint(originalPoints->GetPoint(originalPoints->GetNumberOfPoints() - 1));
            }
            else
            {
                // something went wrong, we could not add a point, therefore just adjust the last point position
                sampledPoints->SetPoint(sampledPoints->GetNumberOfPoints() - 1,
                    originalPoints->GetPoint(originalPoints->GetNumberOfPoints() - 1));
            }
        }
        else
        {
            // last segment is only slightly longer than the sampling distance
            // so we just adjust the position of last point
            sampledPoints->SetPoint(sampledPoints->GetNumberOfPoints() - 1,
                originalPoints->GetPoint(originalPoints->GetNumberOfPoints() - 1));
        }
    }
}

vtkSmartPointer<vtkImageData> iadAlgorithmCPR::computeStraightVolumeCPR(vtkPolyData* curve, vtkImageData* originalDcm)
{
    std::cout << "CurvedPlanarReformatLogic::computeStraightenVolumeData" << std::endl;
    int roi[2] = {150, 300};
    double spacing[3];
    originalDcm->GetSpacing(spacing);

    vtkNew<vtkPoints> centerlinePoints;
    centerlinePoints->DeepCopy(curve->GetPoints());

    vtkNew<vtkPoints> centerlinePointsResample;
    ResamplePoints(centerlinePoints, centerlinePointsResample, 0.5, 0); //重采样为间距0.5mm

    vtkNew<vtkPolyData> polyline1mm;
    convertPointsToPolyline(centerlinePointsResample, polyline1mm);

    vtkNew<vtkParallelTransportFrame> parallelTrans;
    parallelTrans->SetInputData(polyline1mm);
    //parallelTrans->SetPreferredInitialNormalVector(transformGridAxisX); //不指定的话默认 [1.0, 0.0, 0.0]
    parallelTrans->Update();

    vtkNew<vtkPolyData> ctrlineParallelTransport;
    ctrlineParallelTransport->DeepCopy(parallelTrans->GetOutput());

    vtkNew<vtkDoubleArray> ctrlineNormal;
    ctrlineNormal->DeepCopy(ctrlineParallelTransport->GetPointData()->GetAbstractArray(parallelTrans->GetNormalsArrayName()));
    vtkNew<vtkDoubleArray> ctrlineBinormal;
    ctrlineBinormal->DeepCopy(ctrlineParallelTransport->GetPointData()->GetAbstractArray(parallelTrans->GetBinormalsArrayName()));
    vtkNew<vtkDoubleArray> ctrlineTangent;
    ctrlineTangent->DeepCopy(ctrlineParallelTransport->GetPointData()->GetAbstractArray(parallelTrans->GetTangentsArrayName()));

    vtkSmartPointer<vtkImageAppend> appendCPR = vtkSmartPointer<vtkImageAppend>::New();
    appendCPR->SetAppendAxis(2);

    vtkSmartPointer<vtkImageData> tempSlice = vtkSmartPointer<vtkImageData>::New();
    for (vtkIdType i = 0; i < centerlinePointsResample->GetNumberOfPoints(); i++)
    {
        double curPt[3];
        centerlinePointsResample->GetPoint(i, curPt);

        vtkNew<vtkMatrix4x4> resliceMatrix;
        resliceMatrix->Identity();
        resliceMatrix->SetElement(0, 0, ctrlineNormal->GetTuple3(i)[0]);
        resliceMatrix->SetElement(1, 0, ctrlineNormal->GetTuple3(i)[1]);
        resliceMatrix->SetElement(2, 0, ctrlineNormal->GetTuple3(i)[2]);
        resliceMatrix->SetElement(0, 1, ctrlineBinormal->GetTuple3(i)[0]);
        resliceMatrix->SetElement(1, 1, ctrlineBinormal->GetTuple3(i)[1]);
        resliceMatrix->SetElement(2, 1, ctrlineBinormal->GetTuple3(i)[2]);
        resliceMatrix->SetElement(0, 2, ctrlineTangent->GetTuple3(i)[0]);
        resliceMatrix->SetElement(1, 2, ctrlineTangent->GetTuple3(i)[1]);
        resliceMatrix->SetElement(2, 2, ctrlineTangent->GetTuple3(i)[2]);
        resliceMatrix->SetElement(0, 3, curPt[0]);
        resliceMatrix->SetElement(1, 3, curPt[1]);
        resliceMatrix->SetElement(2, 3, curPt[2]);

        vtkNew<vtkImageReslice> reslice;
        reslice->SetInputData(originalDcm);
        reslice->TransformInputSamplingOff();
        reslice->SetOutputDimensionality(2);
        reslice->SetInterpolationModeToCubic();
        reslice->SetResliceAxes(resliceMatrix);
        reslice->SetOutputExtent(0, roi[0] - 1, 0, roi[1] - 1, 0, 0);
        reslice->SetOutputOrigin(-(roi[0] - 1) / 2 * spacing[0], -(roi[1] - 1) / 2 * spacing[1], 0); //左下角为原点
        reslice->Update();

        vtkNew<vtkImageChangeInformation> changer;
        changer->SetInputData(reslice->GetOutput());
        changer->SetOutputExtentStart(0, 0, 0);
        changer->SetOutputOrigin(0, 0, 0);
        changer->SetOutputSpacing(0.5, 0.5, 0.5);
        changer->Update();

        tempSlice->DeepCopy(changer->GetOutput());
        appendCPR->AddInputData(reslice->GetOutput());
    }
    appendCPR->Update();

    vtkNew<vtkImageFlip> flipFilter;
    flipFilter->SetInputData(appendCPR->GetOutput());
    flipFilter->SetFilteredAxes(2);
    flipFilter->Update();

    //    vtkNew<vtkXMLImageDataWriter> writer1;
    //    writer1->SetInputData(flipFilter->GetOutput());
    //    writer1->SetFileName("/home/arteryflow/Desktop/CPR-TestData/results/straightVolume.vti");
    //    writer1->Write();

    return flipFilter->GetOutput();
}

vtkSmartPointer<vtkVolume> iadAlgorithmCPR::dentalVR(vtkImageData* dcmImg)
{
    vtkSmartPointer<vtkGPUVolumeRayCastMapper> volumeMapperGPU = vtkSmartPointer<vtkGPUVolumeRayCastMapper>::New();
    volumeMapperGPU->SetInputData(dcmImg);
    volumeMapperGPU->SetImageSampleDistance(1.0);
    volumeMapperGPU->SetSampleDistance(1.0);
    volumeMapperGPU->SetAutoAdjustSampleDistances(1);
    volumeMapperGPU->SetUseJittering(1);

    vtkSmartPointer<vtkPiecewiseFunction> compositeOpacity = vtkSmartPointer<vtkPiecewiseFunction>::New();
    compositeOpacity->AddPoint(-50.0, 0.0);
    compositeOpacity->AddPoint(625.49, 0.0);
    compositeOpacity->AddPoint(1286.34, 0.0);
    compositeOpacity->AddPoint(1917.15, 0.175); // 0.7
    compositeOpacity->AddPoint(2300, 0.25); // 1.0
    compositeOpacity->AddPoint(4043.31, 0.25); // 1.0
    compositeOpacity->AddPoint(5462.06, 0.25); // 1.0 

    vtkSmartPointer<vtkColorTransferFunction> colorTransfer = vtkSmartPointer<vtkColorTransferFunction>::New();
    colorTransfer->AddRGBPoint(-50.38, 0.0, 1.0, 1.0);
    colorTransfer->AddHSVPoint(-50.38, 180.0 / 255.0, 1.0, 1.0);

    colorTransfer->AddRGBPoint(595.45, 76.0 / 255, 141.0 / 255, 141.0 / 255);
    colorTransfer->AddHSVPoint(595.45, 180.0 / 255.0, 118.0 / 255.0, 141.0 / 255.0);

    colorTransfer->AddRGBPoint(1196.22, 170.0 / 255, 0.0 / 255, 0.0 / 255);
    colorTransfer->AddHSVPoint(1196.22, 0.0, 1.0, 170.0 / 255.0);

    colorTransfer->AddRGBPoint(1568.38, 208.0 / 255.0, 116.0 / 255.0, 79.0 / 255.0);
    colorTransfer->AddHSVPoint(1568.38, 17.0 / 255.0, 158.0 / 255.0, 208.0 / 255.0);

    colorTransfer->AddRGBPoint(2427.80, 235.0 / 255, 197.0 / 255, 133.0 / 255);
    colorTransfer->AddHSVPoint(2427.80, 37.0 / 255.0, 111.0 / 255.0, 235.0 / 255.0);

    colorTransfer->AddRGBPoint(2989.06, 255.0 / 255, 255.0 / 255, 255.0 / 255);
    colorTransfer->AddHSVPoint(2989.06, 0.0, 0.0, 1.0);

    colorTransfer->AddRGBPoint(4680.69, 1.0, 1.0, 1.0);
    colorTransfer->AddHSVPoint(4680.69, 0.0, 0.0, 1.0);

    vtkSmartPointer<vtkVolumeProperty> volumeProperty = vtkSmartPointer<vtkVolumeProperty>::New();
    volumeProperty->SetInterpolationTypeToLinear();
    volumeProperty->ShadeOn();
    volumeProperty->SetAmbient(0.30);
    volumeProperty->SetDiffuse(0.50);
    volumeProperty->SetSpecular(0.25);
    volumeProperty->SetSpecularPower(37.5);
    volumeProperty->SetScalarOpacity(compositeOpacity);
    volumeProperty->SetColor(colorTransfer);
    //volumeProperty->SetGradientOpacity(gradientOpacity);
    volumeProperty->SetDisableGradientOpacity(1);

    vtkSmartPointer<vtkVolume> dentVolume = vtkSmartPointer<vtkVolume>::New();
    dentVolume->SetMapper(volumeMapperGPU);
    dentVolume->SetProperty(volumeProperty);
    dentVolume->Update();

    //vtkSmartPointer<vtkRenderer> ren = vtkSmartPointer<vtkRenderer>::New();
    //ren->AddActor(dentVolume);
    //ren->SetBackground(0, 0, 0);
    //ren->SetUseDepthPeelingForVolumes(1);
    //ren->SetMaximumNumberOfPeels(100);
    //ren->SetOcclusionRatio(0.5);

    return dentVolume;
}

// Helper functions
bool iadAlgorithmCPR::GetPositionAndClosestPointIndexAlongCurve(double foundCurvePosition[3], vtkIdType& foundClosestPointIndex, vtkIdType startCurvePointId, double distanceFromStartPoint, vtkPoints* curvePoints, bool closedCurve)
{
    vtkIdType numberOfCurvePoints = (curvePoints != nullptr ? curvePoints->GetNumberOfPoints() : 0);
    if (numberOfCurvePoints == 0)
    {
        std::cout << "iadAlgorithmCPR::GetPositionAndClosestPointIndexAlongCurve failed: invalid input points" << std::endl;
        foundClosestPointIndex = -1;
        return false;
    }
    if (startCurvePointId < 0 || startCurvePointId >= numberOfCurvePoints)
    {
        std::cout << "iadAlgorithmCPR::GetPositionAndClosestPointIndexAlongCurve failed: startCurvePointId is out of range" << std::endl;
        foundClosestPointIndex = -1;
        return false;
    }
    if (numberOfCurvePoints == 1 || distanceFromStartPoint == 0)
    {
        curvePoints->GetPoint(startCurvePointId, foundCurvePosition);
        foundClosestPointIndex = startCurvePointId;
        if (distanceFromStartPoint > 0.0)
        {
            std::cout << "iadAlgorithmCPR::GetPositionAndClosestPointIndexAlongCurve failed: non-zero distance"
                " is requested but only 1 point is available" << std::endl;
            return false;
        }
        else
        {
            return true;
        }
    }
    vtkIdType idIncrement = (distanceFromStartPoint > 0 ? 1 : -1);
    double remainingDistanceFromStartPoint = abs(distanceFromStartPoint);
    double previousPoint[3] = { 0.0 };
    curvePoints->GetPoint(startCurvePointId, previousPoint);
    vtkIdType pointId = startCurvePointId;
    bool curveConfirmedToBeNonZeroLength = false;
    double lastSegmentLength = 0;
    while (true)
    {
        pointId += idIncrement;

        // if reach the end then wrap around for closed curve, terminate search for open curve
        if (pointId < 0 || pointId >= numberOfCurvePoints)
        {
            if (closedCurve)
            {
                if (!curveConfirmedToBeNonZeroLength)
                {
                    if (GetCurveLength(curvePoints, closedCurve, -1, numberOfCurvePoints) == 0.0)
                    {
                        foundClosestPointIndex = -1;
                        return false;
                    }
                    curveConfirmedToBeNonZeroLength = true;
                }
                pointId = (pointId < 0 ? numberOfCurvePoints : -1);
                continue;
            }
            else
            {
                // reached end of curve before getting at the requested distance
                // return closest
                foundClosestPointIndex = (pointId < 0 ? 0 : numberOfCurvePoints - 1);
                curvePoints->GetPoint(foundClosestPointIndex, foundCurvePosition);
                return false;
            }
        }

        // determine how much closer we are now
        double* nextPoint = curvePoints->GetPoint(pointId);
        lastSegmentLength = sqrt(vtkMath::Distance2BetweenPoints(nextPoint, previousPoint));
        remainingDistanceFromStartPoint -= lastSegmentLength;

        if (remainingDistanceFromStartPoint <= 0)
        {
            // reached the requested distance (and probably a bit more)
            for (int i = 0; i < 3; i++)
            {
                foundCurvePosition[i] = nextPoint[i] +
                    remainingDistanceFromStartPoint * (nextPoint[i] - previousPoint[i]) / lastSegmentLength;
            }
            if (fabs(remainingDistanceFromStartPoint) <= fabs(remainingDistanceFromStartPoint + lastSegmentLength))
            {
                foundClosestPointIndex = pointId;
            }
            else
            {
                foundClosestPointIndex = pointId - 1;
            }
            break;
        }

        previousPoint[0] = nextPoint[0];
        previousPoint[1] = nextPoint[1];
        previousPoint[2] = nextPoint[2];
    }
    return true;
}

double iadAlgorithmCPR::GetCurveLength(vtkPoints* curvePoints, bool closedCurve, vtkIdType startCurvePointIndex, vtkIdType numberOfCurvePoints)
{
    if (!curvePoints || curvePoints->GetNumberOfPoints() < 2)
    {
        return 0.0;
    }
    if (startCurvePointIndex < 0)
    {
        std::cout << "Invalid startCurvePointIndex=" << startCurvePointIndex << ", using 0 instead" << std::endl;

        startCurvePointIndex = 0;
    }
    vtkIdType lastCurvePointIndex = curvePoints->GetNumberOfPoints() - 1;
    if (numberOfCurvePoints >= 0 && startCurvePointIndex + numberOfCurvePoints - 1 < lastCurvePointIndex)
    {
        lastCurvePointIndex = startCurvePointIndex + numberOfCurvePoints - 1;
    }

    double length = 0.0;
    double previousPoint[3] = { 0.0 };
    double nextPoint[3] = { 0.0 };
    curvePoints->GetPoint(startCurvePointIndex, previousPoint);
    for (vtkIdType curvePointIndex = startCurvePointIndex + 1; curvePointIndex <= lastCurvePointIndex; curvePointIndex++)
    {
        curvePoints->GetPoint(curvePointIndex, nextPoint);
        length += sqrt(vtkMath::Distance2BetweenPoints(previousPoint, nextPoint));
        previousPoint[0] = nextPoint[0];
        previousPoint[1] = nextPoint[1];
        previousPoint[2] = nextPoint[2];
    }
    // Add length of closing segment
    if (closedCurve && (numberOfCurvePoints < 0 || numberOfCurvePoints >= curvePoints->GetNumberOfPoints()))
    {
        curvePoints->GetPoint(0, nextPoint);
        length += sqrt(vtkMath::Distance2BetweenPoints(previousPoint, nextPoint));
    }
    return length;
}

void iadAlgorithmCPR::convertPointsToPolyline(vtkPoints* inputPoints, vtkPolyData* outputPolyline)
{
    vtkNew<vtkPolyLine> polyLine;
    polyLine->GetPointIds()->SetNumberOfIds(inputPoints->GetNumberOfPoints());
    for (unsigned int i = 0; i < inputPoints->GetNumberOfPoints(); i++)
    {
        polyLine->GetPointIds()->SetId(i, i);
    }

    // Create a cell array to store the lines in and add the lines to it
    vtkNew<vtkCellArray> cells;
    cells->InsertNextCell(polyLine);

    // Create a polydata to store everything in
    vtkNew<vtkPolyData> polyData;

    // Add the points to the dataset
    polyData->SetPoints(inputPoints);

    // Add the lines to the dataset
    polyData->SetLines(cells);

    outputPolyline->DeepCopy(polyData);
}
