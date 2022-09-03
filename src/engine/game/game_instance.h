#pragma once
#include "engine.h"

namespace wyc
{
	class IRenderer;

	class WYCAPI IGameInstance
	{
	public:
		virtual void initialize() = 0;
		virtual void exit() = 0;
		virtual void tick(float deltaTime) = 0;
		virtual void draw(IRenderer* pRenderer) = 0;
	};
}