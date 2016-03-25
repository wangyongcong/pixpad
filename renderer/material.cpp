#include "material.h"

namespace wyc
{
	CMaterial::CMaterial()
		: shader(nullptr)
	{
		mvp_matrix.makeIdentity();
	}

	CMaterial::~CMaterial()
	{
		shader = nullptr;
	}

} // namespace wyc