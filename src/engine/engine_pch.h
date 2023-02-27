#pragma once

#if defined(PLATFORM_WINDOWS) || defined(WIN32) 
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <shellapi.h> // For CommandLineToArgvW

// The min/max macros conflict with like-named member functions.
// Only use std::min and std::max defined in <algorithm>.
#if defined(min)
#undef min
#endif

#if defined(max)
#undef max
#endif

// In order to define a function called create, the Windows macro needs to
// be undefined.
#if defined(CreateWindow)
#undef CreateWindow
#endif

// Windows Runtime Library. Needed for Microsoft::WRL::ComPtr<> template class.
#include <wrl.h>
using namespace Microsoft::WRL;
#endif

#ifdef RENDERER_DIRECT3D12
// DirectX 12 specific headers.
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

// D3D12 extension library.
#include <d3dx12.h>
#endif

// STL headers
#include <string>
#include <algorithm>
#include <cassert>
#include <chrono>
#include <map>
#include <memory>

// rtm
#include "rtm/constants.h"
#include "rtm/scalarf.h"
#include "rtm/vector4f.h"
#include "rtm/matrix3x3f.h"
#include "rtm/matrix3x4f.h"
#include "rtm/matrix4x4f.h"
#include "rtm/quatf.h"
#include "rtm/qvvf.h"
#include "rtm/mask4f.h"