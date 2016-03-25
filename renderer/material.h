#pragma once
#include <string>
#include <OpenEXR/ImathMatrix.h>
#include "shader.h"

#define PROPERTY
#define PUBLIC_PROPERTY

namespace wyc
{
	class CMaterial
	{
	public:
		CMaterial();
		virtual ~CMaterial();
	
	public:
		// public material property
		std::string name;
		shader_ptr shader;
		Matrix44f mvp_matrix;
	};

	typedef std::shared_ptr<CMaterial> material_ptr;

} // namespace wyc