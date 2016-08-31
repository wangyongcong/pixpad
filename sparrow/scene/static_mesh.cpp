#include "static_mesh.h"
#include "material.h"
#include "mesh.h"

namespace wyc
{
	CStaticMesh::CStaticMesh()
		: CSceneObj()
		, m_mesh(nullptr)
		, m_material(nullptr)
	{
	}

	void CStaticMesh::render(std::shared_ptr<CRenderer> renderer, std::shared_ptr<CCamera> camera)
	{
		log_debug("render: %s", m_name.c_str());
		if (!m_mesh || !m_material)
			return;
		Matrix44f mvp = camera->get_view_projection() * m_transform;
		m_material->set_uniform("mvp_matrix", mvp);
		auto cmd = renderer->new_command<cmd_draw_mesh>();
		cmd->mesh = m_mesh.get();
		cmd->material = m_material.get();
		renderer->enqueue(cmd);
	}

} // namespace wyc