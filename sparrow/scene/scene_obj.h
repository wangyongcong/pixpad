#pragma once
#include "renderer.h"

namespace wyc
{
	class CScene;
	class CCamera;
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
		virtual void render(CRenderer* renderer, const CCamera* camera);

		inline void set_name(const char* name) {
			m_name = name;
		}
		inline void set_name(const std::string &name) {
			m_name = name;
		}
		inline const std::string& get_name() const {
			return m_name;
		}
		inline void set_transform(const Imath::M44f& transform) {
			m_transform = transform;
		}
		inline const Imath::M44f& get_transform() const {
			return m_transform;
		}
		inline void join_scene(CScene *scn) {
			m_scene = scn;
		}

	protected:
		unsigned m_pid;
		std::string m_name;
		Imath::M44f m_transform;
		CScene* m_scene;
	};

} // namespace wyc
