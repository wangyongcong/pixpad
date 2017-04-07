#include "static_mesh.h"
#include "material.h"
#include "mesh.h"
#include "camera.h"
#include "scene.h"

namespace wyc
{
	CStaticMesh::CStaticMesh()
		: CSceneObj()
		, m_mesh(nullptr)
		, m_material(nullptr)
	{
	}

	void CStaticMesh::render(CRenderer* renderer, const CCamera* camera)
	{
		log_debug("render: %s", m_name.c_str());
		if (!m_mesh || !m_material)
			return;
		Imath::M44f mvp = camera->get_view_projection() * m_transform;
		m_material->set_uniform("mvp_matrix", mvp);
		auto lights = m_scene->get_mtl_lights();
		m_material->set_uniform("lights", lights);
		auto cmd = renderer->new_command<cmd_draw_mesh>();
		cmd->mesh = m_mesh.get();
		cmd->material = m_material.get();
		renderer->enqueue(cmd);
	}

} // namespace wyc