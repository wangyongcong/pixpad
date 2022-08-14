#pragma once
#include "game_framework.h"

namespace wyc
{

	class IRenderer;

	class GAME_FRAMEWORK_API IGameInstance
	{
	public:
		virtual void initialize() = 0;
		virtual void exit() = 0;
		virtual void tick(float deltaTime) = 0;
		virtual void draw(IRenderer* pRenderer) = 0;
	};
}