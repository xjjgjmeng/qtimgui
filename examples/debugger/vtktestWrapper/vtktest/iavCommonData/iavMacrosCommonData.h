#pragma once

#ifdef IAVCOMMONDATA_DLL_EXPORTS
#define IAVCOMMONDATA_API __declspec(dllexport)
#else
#define IAVCOMMONDATA_API __declspec(dllimport)
#endif