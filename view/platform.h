#pragma once

// platform specific headers
#ifdef WIN32
#include "min_windows.h"
#endif 

// type definition
#ifdef WIN32
using os_window = HWND;
#else
using os_window = void*;
#endif