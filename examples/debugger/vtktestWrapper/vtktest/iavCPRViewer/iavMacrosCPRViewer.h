#pragma once

#ifdef IAVCPRVIEWER_DLL_EXPORTS
#define IAVCPRVIEWER_API __declspec(dllexport)
#else
#define IAVCPRVIEWER_API __declspec(dllimport)
#endif