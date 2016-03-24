#pragma once
#include "shader.h"

#define MATERIAL_PROPERTY(type, name) type name

namespace wyc
{
	class CMaterial
	{
	public:
		CMaterial();
		virtual ~CMaterial();
		inline shader_ptr get_shader() const {
			return m_shader;
		}

	protected:
		shader_ptr m_shader;
	};

	typedef std::shared_ptr<CMaterial> material_ptr;

} // namespace wyc