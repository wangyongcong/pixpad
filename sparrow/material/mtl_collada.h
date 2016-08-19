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

		virtual void vertex_shader(const void *vertex_in, void *vertex_out) const override
		{

		}

		virtual bool fragment_shader(const void *vertex_out, Color4f &frag_color) const override
		{

		}

	protected:
		Imath::M44f mvp_matrix;
		Imath::C4f emission, ambient, diffuse, specular;
		float shininess;
	};


} // namespace wyc