#pragma once

#if defined(_WINDOWS) && defined(ENGINE_SHALRED_LIBS)
#ifdef engine_EXPORTS
	#define WYCAPI __declspec(dllexport)
#else
	#define WYCAPI __declspec(dllimport)
#endif
#else
	#define WYCAPI
#endif
