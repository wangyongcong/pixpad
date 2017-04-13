#pragma once
#include <type_traits>
#include <OpenEXR/ImathVec.h>
#include <OpenEXR/ImathColor.h>

namespace wyc
{

	enum EAttribUsage
	{
		ATTR_POSITION = 0,
		ATTR_COLOR,
		ATTR_UV0,
		ATTR_UV1,
		ATTR_NORMAL,
		ATTR_TANGENT,
		ATTR_USAGE_0,
		ATTR_USAGE_1,
		ATTR_USAGE_2,
		ATTR_USAGE_3,
		ATTR_MAX_COUNT,
	};

	struct VertexAttrib
	{
		EAttribUsage usage;
		uint8_t component;
		uint16_t offset;
	};

#define VERTEX_LAYOUT(comp, pos, attr)	struct Layout {\
	public:\
		static const unsigned component = comp;\
		static const unsigned offset_pos = pos;\
		static const unsigned attr_count = attr;\
		static const VertexAttrib attr_table[attr];\
	};

	struct VertexP3C3
	{
		Imath::V3f pos;
		Imath::C3f color;
		VERTEX_LAYOUT(6, 0, 2)
	};

	struct VertexP4C3
	{
		Imath::V4f pos;
		Imath::C3f color;
		VERTEX_LAYOUT(7, 0, 2)
	};

	struct VertexP3Tex2N3
	{
		Imath::V3f pos;
		Imath::V2f uv;
		Imath::V3f normal;
		VERTEX_LAYOUT(8, 0, 3)
	};

	struct VertexTex2N3
	{
		Imath::V2f uv;
		Imath::V3f normal;
		VERTEX_LAYOUT(5, 0, 2)
	};

} // namespace wyc