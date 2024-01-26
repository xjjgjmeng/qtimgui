#pragma once

#ifdef IAVTMJVIEWER_DLL_EXPORTS
#define IAVTMJVIEWER_API __declspec(dllexport)
#else
#define IAVTMJVIEWER_API __declspec(dllimport)
#endif