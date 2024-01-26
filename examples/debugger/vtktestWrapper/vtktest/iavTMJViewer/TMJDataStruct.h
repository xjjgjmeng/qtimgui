#pragma once
#include <vtkSmartPointer.h>
#include <vtkDICOMImageReader.h>
#include <vtkImageActor.h>
#include <vtkPolyData.h>
#include <vtkDoubleArray.h>
#include <vtkVolume.h>
#include <vtkClipVolume.h>
#include <vtkImageAppend.h>
#include <array>


struct SlicePosition {
	int s_idx;
	bool s_isSagittal;
	bool s_isLeft;
};

struct TMJDataStruct
{
	static constexpr double splineResamplingDistance = 0.4;
    //vtkSmartPointer<vtkDICOMImageReader> dcmReader = vtkSmartPointer<vtkDICOMImageReader>::New();
    vtkSmartPointer<vtkImageActor> currentTransSliceImgActor = vtkSmartPointer<vtkImageActor>::New();
	vtkSmartPointer<vtkImageAppend> appendLeftTMJ = vtkSmartPointer<vtkImageAppend>::New();  //slices stack
	vtkSmartPointer<vtkImageAppend> appendRightTMJ = vtkSmartPointer<vtkImageAppend>::New();  //slices stack
	vtkSmartPointer<vtkVolume> m_Volume = vtkSmartPointer<vtkVolume>::New();
	double sliceInterval = 1.2;
	int sliceNum = 0;

	bool isRotateP2 = false;
	bool isRotateP1 = false;
	bool isClip = false;  // begin to clip
	//voi point
	double point1[2];
	double point2[2];
	double mid_point[2];
	double WorldPoint1[3];
	double WorldPoint2[3];
	double vWorldPoint1[3];
	double vWorldPoint2[3];
	//mirror voi point
	double mirrorPoint1[2];
	double mirrorPoint2[2];
	double mid_mpoint[2];
	double MWorldPoint1[3];
	double MWorldPoint2[3];
	double vMWorldPoint1[3];
	double vMWorldPoint2[3];

	std::vector<std::array<double, 3>> l_worldArray;  // left retangle points
	std::vector<std::array<double, 3>> r_worldArray;  // right retangle points


	// mouse wheel
	int countOfForwardLeftTop = 0;  // LeftTop
	int countOfBackLeftTop = 0;
	int countOfForwardLeftBottom = 0;  // LeftBottom
	int countOfBackLeftBottom = 0;
	int countOfForwardRightTop = 0;  // RightTop
	int countOfBackRightTop = 0;
	int countOfForwardRightBottom = 0;  // RightBottom
	int countOfBackRightBottom = 0;

	bool isMouseWheel = false;
	bool isLeftTop = false;
	bool isLeftBottom = false;
	bool isRightTop = false;
	bool isRightBottom = false;

	double vPointBak1[2];  // left top
	double vPointBak2[2];
	double vMirrorPointBak1[2]; // right top
	double vMirrorPointBak2[2];
	double pointBak1[2]; //left bottom
	double pointBak2[2];
	double mirrorPointBak1[2];  // right bottom
	double mirrorPointBak2[2];
};