#include "scene.h"
#include <common/util.h>
#include <common/log.h>

namespace wyc
{
	CScene::CScene()
	{
	}

	CScene::~CScene()
	{
	}

	CMesh * CScene::create_mesh(const std::string & name)
	{
		auto it = m_mesh_pool.find(name);
		if(it == m_mesh_pool.end())
		{
			std::shared_ptr<CMesh> mesh(new CMesh);
			m_mesh_pool[name] = mesh;
			return mesh.get();
		}
		debug("%s : The mesh with name [%s] already exists", __FUNCTION__, name.c_str());
		return it->second.get();
	}

} // namespace wyc