#include "material.h"

namespace wyc
{
	CMaterial::CMaterial()
		: shader(nullptr)
	{
		mvp_matrix.makeIdentity();
	}

	bool CMaterial::bind_vertex(const CVertexBuffer & vb)
	{
		auto &attrib_define = get_attrib_define();
		m_attrib_stream.resize(attrib_define.attrib_count);
		for (auto i = 0; i < attrib_define.attrib_count; ++i)
		{
			const AttribSlot &slot = attrib_define.attrib_slots[i];
			if (vb.has_attribute(slot.usage)
				|| vb.attrib_component(slot.usage) < slot.component)
				return false;
			m_attrib_stream[i] = {
				(const char*)vb.attrib_stream(slot.usage), 
				vb.attrib_stride(slot.usage), 
			};
		}
		return true;
	}

	CMaterial::~CMaterial()
	{
		shader = nullptr;
	}

} // namespace wyc