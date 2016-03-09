#pragma once
#include <string>
#include <memory>
#include <unordered_map>
#include "mesh.h"
#include "camera.h"

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
		
		inline std::shared_ptr<CMesh> get_mesh(const std::string &name) const
		{
			auto &it = m_mesh_pool.find(name);
			if (it != m_mesh_pool.end())
				return it->second;
			return nullptr;
		}
		CMesh* create_mesh(const std::string &name);

		inline std::shared_ptr<CCamera> get_camera(const std::string &name) const
		{
			auto &it = m_camera_pool.find(name);
			if (it != m_camera_pool.end())
				return it->second;
			return nullptr;
		}
		CCamera* create_camera(const std::string &name);

		void render();

	private:
		std::unordered_map<std::string, std::shared_ptr<CMesh>> m_mesh_pool;
		std::unordered_map<std::string, std::shared_ptr<CCamera>> m_camera_pool;
	};

} // namespace wyc
