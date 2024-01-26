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
    bool moveAxis;  // ƽ��
    bool rotateAxis; // ��ת
    //double* MoveRotateAxisPoint;  // ����ָ�룬MoveRotateAxisPoint[0]Ϊƽ�ƻ���תʱ��굱ǰ�ĺ�����
    bool isLeftButtonDown;  // �������Ƿ��ڰ���״̬
    vtkSmartPointer<vtkPolyData> cursorPolyData;
    vtkSmartPointer<vtkActor2D> cursorActor;
    vtkSmartPointer<vtkImageActor> imageActor;
    double boundx = 0;  // imageActor��Ӧ�߽�xmax

    //vtkSmartPointer<vtkRenderer> resliceRen[5];  // ��Ƭrender
    vtkSmartPointer<vtkImageReslice> reslice[5];
    vtkSmartPointer<vtkResliceImageViewer> imageViewer[5];
    double angleRadian;  // ��ת����
    double imageResliceAxes;  //��Ƭλ�ö�Ӧ���ᣬ��Ӧ����ʮ�����ƶ���X��

    vtkSmartPointer<vtkMatrix4x4> resliceMatrix1;

    vtkSmartPointer<vtkImageActorPointPlacer> m_imagePointPicker;

};


