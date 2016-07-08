#include "material.h"

namespace wyc
{
	CMaterial::CMaterial()
	{
	}

	CMaterial::CMaterial(const char * name)
		: m_name(name)
	{
	}

	CMaterial::~CMaterial()
	{
	}

	const CUniform * CMaterial::find_uniform(const std::string & name) const
	{
		const auto &uniform_map = get_uniform_define();
		auto it = uniform_map.find(name);
		if (it == uniform_map.end())
			return nullptr;
		return &it->second;
	}

} // namespace wyc