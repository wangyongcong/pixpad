#pragma once

#include <vector>
#include <initializer_list>

#include "OpenEXR/ImathVec.h"
#include "OpenEXR/ImathColor.h"

namespace wyc
{
	enum EVertexLayout
	{
		VF_NONE = 0,
		VF_P3C3,
		VF_P3C3N3, 
	};

	struct VertexP3C3
	{
		Imath::V3f pos;
		Imath::C3f color;
	};

	template<EVertexLayout Layout>
	struct CVertexLayout
	{
		using vertex_t = void;
	};

	template<>
	struct CVertexLayout<VF_P3C3>
	{
		using vertex_t = VertexP3C3;
	};

	class CMesh
	{
	public:
		CMesh();
		~CMesh();
		void clear();
		template<EVertexLayout Layout>
		void set_vertices(std::initializer_list<typename CVertexLayout<Layout>::vertex_t>&& verts);
		template<EVertexLayout Layout>
		const typename CVertexLayout<Layout>::vertex_t* get_vertices() const;
		size_t get_vertex_count() const;
		void set_indices(std::initializer_list<uint32_t>&& indices);
		const uint32_t* get_indices() const;
		size_t get_index_count() const;
	protected:
		EVertexLayout m_layout;
		void *m_vertices;
		size_t m_vert_count;
		uint32_t *m_indices;
		size_t m_index_count;
	};

	template<EVertexLayout Layout>
	inline void CMesh::set_vertices(std::initializer_list<typename CVertexLayout<Layout>::vertex_t>&& verts)
	{
		if (m_vertices)
		{
			m_layout = VF_NONE;
			free(m_vertices);
			m_vertices = nullptr;
			m_vert_count = 0;
		}
		using vertex_t = CVertexLayout<Layout>::vertex_t;
		size_t cnt = verts.size();
		if (!cnt)
		{
			return;
		}
		vertex_t *data = (vertex_t*)malloc(cnt * sizeof(vertex_t));
		if (!data)
		{
			return;
		}
		m_vertices = data;
		m_vert_count = cnt;
		m_layout = Layout;
		for (auto &v : verts)
		{
			*data++ = v;
		}
	}

	template<EVertexLayout Layout>
	inline const typename CVertexLayout<Layout>::vertex_t * CMesh::get_vertices() const
	{
		if (Layout != m_layout)
			return nullptr;
		using vertex_t = CVertexLayout<Layout>::vertex_t;
		return static_cast<const vertex_t*>(m_vertices);
	}

	inline size_t CMesh::get_vertex_count() const
	{
		return m_vert_count;
	}

	inline const uint32_t* CMesh::get_indices() const
	{
		return m_indices;
	}

	inline size_t CMesh::get_index_count() const
	{
		return m_index_count;
	}

	class CTriangleMesh : public CMesh
	{
	public:
		CTriangleMesh();
	};

} // namespace wyc