#pragma once
#include "material.h"
#include "mesh.h"
#include "renderer.h"
#include "camera.h"

namespace wyc
{
	class CSceneObj
	{
	public:
		CSceneObj(unsigned pid);
		virtual ~CSceneObj();
		inline unsigned get_pid() const {
			return m_pid;
		}
		virtual void render(std::shared_ptr<CRenderer> renderer, std::shared_ptr<CCamera> camera);

		inline void set_mesh(std::shared_ptr<CMesh> mesh) {
			m_mesh = mesh;
		}
		inline const CMesh* get_mesh() const {
			return m_mesh.get();
		}
		inline void set_transform(const Matrix44f& transform) {
			m_transform = transform;
		}
		inline const Matrix44f& get_transform() const {
			return m_transform;
		}
		inline void set_material(material_ptr material) {
			m_material = material;
			m_is_material_changed = true;
		}
		inline material_ptr get_material() const {
			return m_material;
		}

		void load_default_material();

	protected:
		unsigned m_pid;
		std::shared_ptr<CMesh> m_mesh;
		Matrix44f m_transform;
		material_ptr m_material;
		bool m_is_material_changed;
	};

} // namespace wyc
