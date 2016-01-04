#pragma once
#include "OpenEXR/ImathVec.h"
#include "OpenEXR/ImathColor.h"

namespace wyc
{
	enum EVertexLayout
	{
		VF_NONE = 0,
		VF_P3C3,
		VF_P3S2,
		VF_P3N3,
		VF_P3C3N3,
		VF_P3S2N3,
	};

	template<EVertexLayout Layout>
	struct CVertexLayout
	{
		using vertex_t = void;
		//static const int attr_count = 0;
		//static const VertexAttrib *attr_table;
	};

	struct VertexP3C3
	{
		Imath::V3f pos;
		Imath::C3f color;
	};

	template<>
	struct CVertexLayout<VF_P3C3>
	{
		using vertex_t = VertexP3C3;
		static const int attr_count = 2;
		static const VertexAttrib attr_table[attr_count];
	};

	struct VertexP4C3
	{
		Imath::V4f pos;
		Imath::C3f color;
	};

	struct VertexP3S2
	{
		Imath::V3f pos;
		Imath::V2f uv;
	};

	struct VertexP3C3N3
	{
		Imath::V3f pos;
		Imath::C3f color;
		Imath::V3f normal;
	};

	struct VertexP3S2N3
	{
		Imath::V3f pos;
		Imath::V2f uv;
		Imath::V3f normal;
	};

#define LAYOUT(f) \
	template<>\
	struct CVertexLayout<VF_##f>\
	{\
		using vertex_t = Vertex##f;\
	}

	LAYOUT(P3S2);
	LAYOUT(P3C3N3);
	LAYOUT(P3S2N3);

} // namespace wyc