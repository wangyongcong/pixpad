#pragma once
#include <string>
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
		inline shader_ptr get_shader() const {
			return m_shader;
		}
		inline void set_shader(shader_ptr shader) {
			m_shader = shader;
		}
	
	public:
		// public material property
		std::string name;

	protected:
		shader_ptr m_shader;
	};

	typedef std::shared_ptr<CMaterial> material_ptr;

} // namespace wyc