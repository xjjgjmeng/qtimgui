#pragma once
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkCoordinate.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkActor2D.h>
#include <vtkPolyData.h>
#include <vtkPoints.h>
#include <vtkProperty2D.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkMatrix4x4.h>
#include <vtkLine.h>
#include <vtkCamera.h>
#include <vtkRenderer.h>
#include <vtkResliceImageViewer.h>
#include <vtkImageReslice.h>
#include <vtkImageMapToColors.h>
#include <vtkCellPicker.h>
#include <vtkVolume.h>
#include <vtkGPUVolumeRayCastMapper.h>
#include <vtkPlane.h>
#include <vtkPolyData.h>
#include <vtkImageActor.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkImageChangeInformation.h>
#include <vtkImageData.h>
#include <vtkInteractorStyleTrackballActor.h>
#include <vtkImageActorPointPlacer.h>

#include "CPRPanWindow.h"

#define M_PI 3.14159265358979323846
class InteractorStyleCprNormal : public vtkInteractorStyleTrackballActor
{
public:
    static InteractorStyleCprNormal* New()
    {
        return new InteractorStyleCprNormal();
    }

    void OnLeftButtonDown(void)override;
    void OnLeftButtonUp(void)override;
    void OnLeftButtonDoubleDown() {}
    void OnMouseMove(void)override;

    void SetLinePolyData(vtkSmartPointer<vtkPolyData> polydata);
    void SetLineActor(vtkSmartPointer<vtkActor2D> lineActor);
    void SetImageActor(vtkSmartPointer<vtkImageActor> imageactor);
    void SetPanWindowClass(CPRPanWindow* window);
   // void SetRenSliceViewer(vtkSmartPointer<vtkRenderer> ren[], vtkSmartPointer<vtkImageReslice> slice[], vtkSmartPointer<vtkResliceImageViewer> viewer[]);


    double GetRotateRadian();
    double GetImageResliceAxis();
    void ResliceUpdate(double XtoZ, double origin0);
    double distancePointToLine(const double origin[3], const double linePoint1[3], const double linePoint2[3]);

private:
    int currPos[2];
    
    int lastPos[2];
    int mMouseUpTime = -60000;
    
    CPRPanWindow* m_panWindow;

protected:
    bool moveAxis;  // 平移
    bool rotateAxis; // 旋转
    //double* MoveRotateAxisPoint;  // 坐标指针，MoveRotateAxisPoint[0]为平移或旋转时鼠标当前的横坐标
    bool isLeftButtonDown;  // 鼠标左键是否处于按下状态
    vtkSmartPointer<vtkPolyData> cursorPolyData;
    vtkSmartPointer<vtkActor2D> cursorActor;
    vtkSmartPointer<vtkImageActor> imageActor;
    double boundx = 0;  // imageActor对应边界xmax

    //vtkSmartPointer<vtkRenderer> resliceRen[5];  // 切片render
    vtkSmartPointer<vtkImageReslice> reslice[5];
    vtkSmartPointer<vtkResliceImageViewer> imageViewer[5];
    double angleRadian;  // 旋转弧度
    double imageResliceAxes;  //切片位置对应的轴，对应的是十字线移动的X轴

    vtkSmartPointer<vtkMatrix4x4> resliceMatrix1;

    vtkSmartPointer<vtkImageActorPointPlacer> m_imagePointPicker;

};


