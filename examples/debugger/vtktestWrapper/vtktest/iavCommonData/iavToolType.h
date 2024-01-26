#pragma once

enum class ToolType
{
    // 通用工具
    NORMAL,                             // 默认工具
    LINE,                               // 直线
    POLY_LINE,                          // 多线段
    ANGLE,                              // 角度
    ARROW,                              // 箭头
    TEXT,                               // 文字
    FREE_DRAWING,                       // 自由画图
    ROI,                                // ROI

    // CPR工具
    CPR_DRAW_ARCH,                      // 绘制牙弓线
    CPR_ROI,                            // CPR的ROI调节
    CPR_DRAW_Nerve,                     // CPR绘制神经管
    CPR_DRAW_IMPLANT,                   // CPR绘制种植体

    // TMJ
    TMJ_DRAW,                           // TMJ绘制
    TMJ_SLICE,                          // TMJ SLICE提取
    TMJ_SHOWSLICE,                      //  TMJ SLICE显示
    TMJ_ROI,                            // TMJ ROI调节
};