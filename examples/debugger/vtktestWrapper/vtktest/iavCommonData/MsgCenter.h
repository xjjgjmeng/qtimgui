#pragma once

#include <functional>
#include <any>

#include <iavMacrosCommonData.h>

namespace MsgCenter
{
    enum Msg : int
    {
        // Common
        GlobalImageLoaded,              // 全局使用的影像数据已加载

        // CPR
        CprAxialSliceChanged,           // CPR横截面的slice发生切换

        CprUpdateSpline,
        CprUpdateCurvePlane,
        CprUpdatePanCursorLineTranslate,
        CprUpdatePanCursorLineRotate,
        CprSplineFinishEditing,
        CprSliceIntervalChanged,
        CprRoiAdjust,                   // 通知进入ROI调节Style
        CprRoiChanged,                  // ROI调节结束
        CprToolNormal,
        CprToolDrawArch,                // cpr编辑牙弓线编辑牙弓线工具
        CprToolLine,                    // cpr直线测量工具
        CprDrawNerve,                   // cpr绘制神经管
        CprUpdataNerveTube,             // cpr更新神经管
        CprDrawImplant,                 // cpr绘制种植体
        CprUpdateImplant,               // cpr更新种植体
        CprUpdateImplantPosition,       // cpr更新种植体位置(旋转，平移)

        // TMJ
        TMJToolDrawLine,                // draw crosshair
        TMJROI,                         // 调节ROI    
        TMJVolumeClipRequest,           // 光标完成，开始切割
        TMJShowImageViewer,             
        TMJShowRenderingVolume,
        TMJSliceIntervalChanged,        // slice的间距被用户通过UI调节
        TMJAxialSliceChanged,

        TMJLeftTopCursorLineChanged,
        TMJLeftBottomCursorLineChanged,
        TMJRightTopCursorLineChanged,
        TMJRightBottomCursorLineChanged,
    };
}

namespace MsgCenter
{
    using callback_t = std::function<void(const Msg, const std::any&)>;

    IAVCOMMONDATA_API void send(const Msg msg, const std::any& data = std::any{});
    IAVCOMMONDATA_API void attach(void* id, const callback_t& cb);
    IAVCOMMONDATA_API void detach(void* id);
}