#include "material.h"

namespace wyc
{
	CMaterial::CMaterial()
		: m_shader(nullptr)
	{
	}

	CMaterial::~CMaterial()
	{
		m_shader = nullptr;
	}

} // namespace wyc