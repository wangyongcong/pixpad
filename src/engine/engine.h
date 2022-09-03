#pragma once

#ifdef WIN32
#ifdef engine_EXPORTS
	#define WYCAPI __declspec(dllexport)
#else
	#define WYCAPI __declspec(dllimport)
#endif
#else
#define WYCAPI
#endif

