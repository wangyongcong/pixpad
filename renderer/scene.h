#pragma once
#include <string>

namespace wyc
{
	class CScene
	{
	public:
		CScene();
		~CScene();
		bool load_collada(const std::wstring &file);
	};

} // namespace wyc
