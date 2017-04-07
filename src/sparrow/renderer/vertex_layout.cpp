#include "vertex_layout.h"

namespace wyc 
{

	const VertexAttrib VertexP3C3::Layout::attr_table[] = 
	{
		VertexAttrib{ ATTR_POSITION, 3, offsetof(VertexP3C3, pos) },
		VertexAttrib{ ATTR_COLOR, 3, offsetof(VertexP3C3, color) },
	};

	const VertexAttrib VertexP4C3::Layout::attr_table[] =
	{
		VertexAttrib{ ATTR_POSITION, 3, offsetof(VertexP4C3, pos) },
		VertexAttrib{ ATTR_COLOR, 3, offsetof(VertexP4C3, color) },
	};

	const VertexAttrib VertexP3Tex2N3::Layout::attr_table[] =
	{
		VertexAttrib{ ATTR_POSITION, 3, offsetof(VertexP3Tex2N3, pos) },
		VertexAttrib{ ATTR_TEXTURE, 2, offsetof(VertexP3Tex2N3, uv) },
		VertexAttrib{ ATTR_NORMAL, 3, offsetof(VertexP3Tex2N3, normal) },
	};

	const VertexAttrib VertexTex2N3::Layout::attr_table[] =
	{
		VertexAttrib{ ATTR_TEXTURE, 2, offsetof(VertexTex2N3, uv) },
		VertexAttrib{ ATTR_NORMAL, 3, offsetof(VertexTex2N3, normal) },
	};

} // namespace wyc