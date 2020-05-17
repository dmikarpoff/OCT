#pragma once

#include <windows.h>

#ifdef OCT_UTILS_EXPORTS

#define OCT_UTILS_API __declspec(dllexport)

#else

#define OCT_UTILS_API __declspec(dllimport)

#endif
