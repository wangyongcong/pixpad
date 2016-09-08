#pragma once
#include <string>
#include <memory>
#include <unordered_map>
#include "renderer.h"
#include "scene_obj.h"
#include "camera.h"
#include "light.h"

namespace wyc
{
	class CScene
	{
	public:
		CScene();
		~CScene();
		CScene(const CScene* &scn) = delete;
		CScene& operator = (const CScene* &scn) = delete;

		bool load_collada(const std::wstring &file);
		
		void add_camera(std::shared_ptr<CCamera> camera);
		std::shared_ptr<CCamera> get_camera(unsigned pid) const;
		void set_active_camera(std::shared_ptr<CCamera> camera);
		inline std::shared_ptr<CCamera> get_active_camera() const {
			return m_active_camera;
		}

		void add_object(std::shared_ptr<CSceneObj> obj);
		std::shared_ptr<CSceneObj> get_object(unsigned pid);

		void add_light(std::shared_ptr<CLight> light);
		std::shared_ptr<CLight> get_light(unsigned pid);

		void render(std::shared_ptr<CRenderer> renderer);

	private:
		bool _add_object(CSceneObj *obj);

		unsigned m_cur_pid;
		std::shared_ptr<CCamera> m_active_camera;
		std::unordered_map<unsigned, std::shared_ptr<CCamera>> m_cameras;
		std::unordered_map<unsigned, std::shared_ptr<CSceneObj>> m_objs;
		std::unordered_map<unsigned, std::shared_ptr<CLight>> m_lights;
		MtlUniformData m_udata;
	};

} // namespace wyc
