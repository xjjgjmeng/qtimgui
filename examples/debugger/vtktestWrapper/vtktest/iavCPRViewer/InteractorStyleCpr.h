#include <vtkInteractorStyleTrackballActor.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkCellPicker.h>
#include <vtkCamera.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkTransform.h>
#include <vtkRendererCollection.h>
#include <vtkPoints.h>
#include <vtkLineSource.h>
#include <vtkActor.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkVertexGlyphFilter.h>
#include <vtkActor2D.h>
#include <vtkAssembly.h>

#include "CPRDataStruct.h"

#define M_PI 3.14159265358979323846


class InteractorStyleCpr : public vtkInteractorStyleTrackballActor
{
public:
	static InteractorStyleCpr* New();
	vtkTypeMacro(InteractorStyleCpr, vtkInteractorStyleTrackballActor);

	void OnLeftButtonDown() override;
	void OnLeftButtonUp() override;
	void Spin() override;
	void Pan() override;
	void Init(const double bounds[6], CPRDataStruct* pData);
	void SetImplantActors(vtkSmartPointer<vtkActor> implantActors[]);
	//void SetImplantActor(vtkSmartPointer<vtkAssembly> implantActor) ;

protected:
	~InteractorStyleCpr()
	{
		if (auto rw = this->m_ren->GetRenderWindow())
		{
			this->m_ren->RemoveActor(m_lineActor);
			rw->Render();
		}
	}

private:
	vtkSmartPointer<vtkActor> m_lineActor;
	vtkSmartPointer<vtkLineSource> lineSource;
	vtkSmartPointer<vtkRenderer> m_ren;
	//vtkActor* pImplantCtrLineActor;
	//vtkSmartPointer<vtkAssembly> pImplantCtrLineActor;
	vtkSmartPointer<vtkActor> pImplantCtrLineActor;
	vtkSmartPointer<vtkActor> pImplantPointActor0;
	vtkSmartPointer<vtkActor> pImplantPointActor1;
	vtkSmartPointer<vtkPoints> point0 = vtkSmartPointer<vtkPoints>::New();
	vtkSmartPointer<vtkPoints> point1 = vtkSmartPointer<vtkPoints>::New();
	vtkSmartPointer<vtkActor> PointActor0 = vtkSmartPointer<vtkActor>::New();
	vtkSmartPointer<vtkActor> PointActor1 = vtkSmartPointer<vtkActor>::New();


	CPRDataStruct* m_Data = nullptr;

	double Bounds[6];  // 图像边界
	double angleRadian;  // 旋转弧度，相对于竖直方向
	double MouseCoords[3];  // 鼠标对应的像素坐标
	double angle;
	double lineLength = 0;
};
