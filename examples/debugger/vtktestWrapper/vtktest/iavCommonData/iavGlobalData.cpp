#include "iavGlobalData.h"
#include "MsgCenter.h"

namespace InnerData
{
    vtkImageData* pImage = nullptr;
}

namespace iavGlobalData
{
    void setImageData(vtkImageData* v)
    {
        InnerData::pImage = v;
        MsgCenter::send(MsgCenter::GlobalImageLoaded);
    }

    vtkImageData* getImageData()
    {
        return InnerData::pImage;
    }
}