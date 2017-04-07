#pragma once
#include "scene_obj.h"

namespace wyc
{
	class CLight : public CSceneObj
	{
	public:
		CLight();
		inline void set_color(const Imath::C3f &c) {
			m_color = c;
		}
		inline const Imath::C3f& get_color() const {
			return m_color;
		}
		inline void set_intensity(float i) {
			m_intensity = i;
		}
		inline float get_intensity() const {
			return m_intensity;
		}

	protected:
		Imath::C3f m_color;
		float m_intensity;
	};

} // namespace wyc