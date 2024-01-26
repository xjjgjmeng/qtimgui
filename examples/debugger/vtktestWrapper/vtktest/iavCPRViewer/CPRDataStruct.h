#pragma once
#include <vtkSmartPointer.h>
#include <vtkDICOMImageReader.h>
#include <vtkImageActor.h>
#include <vtkPolyData.h>
#include <vtkDoubleArray.h>
#include <vtkActor.h>
#include <vtkAssembly.h>


struct CPRDataStruct
{
    //CPR window data definition - Put only shared data here!
    static constexpr double splineResamplingDistance = 0.4;
    int currentPickedPointID = 0; 
    double sliceInterval = 1.2;
    int sliceNum = 0;


    //vtkSmartPointer<vtkDICOMImageReader> dcmReader = vtkSmartPointer<vtkDICOMImageReader>::New();
    vtkSmartPointer<vtkImageActor> currentTransSliceImgActor = vtkSmartPointer<vtkImageActor>::New();

    vtkSmartPointer<vtkPoints> splinePointsSet = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkPolyData> splinePoints = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkPolyData> spline = vtkSmartPointer<vtkPolyData>::New(); //red

    vtkSmartPointer<vtkDoubleArray> ctrlineNormal = vtkSmartPointer<vtkDoubleArray>::New();
    vtkSmartPointer<vtkDoubleArray> ctrlineBinormal = vtkSmartPointer<vtkDoubleArray>::New();
    vtkSmartPointer<vtkDoubleArray> ctrlineTangent = vtkSmartPointer<vtkDoubleArray>::New();
    std::pair<double, double> cprRoi;

    // Nerve drawing data 
    double numOfTubes = 0;
    double tubeRadius = 1.0;
    vtkSmartPointer<vtkPoints> NervePointsSetInPanWin = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkPoints> NervePointsSetInPanWin2 = vtkSmartPointer<vtkPoints>::New();

    vtkSmartPointer<vtkPoints> NervePointsSetInVrWin = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkPoints> NervePointsSetInVrWin2 = vtkSmartPointer<vtkPoints>::New();

    std::vector<vtkSmartPointer<vtkActor>> nervePointsActorVec1;
    std::vector<vtkSmartPointer<vtkActor>> nervePointsActorVec2;


    // pan window blue cursor line data
    double MoveRotateAxisPoint[4];
    double panCursorLineRoateAngleRadian = 0;
    vtkSmartPointer<vtkImageData> straightVolume = vtkSmartPointer<vtkImageData>::New();
    double CursorLineXCoords;  // p0[2]
    vtkSmartPointer<vtkPolyData> lineCursorCpy = vtkSmartPointer<vtkPolyData>::New();


    //Implant drawing data
    vtkSmartPointer<vtkPoints> ImplantPointsSetInPanWin = vtkSmartPointer<vtkPoints>::New(); 
    vtkSmartPointer<vtkPoints> ImplantPointsSetInVrWin = vtkSmartPointer<vtkPoints>::New();
    //vtkSmartPointer<vtkAssembly> ImplantAssembly = vtkSmartPointer<vtkAssembly>::New();
    vtkSmartPointer<vtkActor> ImplantActors[3];
    double implantLength = 0;
    double point0[3];
    double point1[3];
    double panImplantRotateAngleRadian = 0;
};