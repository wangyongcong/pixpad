#include "scene.h"
#include <common/util.h>
#include <common/log.h>
#include <sparrow/shader/flat_color.h>

namespace wyc
{
	CScene::CScene()
		: m_cur_pid(0)
	{
	}

	CScene::~CScene()
	{
	}

	std::shared_ptr<CMesh> CScene::create_mesh(const std::string & name)
	{
		auto it = m_mesh_pool.find(name);
		if(it == m_mesh_pool.end())
		{
			std::shared_ptr<CMesh> mesh(new CMesh);
			m_mesh_pool[name] = mesh;
			return mesh;
		}
		debug("%s : The mesh with name [%s] already exists", __FUNCTION__, name.c_str());
		return it->second;
	}

	std::shared_ptr<CCamera> CScene::create_camera(const std::string & name)
	{
		auto it = m_camera_pool.find(name);
		if (it == m_camera_pool.end())
		{
			std::shared_ptr<CCamera> camera(new CCamera);
			m_camera_pool[name] = camera;
			return camera;
		}
		debug("%s : The mesh with name [%s] already exists", __FUNCTION__, name.c_str());
		return it->second;
	}

	std::shared_ptr<CSceneObj> CScene::add_object(const std::string & mesh_name, const Matrix44f & transform)
	{
		m_cur_pid += 1;
		auto obj = std::shared_ptr<CSceneObj>(new CSceneObj(m_cur_pid));
		m_objs[m_cur_pid] = obj;
		auto it = m_mesh_pool.find(mesh_name);
		if(it != m_mesh_pool.end())
			obj->set_mesh(it->second);
		obj->set_transform(transform);
		return obj;
	}

	std::shared_ptr<CSceneObj> CScene::get_object(unsigned pid)
	{
		auto it = m_objs.find(pid);
		if (it != m_objs.end())
			return it->second;
		return nullptr;
	}

	void CScene::render(std::shared_ptr<CRenderer> renderer)
	{
		CSceneObj *obj;
		auto camera = get_active_camera();
		Matrix44f world_to_camera = camera->get_transform().inverse();
		world_to_camera *= camera->get_projection();
		CShaderFlatColor material;
		for (auto it : m_objs)
		{
			obj = it.second.get();
			material.m_uniform.mvp = obj->get_transform() * world_to_camera;
			obj->render(renderer);
		}
	}

} // namespace wyc