#pragma once

#ifdef engine_EXPORTS
	#define GAME_FRAMEWORK_API __declspec(dllexport)
#else
	#define GAME_FRAMEWORK_API __declspec(dllimport)
#endif
