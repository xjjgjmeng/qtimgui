#pragma once

#include <list>
#include <functional>

#include <imgui.h>
#include <QtImGui.h>

#include "vtkBoundedPlanePointPlacer.h"
#include "vtkCellPicker.h"
#include "vtkCommand.h"
#include "vtkDICOMImageReader.h"
#include "vtkDistanceRepresentation.h"
#include "vtkDistanceRepresentation2D.h"
#include "vtkDistanceWidget.h"
#include "vtkHandleRepresentation.h"
#include "vtkImageData.h"
#include "vtkImageMapToWindowLevelColors.h"
#include "vtkImageSlabReslice.h"
#include "vtkInteractorStyleImage.h"
#include "vtkLookupTable.h"
#include "vtkPlane.h"
#include "vtkPlaneSource.h"
#include "vtkPointHandleRepresentation2D.h"
#include "vtkPointHandleRepresentation3D.h"
#include "vtkProperty.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkResliceCursor.h"
#include "vtkResliceCursorActor.h"
#include "vtkResliceCursorLineRepresentation.h"
#include "vtkResliceCursorPolyDataAlgorithm.h"
#include "vtkResliceCursorThickLineRepresentation.h"
#include "vtkResliceCursorWidget.h"
#include "vtkResliceImageViewer.h"
#include "vtkResliceImageViewerMeasurements.h"
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkCamera.h>
#include <vtkImageActor.h>
#include <vtkMatrix4x4.h>
#include <vtkRendererCollection.h>

