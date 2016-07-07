#include "scene_obj.h"
#include "flat_color.h"

namespace wyc
{
	CSceneObj::CSceneObj(unsigned pid)
		: m_pid(pid)
		, m_mesh(nullptr)
		, m_is_material_changed(false)
	{
		m_transform.makeIdentity();
	}

	CSceneObj::~CSceneObj()
	{
		m_mesh = nullptr;
	}

	void CSceneObj::render(std::shared_ptr<CRenderer> renderer, std::shared_ptr<CCamera> camera)
	{
		log_debug("render: %s", m_name.c_str());
		if (!m_mesh)
			return;
		if (!m_material)
			load_default_material();
		Matrix44f mvp = m_transform * camera->get_view_projection();
		m_material->set_uniform("mvp_matrix", mvp);
		auto cmd = renderer->new_command<cmd_draw_mesh>();
		cmd->mesh = m_mesh.get();
		cmd->material = m_material.get();
		renderer->enqueue(cmd);
	}

	void CSceneObj::load_default_material()
	{
		CMaterialFlatColor *mateiral = new CMaterialFlatColor();
		Color4f color = { 0.0f, 1.0f, 0.0f, 1.0f };
		mateiral->set_uniform("color", color);
		m_material = material_ptr(mateiral);
		m_is_material_changed = true;
	}

} // namespace wyc