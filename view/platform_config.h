#pragma once

// platform specific headers
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif 

// type definition
#ifdef WIN32
using WINDOW_HANDLE = HWND;
using INSTANCE_HANDLE = HINSTANCE;
#else
using WINDOW_HANDLE = void*;
using INSTANCE_HANDLE = void*;
#endif