static void vtkObjImgui(const char* titile, vtkObject* vtkobj, std::list<std::function<void()>>& funcList, const ImGuiTreeNodeFlags flags = 0)
{
    if (!vtkobj)
    {
        ImGui::Text("%s is nullptr", titile);
        return;
    }

    if (ImGui::TreeNodeEx(titile, flags))
    {
        if (ImGui::CollapsingHeader("vtkObject", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Text("ptr: %0x", vtkobj);
            ImGui::Text("MTime: %ld", vtkobj->GetMTime());
            ImGui::Text("ClassName: %s", vtkobj->GetClassName());
        }

        // vtkObject
        if (auto pImageViewer2 = vtkImageViewer2::SafeDownCast(vtkobj); pImageViewer2 && ImGui::CollapsingHeader("vtkImageViewer2", ImGuiTreeNodeFlags_DefaultOpen))
        {
            {
                int v[3]; pImageViewer2->GetSliceRange(v);
                ImGui::Text("SliceRange: %d %d", v[0], v[1]);
                if (v[2] = pImageViewer2->GetSlice(), ImGui::SliderInt("Slice", &v[2], v[0], v[1]))
                {
                    funcList.push_back([pImageViewer2, v]
                        {
                            pImageViewer2->SetSlice(v[2]);
                        });
                }
            }

            if (float v = pImageViewer2->GetColorLevel(); ImGui::SliderFloat("ColorLevel", &v, 0., 10000.))
            {
                funcList.push_back([pImageViewer2, v]
                    {
                        pImageViewer2->SetColorLevel(v);
                    });
            }

            if (float v = pImageViewer2->GetColorWindow(); ImGui::SliderFloat("ColorWindow", &v, 0., 10000.))
            {
                funcList.push_back([pImageViewer2, v]
                    {
                        pImageViewer2->SetColorWindow(v);
                    });
            }

            if (int v[2]{ pImageViewer2->GetSize()[0], pImageViewer2->GetSize()[1] }; ImGui::DragInt2("Size", v))
            {
                funcList.push_back([pImageViewer2, v]()mutable
                    {
                        pImageViewer2->SetSize(v);
                    });
            }

            if (int v[2]{ pImageViewer2->GetPosition()[0], pImageViewer2->GetPosition()[1] }; ImGui::DragInt2("Position", v))
            {
                funcList.push_back([pImageViewer2, v]()mutable
                    {
                        pImageViewer2->SetPosition(v);
                    });
            }

            {
                const char* modeText[] = { "SLICE_ORIENTATION_YZ", "SLICE_ORIENTATION_XZ", "SLICE_ORIENTATION_XY" };
                if (auto v = pImageViewer2->GetSliceOrientation(); ImGui::Combo("SliceOrientation", &v, modeText, IM_ARRAYSIZE(modeText)))
                {
                    funcList.push_back([pImageViewer2, v]()mutable
                        {
                            pImageViewer2->SetSliceOrientation(v);
                        });
                }
            }

            if (bool v = pImageViewer2->GetOffScreenRendering(); ImGui::Checkbox("OffScreenRendering ", &v))
            {
                funcList.push_back([pImageViewer2, v]()mutable
                    {
                        pImageViewer2->SetOffScreenRendering(v);
                    });
            }
            vtkObjImgui("ImageActor", pImageViewer2->GetImageActor(), funcList, ImGuiTreeNodeFlags_DefaultOpen);
        }
        else if (auto pResliceCursor = vtkResliceCursor::SafeDownCast(vtkobj); pResliceCursor && ImGui::CollapsingHeader("vtkResliceCursor", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (bool v = pResliceCursor->GetThickMode(); ImGui::Checkbox("ThickMode ", &v))
            {
                funcList.push_back([pResliceCursor, v]
                    {
                        pResliceCursor->SetThickMode(v);
                    });
            }

            if (double v[3]; pResliceCursor->GetThickness(v), ImGui::DragScalarN("Thickness", ImGuiDataType_Double, v, 3, 0.5f))
            {
                funcList.push_back([pResliceCursor, v]
                    {
                        pResliceCursor->SetThickness(v);
                    });
            }
        }
        else if (auto pInteractorObserver = vtkInteractorObserver::SafeDownCast(vtkobj); pInteractorObserver && ImGui::CollapsingHeader("vtkInteractorObserver", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (bool v = pInteractorObserver->GetEnabled(); ImGui::Checkbox("Enabled ", &v))
            {
                pInteractorObserver->SetEnabled(v);
            }

            if (ImGui::Button("On")) pInteractorObserver->On(); ImGui::SameLine();
            if (ImGui::Button("Off")) pInteractorObserver->Off();
        }
        else if (auto pProp = vtkProp::SafeDownCast(vtkobj); pProp && ImGui::CollapsingHeader("vtkProp", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (pProp->GetBounds())
            {
                ImGui::InputScalarN("Bounds", ImGuiDataType_Double, pProp->GetBounds(), 6, nullptr, nullptr, nullptr, ImGuiInputTextFlags_ReadOnly);
            }
            else
            {
                ImGui::Text("Bounds nullptr");
            }

            if (pProp->GetMatrix())
            {
                ImGui::InputScalarN("Matrix0", ImGuiDataType_Double, pProp->GetMatrix()->GetData() + 0, 4, nullptr, nullptr, nullptr, ImGuiInputTextFlags_ReadOnly);
                ImGui::InputScalarN("Matrix1", ImGuiDataType_Double, pProp->GetMatrix()->GetData() + 4, 4, nullptr, nullptr, nullptr, ImGuiInputTextFlags_ReadOnly);
                ImGui::InputScalarN("Matrix2", ImGuiDataType_Double, pProp->GetMatrix()->GetData() + 8, 4, nullptr, nullptr, nullptr, ImGuiInputTextFlags_ReadOnly);
                ImGui::InputScalarN("Matrix3", ImGuiDataType_Double, pProp->GetMatrix()->GetData() + 12, 4, nullptr, nullptr, nullptr, ImGuiInputTextFlags_ReadOnly);
            }
            else
            {
                ImGui::Text("Matrix nullptr");
            }

            if (bool visibility = pProp->GetVisibility(); ImGui::Checkbox("Visibility ", &visibility))
            {
                pProp->SetVisibility(visibility);
            }
            ImGui::SameLine();
            if (bool pickable = pProp->GetPickable(); ImGui::Checkbox("Pickable", &pickable))
            {
                pProp->SetPickable(pickable);
            }
            ImGui::SameLine();
            if (bool dragable = pProp->GetDragable(); ImGui::Checkbox("Dragable", &dragable))
            {
                pProp->SetDragable(dragable);
            }
            }
        else if (const auto pProperty = vtkProperty::SafeDownCast(vtkobj); pProperty && ImGui::CollapsingHeader("vtkProperty", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (bool lighting = pProperty->GetLighting(); ImGui::Checkbox("Lighting", &lighting))
            {
                pProperty->SetLighting(lighting);
            }
            if (bool f = pProperty->GetEdgeVisibility(); ImGui::Checkbox("EdgeVisibility", &f))
            {
                pProperty->SetEdgeVisibility(f);
            }
            if (bool f = pProperty->GetVertexVisibility(); ImGui::Checkbox("VertexVisibility", &f))
            {
                pProperty->SetVertexVisibility(f);
            }
            if (bool f = pProperty->GetRenderPointsAsSpheres(); ImGui::Checkbox("RenderPointsAsSpheres", &f))
            {
                pProperty->SetRenderPointsAsSpheres(f);
            }
            if (bool f = pProperty->GetRenderLinesAsTubes(); ImGui::Checkbox("RenderLinesAsTubes", &f))
            {
                pProperty->SetRenderLinesAsTubes(f);
            }
            float color[3] = { pProperty->GetColor()[0],pProperty->GetColor()[1],pProperty->GetColor()[2] };
            if (ImGui::ColorEdit3("Color", color))
            {
                pProperty->SetColor(color[0], color[1], color[2]);
            }
            float edgeColor[3] = { pProperty->GetEdgeColor()[0],pProperty->GetEdgeColor()[1],pProperty->GetEdgeColor()[2] };
            if (ImGui::ColorEdit3("EdgeColor", edgeColor))
            {
                pProperty->SetEdgeColor(edgeColor[0], edgeColor[1], edgeColor[2]);
            }
            float vertexColor[3] = { pProperty->GetVertexColor()[0],pProperty->GetVertexColor()[1],pProperty->GetVertexColor()[2] };
            if (ImGui::ColorEdit3("VertexColor", vertexColor))
            {
                pProperty->SetVertexColor(vertexColor[0], vertexColor[1], vertexColor[2]);
            }
            //float coatColor[3] = { pProperty->GetCoatColor()[0],pProperty->GetCoatColor()[1],pProperty->GetCoatColor()[2] };
            //if (ImGui::ColorEdit3("CoatColor", coatColor))
            //{
            //    pProperty->SetCoatColor(coatColor[0], coatColor[1], coatColor[2]);
            //}
            if (float opacity = pProperty->GetOpacity(); ImGui::SliderFloat("Opacity", &opacity, 0.0f, 1.0f, "opacity = %.3f"))
            {
                pProperty->SetOpacity(opacity);
            }
            if (float v = pProperty->GetLineWidth(); ImGui::SliderFloat("LineWidth", &v, 0.0f, 30.0f))
            {
                pProperty->SetLineWidth(v);
            }
            if (float v = pProperty->GetPointSize(); ImGui::SliderFloat("PointSize", &v, 1.0f, 100.0f))
            {
                pProperty->SetPointSize(v);
            }
            //if (float v = pProperty->GetCoatStrength(); ImGui::SliderFloat("CoatStrength", &v, 0.0f, 1.0f))
            //{
            //    pProperty->SetCoatStrength(v);
            //}
            //if (float v = pProperty->GetCoatRoughness(); ImGui::SliderFloat("CoatRoughness", &v, 0.0f, 1.0f))
            //{
            //    pProperty->SetCoatRoughness(v);
            //}
            if (float v = pProperty->GetMetallic(); ImGui::SliderFloat("Metallic", &v, 0.0f, 1.0f))
            {
                pProperty->SetMetallic(v);
            }
            if (int v = pProperty->GetLineStipplePattern(); ImGui::DragInt("LineStipplePattern", &v, 0xFF))
            {
                pProperty->SetLineStipplePattern(v);
            }
            if (int v = pProperty->GetLineStippleRepeatFactor(); ImGui::SliderInt("LineStippleRepeatFactor", &v, 1, 100))
            {
                pProperty->SetLineStippleRepeatFactor(v);
            }
        }

        // vtkProp
        if (auto pWidgetRepresentation = vtkWidgetRepresentation::SafeDownCast(vtkobj); pWidgetRepresentation && ImGui::CollapsingHeader("vtkWidgetRepresentation", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (bool v = pWidgetRepresentation->GetPickingManaged(); ImGui::Checkbox("PickingManaged", &v))
            {
                pWidgetRepresentation->SetPickingManaged(v);
            }
            if (float v = pWidgetRepresentation->GetPlaceFactor(); ImGui::SliderFloat("PlaceFactor", &v, 0., 2.))
            {
                pWidgetRepresentation->SetPlaceFactor(v);
            }
            if (float v = pWidgetRepresentation->GetHandleSize(); ImGui::SliderFloat("HandleSize", &v, 0., 20.))
            {
                pWidgetRepresentation->SetHandleSize(v);
            }
            ImGui::Text("InteractionState: %d", pWidgetRepresentation->GetInteractionState());
        }
        else if (auto pProp3D = vtkProp3D::SafeDownCast(vtkobj); pProp3D && ImGui::CollapsingHeader("vtkProp3D", ImGuiTreeNodeFlags_DefaultOpen))
        {

        }

        // vtkProp3D
        if (auto pResliceCursorActor = vtkResliceCursorActor::SafeDownCast(vtkobj); pResliceCursorActor && ImGui::CollapsingHeader("vtkResliceCursorActor", ImGuiTreeNodeFlags_DefaultOpen))
        {
            vtkObjImgui("CenterlineActor0", pResliceCursorActor->GetCenterlineActor(0), funcList);
            vtkObjImgui("CenterlineActor1", pResliceCursorActor->GetCenterlineActor(1), funcList);
            vtkObjImgui("CenterlineActor2", pResliceCursorActor->GetCenterlineActor(2), funcList);
            vtkObjImgui("CenterlineProperty0", pResliceCursorActor->GetCenterlineProperty(0), funcList);
            vtkObjImgui("CenterlineProperty1", pResliceCursorActor->GetCenterlineProperty(1), funcList);
            vtkObjImgui("CenterlineProperty2", pResliceCursorActor->GetCenterlineProperty(2), funcList);
            vtkObjImgui("ThickSlabProperty0", pResliceCursorActor->GetThickSlabProperty(0), funcList);
            vtkObjImgui("ThickSlabProperty1", pResliceCursorActor->GetThickSlabProperty(1), funcList);
            vtkObjImgui("ThickSlabProperty2", pResliceCursorActor->GetThickSlabProperty(2), funcList);
        }

        // vtkThreadedImageAlgorithm
        if (auto pImageReslice = vtkImageReslice::SafeDownCast(vtkobj); pImageReslice && ImGui::CollapsingHeader("vtkImageReslice", ImGuiTreeNodeFlags_DefaultOpen))
        {
            {
                if (ImGui::TreeNodeEx("ResliceAxesDirectionCosines", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    double xyz[9];
                    pImageReslice->GetResliceAxesDirectionCosines(xyz);
                    if (ImGui::DragScalarN("X", ImGuiDataType_Double, xyz, 3, .01f))
                    {
                        pImageReslice->SetResliceAxesDirectionCosines(xyz);
                        pImageReslice->Update();
                    }
                    if (ImGui::DragScalarN("Y", ImGuiDataType_Double, xyz + 3, 3, .01f))
                    {
                        pImageReslice->SetResliceAxesDirectionCosines(xyz);
                        //::colorMap->Update();
                    }
                    if (ImGui::DragScalarN("Z", ImGuiDataType_Double, xyz + 6, 3, .01f))
                    {
                        pImageReslice->SetResliceAxesDirectionCosines(xyz);
                        //::colorMap->Update();
                    }

                    ImGui::TreePop();
                }

                double o[3];
                pImageReslice->GetResliceAxesOrigin(o);
                if (ImGui::DragScalarN("ResliceAxesOrigin", ImGuiDataType_Double, o, 3, .1f))
                {
                    pImageReslice->SetResliceAxesOrigin(o);
                    //::colorMap->Update();
                }
            }

            if (int v = pImageReslice->GetSlabNumberOfSlices(); ImGui::DragInt("SlabNumberOfSlices", &v, 1, 1, 10000))
            {
                pImageReslice->SetSlabNumberOfSlices(v);
                //::colorMap->Update();
            }

            {
                const char* slabModeText[] = { "VTK_IMAGE_SLAB_MIN", "VTK_IMAGE_SLAB_MAX", "VTK_IMAGE_SLAB_MEAN", "VTK_IMAGE_SLAB_SUM" };
                if (int v = pImageReslice->GetSlabMode(); ImGui::Combo("SlabMode", &v, slabModeText, IM_ARRAYSIZE(slabModeText)))
                {
                    pImageReslice->SetSlabMode(v);
                    //::colorMap->Update();
                }
            }

            if (ImGui::TreeNodeEx("Output", ImGuiTreeNodeFlags_DefaultOpen))
            {
                {
                    double myArray[3];
#if 1
                    pImageReslice->GetOutputSpacing(myArray); //1
#else
                    ::reslice->GetOutputInformation(0)->Get(vtkDataObject::SPACING(), myArray); // 0.25
#endif
                    if (ImGui::DragScalarN("Spacing", ImGuiDataType_Double, myArray, 3, .1f))
                    {
                        pImageReslice->SetOutputSpacing(myArray);
                        //::colorMap->Update();
                    }
                }
                {
                    int myArray[6];
                    pImageReslice->GetOutputExtent(myArray);
                    if (ImGui::DragScalarN("Extent", ImGuiDataType_S32, myArray, 6))
                    {
                        pImageReslice->SetOutputExtent(myArray);
                        //::colorMap->Update();
                    }
                }

                ImGui::TreePop();
            }
        }

        // vtkImageReslice
        if (auto pImageSlabReslice = vtkImageSlabReslice::SafeDownCast(vtkobj); pImageSlabReslice && ImGui::CollapsingHeader("vtkImageSlabReslice", ImGuiTreeNodeFlags_DefaultOpen))
        {
            {
                const char* modeText[] = { "VTK_IMAGE_SLAB_MIN", "VTK_IMAGE_SLAB_MAX", "VTK_IMAGE_SLAB_MEAN", "VTK_IMAGE_SLAB_SUM" };
                if (auto v = pImageSlabReslice->GetBlendMode(); ImGui::Combo("BlendMode", &v, modeText, IM_ARRAYSIZE(modeText)))
                {
                    funcList.push_back([pImageSlabReslice, v]
                        {
                            pImageSlabReslice->SetBlendMode(v);
                        });
                }
            }
            if (float v = pImageSlabReslice->GetSlabThickness(); ImGui::SliderFloat("SlabThickness", &v, 0., 20.))
            {
                pImageSlabReslice->SetSlabThickness(v);
            }
        }

        // vtkWidgetRepresentation
        if (auto pResliceCursorRepresentation = vtkResliceCursorRepresentation::SafeDownCast(vtkobj); pResliceCursorRepresentation && ImGui::CollapsingHeader("vtkResliceCursorRepresentation", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (bool v = pResliceCursorRepresentation->GetShowReslicedImage(); ImGui::Checkbox("ShowReslicedImage", &v))
            {
                pResliceCursorRepresentation->SetShowReslicedImage(v);
            }
            if (bool v = pResliceCursorRepresentation->GetRestrictPlaneToVolume(); ImGui::Checkbox("RestrictPlaneToVolume", &v))
            {
                pResliceCursorRepresentation->SetRestrictPlaneToVolume(v);
            }
            if (bool v = pResliceCursorRepresentation->GetDisplayText(); ImGui::Checkbox("DisplayText", &v))
            {
                pResliceCursorRepresentation->SetDisplayText(v);
            }
            if (bool v = pResliceCursorRepresentation->GetUseImageActor(); ImGui::Checkbox("UseImageActor", &v))
            {
                pResliceCursorRepresentation->SetUseImageActor(v);
            }
            {
                double wl[2];
                pResliceCursorRepresentation->GetWindowLevel(wl);
                if (float v[2]{ wl[0], wl[1] }; ImGui::SliderFloat2("WindowLevel", v, 0., 90000.))
                {
                    funcList.push_back([pResliceCursorRepresentation, v]
                        {
                            pResliceCursorRepresentation->SetWindowLevel(v[0], v[1]);
                        });
                }
            }
            ImGui::InputScalarN("GetResliceAxes Matrix0", ImGuiDataType_Double, pResliceCursorRepresentation->GetResliceAxes()->GetData() + 0, 4, nullptr, nullptr, nullptr, ImGuiInputTextFlags_ReadOnly);
            ImGui::InputScalarN("GetResliceAxes Matrix1", ImGuiDataType_Double, pResliceCursorRepresentation->GetResliceAxes()->GetData() + 4, 4, nullptr, nullptr, nullptr, ImGuiInputTextFlags_ReadOnly);
            ImGui::InputScalarN("GetResliceAxes Matrix2", ImGuiDataType_Double, pResliceCursorRepresentation->GetResliceAxes()->GetData() + 8, 4, nullptr, nullptr, nullptr, ImGuiInputTextFlags_ReadOnly);
            ImGui::InputScalarN("GetResliceAxes Matrix3", ImGuiDataType_Double, pResliceCursorRepresentation->GetResliceAxes()->GetData() + 12, 4, nullptr, nullptr, nullptr, ImGuiInputTextFlags_ReadOnly);
            //ImGui::Text("Reslice: %s", pResliceCursorRepresentation->GetReslice()->GetClassName());
            vtkObjImgui("Reslice", pResliceCursorRepresentation->GetReslice(), funcList, ImGuiTreeNodeFlags_DefaultOpen);
            //vtkObjImgui("ImageActor", pResliceCursorRepresentation->GetImageActor(), funcList, ImGuiTreeNodeFlags_DefaultOpen);
            ImGui::Text("ThicknessLabelFormat: %s", pResliceCursorRepresentation->GetThicknessLabelFormat());
        }

        // vtkResliceCursorRepresentation
        if (auto pResliceCursorLineRepresentation = vtkResliceCursorLineRepresentation::SafeDownCast(vtkobj); pResliceCursorLineRepresentation && ImGui::CollapsingHeader("vtkResliceCursorLineRepresentation", ImGuiTreeNodeFlags_DefaultOpen))
        {
            vtkObjImgui("ResliceCursor", pResliceCursorLineRepresentation->GetResliceCursor(), funcList);
            vtkObjImgui("ResliceCursorActor", pResliceCursorLineRepresentation->GetResliceCursorActor(), funcList);
        }
        // vtkResliceCursorLineRepresentation
        if (auto pResliceCursorThickLineRepresentation = vtkResliceCursorThickLineRepresentation::SafeDownCast(vtkobj); pResliceCursorThickLineRepresentation && ImGui::CollapsingHeader("vtkResliceCursorThickLineRepresentation", ImGuiTreeNodeFlags_DefaultOpen))
        {
        }

        // vtkInteractorObserver
        if (auto pAbstractWidget = vtkAbstractWidget::SafeDownCast(vtkobj); pAbstractWidget && ImGui::CollapsingHeader("vtkAbstractWidget", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (bool v = pAbstractWidget->GetManagesCursor(); ImGui::Checkbox("ManagesCursor", &v))
            {
                pAbstractWidget->SetManagesCursor(v);
            }
            if (bool v = pAbstractWidget->GetProcessEvents(); ImGui::Checkbox("ProcessEvents", &v))
            {
                pAbstractWidget->SetProcessEvents(v);
            }
            vtkObjImgui("Representation", pAbstractWidget->GetRepresentation(), funcList, ImGuiTreeNodeFlags_DefaultOpen);
            vtkObjImgui("Parent", pAbstractWidget->GetParent(), funcList);
        }

        // vtkAbstractWidget
        if (auto pResliceCursorWidget = vtkResliceCursorWidget::SafeDownCast(vtkobj); pResliceCursorWidget && ImGui::CollapsingHeader("vtkResliceCursorWidget", ImGuiTreeNodeFlags_DefaultOpen))
        {

        }

        // vtkImageViewer2
        if (auto pResliceImageViewer = vtkResliceImageViewer::SafeDownCast(vtkobj); pResliceImageViewer && ImGui::CollapsingHeader("vtkResliceImageViewer", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (bool v = pResliceImageViewer->GetSliceScrollOnMouseWheel(); ImGui::Checkbox("SliceScrollOnMouseWheel", &v))
            {
                funcList.push_back([pResliceImageViewer, v]
                    {
                        pResliceImageViewer->SetSliceScrollOnMouseWheel(v);
                    });
            }

            if (bool v = pResliceImageViewer->GetThickMode(); ImGui::Checkbox("ThickMode", &v))
            {
                funcList.push_back([pResliceImageViewer, v]
                    {
                        pResliceImageViewer->SetThickMode(v);
                    });
            }

            {
                const char* modeText[] = { "RESLICE_AXIS_ALIGNED", "RESLICE_OBLIQUE" };
                if (auto v = pResliceImageViewer->GetResliceMode(); ImGui::Combo("ResliceMode", &v, modeText, IM_ARRAYSIZE(modeText)))
                {
                    funcList.push_back([pResliceImageViewer, v]
                        {
                            pResliceImageViewer->SetResliceMode(v);
                        });
                }
            }

            //if (float v = pResliceImageViewer->GetSliceScrollFactor(); ImGui::SliderFloat("SliceScrollFactor", &v, 0., 10.))
            //{
            //    pResliceImageViewer->SetSliceScrollFactor(v);
            //}

            vtkObjImgui("ResliceCursor", pResliceImageViewer->GetResliceCursor(), funcList, ImGuiTreeNodeFlags_DefaultOpen);
            vtkObjImgui("ResliceCursorWidget", pResliceImageViewer->GetResliceCursorWidget(), funcList, ImGuiTreeNodeFlags_DefaultOpen);
        }

        ImGui::TreePop();
    }
}