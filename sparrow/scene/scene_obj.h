#pragma once
#include "renderer.h"
#include "camera.h"

namespace wyc
{
	class CSceneObj
	{
	public:
		CSceneObj();
		virtual ~CSceneObj();
		inline void set_pid(unsigned pid) {
			m_pid = pid;
		}
		inline unsigned get_pid() const {
			return m_pid;
		}
		virtual void render(std::shared_ptr<CRenderer> renderer, std::shared_ptr<CCamera> camera);

		inline void set_name(const char* name) {
			m_name = name;
		}
		inline void set_name(const std::string &name) {
			m_name = name;
		}
		inline const std::string& get_name() const {
			return m_name;
		}
		inline void set_transform(const Matrix44f& transform) {
			m_transform = transform;
		}
		inline const Matrix44f& get_transform() const {
			return m_transform;
		}

	protected:
		unsigned m_pid;
		std::string m_name;
		Matrix44f m_transform;
	};

} // namespace wyc
