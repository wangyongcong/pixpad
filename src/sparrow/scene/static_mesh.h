#pragma once
#include "scene_obj.h"

namespace wyc
{
	class CStaticMesh : public CSceneObj
	{
	public:
		CStaticMesh();
		virtual void render(CRenderer* renderer, const CCamera* camera);
		inline void set_mesh(std::shared_ptr<CMesh> mesh) {
			m_mesh = mesh;
		}
		inline const CMesh* get_mesh() const {
			return m_mesh.get();
		}
		inline void set_material(material_ptr material) {
			m_material = material;
			m_is_material_changed = true;
		}
		inline material_ptr get_material() const {
			return m_material;
		}

	protected:
		std::shared_ptr<CMesh> m_mesh;
		material_ptr m_material;
		bool m_is_material_changed;
	};

} // namespace wyc