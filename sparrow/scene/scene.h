#pragma once
#include <string>
#include <memory>
#include <unordered_map>
#include "mesh.h"
#include "camera.h"
#include "renderer.h"
#include "material.h"
#include "flat_color.h"
#include "scene_obj.h"

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
		
		std::shared_ptr<CMesh> create_mesh(const std::string &name);
		inline std::shared_ptr<CMesh> get_mesh(const std::string &name) const
		{
			auto &it = m_mesh_pool.find(name);
			if (it != m_mesh_pool.end())
				return it->second;
			return nullptr;
		}

		material_ptr create_material(const std::string &name, const std::string &type);
		inline material_ptr get_material(const std::string &name) const
		{
			auto &it = m_material_lib.find(name);
			if (it != m_material_lib.end())
				return it->second;
			return nullptr;
		}

		std::shared_ptr<CCamera> create_camera(const std::string &name);
		inline std::shared_ptr<CCamera> get_camera(const std::string &name) const
		{
			auto &it = m_camera_pool.find(name);
			if (it != m_camera_pool.end())
				return it->second;
			return nullptr;
		}
		inline void set_active_camera(const std::string &name) {
			m_active_camera = name;
		}
		inline std::shared_ptr<CCamera> get_active_camera() const {
			return get_camera(m_active_camera);
		}

		std::shared_ptr<CSceneObj> add_object(const std::string &mesh_name, const Matrix44f &transform);
		std::shared_ptr<CSceneObj> get_object(unsigned pid);

		void render(std::shared_ptr<CRenderer> renderer);

	private:
		std::unordered_map<std::string, std::shared_ptr<CMesh>> m_mesh_pool;
		std::unordered_map<std::string, std::shared_ptr<CCamera>> m_camera_pool;
		std::unordered_map<unsigned, std::shared_ptr<CSceneObj>> m_objs;
		std::unordered_map<std::string, material_ptr> m_material_lib;
		unsigned m_cur_pid;
		std::string m_active_camera;
	};

} // namespace wyc
