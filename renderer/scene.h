#pragma once
#include <string>
#include <memory>
#include <unordered_map>
#include "mesh.h"
#include "camera.h"
#include "renderer.h"

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

	protected:
		unsigned m_pid;
		std::shared_ptr<CMesh> m_mesh;
		Matrix44f m_transform;
	};

	class CScene
	{
	public:
		CScene();
		~CScene();
		CScene(const CScene* &scn) = delete;
		CScene& operator = (const CScene* &scn) = delete;

		bool load_collada(const std::wstring &file);
		
		inline std::shared_ptr<CMesh> get_mesh(const std::string &name) const
		{
			auto &it = m_mesh_pool.find(name);
			if (it != m_mesh_pool.end())
				return it->second;
			return nullptr;
		}
		std::shared_ptr<CMesh> create_mesh(const std::string &name);

		inline std::shared_ptr<CCamera> get_camera(const std::string &name) const
		{
			auto &it = m_camera_pool.find(name);
			if (it != m_camera_pool.end())
				return it->second;
			return nullptr;
		}
		std::shared_ptr<CCamera> create_camera(const std::string &name);

		std::shared_ptr<CSceneObj> add_object(const std::string &mesh_name, const Matrix44f &transform);
		std::shared_ptr<CSceneObj> get_object(unsigned pid);
		void render(std::shared_ptr<CRenderer> renderer);

	private:
		std::unordered_map<std::string, std::shared_ptr<CMesh>> m_mesh_pool;
		std::unordered_map<std::string, std::shared_ptr<CCamera>> m_camera_pool;
		std::unordered_map<unsigned, std::shared_ptr<CSceneObj>> m_objs;
		unsigned m_cur_pid;
	};

} // namespace wyc
