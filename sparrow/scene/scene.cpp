#include "scene.h"
#include "util.h"
#include "log.h"

namespace wyc
{
	CScene::CScene()
		: m_cur_pid(0)
		, m_active_camera(nullptr)
	{
	}

	CScene::~CScene()
	{
	}

	void CScene::add_camera(std::shared_ptr<CCamera> camera)
	{
		_add_object(camera.get());
		m_cameras[m_cur_pid] = camera;
	}

	std::shared_ptr<CCamera> CScene::get_camera(unsigned pid) const
	{
		auto it = m_cameras.find(pid);
		if (it != m_cameras.end())
			return it->second;
		return nullptr;
	}

	void CScene::set_active_camera(std::shared_ptr<CCamera> camera)
	{
		unsigned pid = camera->get_pid();
		if (m_cameras.find(pid) == m_cameras.end())
		{
			throw std::exception("Camera not in scene");
		}
		m_active_camera = camera;
	}

	void CScene::add_object(std::shared_ptr<CSceneObj> obj)
	{
		_add_object(obj.get());
		m_objs[m_cur_pid] = obj;
	}

	std::shared_ptr<CSceneObj> CScene::get_object(unsigned pid)
	{
		auto it = m_objs.find(pid);
		if (it != m_objs.end())
			return it->second;
		return nullptr;
	}

	void CScene::add_light(std::shared_ptr<CLight> light)
	{
		_add_object(light.get());
		m_lights[m_cur_pid] = light;
	}

	std::shared_ptr<CLight> CScene::get_light(unsigned pid)
	{
		auto it = m_lights.find(pid);
		if (it != m_lights.end()) 
			return it->second;
		return nullptr;
	}

	void CScene::render(std::shared_ptr<CRenderer> renderer)
	{
		CSceneObj *obj;
		auto camera = get_active_camera();
		if (!camera)
			return;
		camera->update_transform();
		CRenderer *rd = renderer.get();
		CCamera *cam = camera.get();
		for (auto it : m_objs)
		{
			obj = it.second.get();
			obj->render(rd, cam);
		}
	}

	bool CScene::_add_object(CSceneObj * obj)
	{
		if (obj->get_pid() != 0)
			return false;
		m_cur_pid += 1;
		obj->set_pid(m_cur_pid);
		obj->join_scene(this);
		return true;
	}

} // namespace wyc