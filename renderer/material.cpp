#include "material.h"

namespace wyc
{
	CMaterial::CMaterial()
	{
		std::string name = "material";
		this->set_uniform("name", name);
		Matrix44f m;
		m.makeIdentity();
		this->set_uniform("mvp_matrix", m);
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