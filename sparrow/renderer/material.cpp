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

	const AttribDefine & CMaterial::get_attrib_define() const
	{
		static AttribDefine ls_attrib_define;
		return ls_attrib_define;
	}

	const UniformMap & CMaterial::get_uniform_define() const
	{
		static UniformMap ls_material_uniform;
		return ls_material_uniform;
	}

	AttribDefine CMaterial::create_attrib_define() const
	{
		auto in_lst = get_attrib_input();
		auto out_lst = get_attrib_ouput();
		unsigned in_size = 0, out_size = 0;
		unsigned in_comp = 0, out_comp = 0;
		if (in_lst) {
			for (auto it = in_lst; it->component > 0; ++it)
			{
				in_size += 1;
				in_comp += it->component;
			}
		}
		if (out_lst) {
			for (auto it = out_lst; it->component > 0; ++it)
			{
				out_size += 1;
				out_comp += it->component;
			}
		}
		AttribDefine attrib_define = {
			// attribute list, attribute count, all attributes' component count
			in_lst, in_size, in_comp, 
			out_lst, out_size, out_comp,
		};
		return attrib_define;
	}

} // namespace wyc