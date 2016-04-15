#include "flat_color.h"
#include <mathex/ImathVecExt.h>
#include <mathex/ImathMatrixExt.h>

namespace wyc 
{
	bool CMaterialFlatColor::bind_vertex(const CVertexBuffer & vb)
	{
		if (vb.has_attribute(ATTR_POSITION) 
			|| vb.attrib_component(ATTR_POSITION) < 3)
			return false;
		m_input.pos_stream = reinterpret_cast<const Vec3f*>(vb.attrib_stream(ATTR_POSITION));
		m_input.pos_stride = vb.attrib_stride(ATTR_POSITION);
		m_input.count = vb.size();
		return true;
	}

	void CMaterialFlatColor::vertex_shader(const float * vertex_in, float * vertex_out, Vec4f & clip_pos) const
	{
	}

	bool CMaterialFlatColor::fragment_shader(const float * vertex_out, Color4f & frag_color) const
	{
		return false;
	}

} // namespace wyc