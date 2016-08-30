#include "scene.h"
#include "util.h"
#include "log.h"
#include "mtl_flat_color.h"
#include "mtl_collada.h"

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
			auto mesh = std::make_shared<CMesh>();
			m_mesh_pool[name] = mesh;
			return mesh;
		}
		log_warn("%s : The mesh with name [%s] already exists", __FUNCTION__, name.c_str());
		return it->second;
	}

	material_ptr CScene::create_material(const std::string & name, const std::string &type)
	{
		auto it = m_material_lib.find(name);
		if (it != m_material_lib.end())
		{
			log_warn("%s : The material with name [%s] already exists", __FUNCTION__, name.c_str());
			return it->second;
		}
		// todo: we need a material factory
		material_ptr mtl;
		if (type == "FlatColor")
		{
			mtl = std::make_shared<CMaterialFlatColor>();
		}
		else if (type == "Collada")
		{
			mtl = std::make_shared<CMaterialCollada>();
		}
		else
		{
			log_error("%s : Unknown material [%s]", __FUNCTION__, type.c_str());
			return nullptr;
		}
		m_material_lib[name] = mtl;
		return mtl;
	}

	bool CScene::add_material(const std::string &name, material_ptr mtl)
	{
		if (m_material_lib.find(name) != m_material_lib.end())
			return false;
		m_material_lib[name] = mtl;
		return true;
	}

	std::shared_ptr<CCamera> CScene::create_camera(const std::string & name)
	{
		auto it = m_camera_pool.find(name);
		if (it == m_camera_pool.end())
		{
			auto camera = std::make_shared<CCamera>();
			m_camera_pool[name] = camera;
			return camera;
		}
		log_debug("%s : The camera with name [%s] already exists", __FUNCTION__, name.c_str());
		return it->second;
	}

	std::shared_ptr<CSceneObj> CScene::add_object(const std::string & mesh_name, const Matrix44f & transform)
	{
		m_cur_pid += 1;
		auto obj = std::make_shared<CSceneObj>(m_cur_pid);
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
		camera->update_transform();
		for (auto it : m_objs)
		{
			obj = it.second.get();
			obj->render(renderer, camera);
		}
	}

} // namespace wyc