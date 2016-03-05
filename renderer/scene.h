#pragma once
#include <string>
#include <memory>
#include <unordered_map>
#include "mesh.h"

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

	private:
		std::unordered_map<std::string, std::shared_ptr<CMesh>> m_mesh_pool;
	};

} // namespace wyc
