#pragma once
#include "material.h"

namespace wyc
{
	class CMaterialCollada : public CMaterial
	{
		INPUT_ATTRIBUTE_LIST
			ATTRIBUTE_SLOT(ATTR_POSITION, 3)
			ATTRIBUTE_SLOT(ATTR_NORMAL, 3)
		INPUT_ATTRIBUTE_LIST_END

		OUTPUT_ATTRIBUTE_LIST
			ATTRIBUTE_SLOT(ATTR_POSITION, 4)
			ATTRIBUTE_SLOT(ATTR_NORMAL, 3)
		OUTPUT_ATTRIBUTE_LIST_END
		
		UNIFORM_MAP
			UNIFORM_SLOT(Imath::M44f, mvp_matrix)
			UNIFORM_SLOT(Imath::C4f, emission)
			UNIFORM_SLOT(Imath::C4f, ambient)
			UNIFORM_SLOT(Imath::C4f, diffuse)
			UNIFORM_SLOT(Imath::C4f, specular)
			UNIFORM_SLOT(float, shininess)
			UNIFORM_SLOT(unsigned, light_count)
			UNIFORM_SLOT(const MtlLight*, lights)
		UNIFORM_MAP_END

	public:
		CMaterialCollada()
			: CMaterial("Collada")
			, emission(0, 0, 0, 1)
			, ambient(0, 0, 0, 1)
			, diffuse(0, 0, 0, 1)
			, specular(0, 0, 0, 1)
			, shininess(50)
		{
			mvp_matrix.makeIdentity();
		}

		struct VertexIn
		{
			Imath::V3f *position;
			Imath::V3f *normal;
		};

		struct VertexOut
		{
			Imath::V4f position;
			Imath::V3f normal;
		};

		virtual void vertex_shader(const void *vertex_in, void *vertex_out) const override
		{
			const VertexIn *v0 = (const VertexIn*)vertex_in;
			VertexOut *vout = (VertexOut*)vertex_out;
		}

		virtual bool fragment_shader(const void *vertex_out, Color4f &frag_color) const override
		{
			const VertexOut *vert_out = (const VertexOut*)vertex_out;
			return false;
		}

	protected:
		Imath::M44f mvp_matrix;
		Imath::C4f emission, ambient, diffuse, specular;
		float shininess;
		unsigned light_count;
		const MtlLight* lights;
	};


} // namespace wyc