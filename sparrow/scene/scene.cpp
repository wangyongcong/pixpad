#include "scene.h"
#include "util.h"
#include "log.h"

namespace wyc
{
	CScene::CScene()
		: m_cur_pid(0)
	{
	}

	CScene::~CScene()
	{
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

	void CScene::add_object(std::shared_ptr<CSceneObj> obj)
	{
		if (obj->get_pid() != 0)
			return;
		m_cur_pid += 1;
		obj->set_pid(m_cur_pid);
		m_objs[m_cur_pid] = obj;
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