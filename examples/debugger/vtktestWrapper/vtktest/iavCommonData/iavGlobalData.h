#pragma once

#include <iavMacrosCommonData.h>

class vtkImageData;

namespace iavGlobalData
{
    constexpr double rendererBackground[] = { 0x36 / 255., 0x37 / 255., 0x39 / 255. };

    IAVCOMMONDATA_API void setImageData(vtkImageData* v);
    IAVCOMMONDATA_API vtkImageData* getImageData();
}