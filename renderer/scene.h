#pragma once
#include <string>
#include <memory>
#include <unordered_map>
#include "mesh.h"
#include "camera.h"
#include "renderer.h"
#include "material.h"
#include <sparrow/shader/flat_color.h>

namespace wyc
{
	class CSceneObj
	{
	public:
		CSceneObj(unsigned pid)
			: m_pid(pid)
			, m_mesh(nullptr)
		{
			m_transform.makeIdentity();
			load_default_material();
		}
		virtual ~CSceneObj()
		{
			m_mesh = nullptr;
		}
		inline unsigned get_pid() const {
			return m_pid;
		}
		virtual void render(std::shared_ptr<CRenderer> renderer)
		{
			auto cmd = renderer->new_command<cmd_draw_mesh>();
			cmd->mesh = m_mesh.get();
			cmd->material = m_material.get();
			renderer->enqueue(cmd);
		}

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
		}
		inline material_ptr get_material() const {
			return m_material;
		}

		void load_default_material()
		{
			CMaterialFlatColor *mateiral = new CMaterialFlatColor();
			mateiral->color = { 0.0f, 1.0f, 0.0f, 1.0f };
			mateiral->shader = shader_ptr(new CShaderFlatColor<VertexP3C3, VertexP3C3>());
			m_material = material_ptr(mateiral);
		}
		void set_camera_transform(const Matrix44f& world_to_camera)
		{
			m_material->mvp_matrix = m_transform * world_to_camera;
		}

	protected:
		unsigned m_pid;
		std::shared_ptr<CMesh> m_mesh;
		Matrix44f m_transform;
		material_ptr m_material;
	};

	class CScene
	{
	public:
		CScene();
		~CScene();
		CScene(const CScene* &scn) = delete;
		CScene& operator = (const CScene* &scn) = delete;

		bool load_collada(const std::wstring &file);
		
		std::shared_ptr<CMesh> create_mesh(const std::string &name);
		inline std::shared_ptr<CMesh> get_mesh(const std::string &name) const
		{
			auto &it = m_mesh_pool.find(name);
			if (it != m_mesh_pool.end())
				return it->second;
			return nullptr;
		}

		material_ptr create_material(const std::string &name, const std::string &type);
		inline material_ptr get_material(const std::string &name) const
		{
			auto &it = m_material_lib.find(name);
			if (it != m_material_lib.end())
				return it->second;
			return nullptr;
		}

		std::shared_ptr<CCamera> create_camera(const std::string &name);
		inline std::shared_ptr<CCamera> get_camera(const std::string &name) const
		{
			auto &it = m_camera_pool.find(name);
			if (it != m_camera_pool.end())
				return it->second;
			return nullptr;
		}
		inline void set_active_camera(const std::string &name) {
			m_active_camera = name;
		}
		inline std::shared_ptr<CCamera> get_active_camera() const {
			return get_camera(m_active_camera);
		}

		std::shared_ptr<CSceneObj> add_object(const std::string &mesh_name, const Matrix44f &transform);
		std::shared_ptr<CSceneObj> get_object(unsigned pid);

		void render(std::shared_ptr<CRenderer> renderer);

	private:
		std::unordered_map<std::string, std::shared_ptr<CMesh>> m_mesh_pool;
		std::unordered_map<std::string, std::shared_ptr<CCamera>> m_camera_pool;
		std::unordered_map<unsigned, std::shared_ptr<CSceneObj>> m_objs;
		std::unordered_map<std::string, material_ptr> m_material_lib;
		std::unordered_map<std::string, shader_ptr> m_shader_lib;
		unsigned m_cur_pid;
		std::string m_active_camera;
	};

} // namespace wyc
