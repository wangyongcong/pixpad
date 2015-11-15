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
		VF_P3S2,
		VF_P3N3,
		VF_P3S2N3,
	};

	struct VertexP3C3
	{
		Imath::V3f pos;
		Imath::C3f color;
	};

	struct VertexP3S2
	{
		Imath::V3f pos;
		Imath::V2f uv;
	};

	struct VertexP3S2N3
	{
		Imath::V3f pos;
		Imath::V2f uv;
		Imath::V3f normal;
	};

	template<EVertexLayout Layout>
	struct CVertexLayout
	{
		using vertex_t = void;
	};

#define LAYOUT(f) \
	template<>\
	struct CVertexLayout<VF_##f>\
	{\
		using vertex_t = Vertex##f;\
	}
	
	LAYOUT(P3C3);
	LAYOUT(P3S2);
	LAYOUT(P3S2N3);

	class CMesh
	{
	public:
		CMesh();
		~CMesh();
		void clear();
		template<EVertexLayout Layout>
		bool resize(size_t count);
		template<EVertexLayout Layout>
		void set_vertices(std::initializer_list<typename CVertexLayout<Layout>::vertex_t>&& verts);
		template<EVertexLayout Layout>
		const typename CVertexLayout<Layout>::vertex_t* get_vertices() const;
		size_t get_vertex_count() const;
		void set_indices(std::initializer_list<uint32_t>&& indices);
		const uint32_t* get_indices() const;
		size_t get_index_count() const;
		
		// load from .obj file
		bool load_obj(const std::wstring &filepath);

	protected:
		bool reserve(size_t count, size_t vert_size);

		EVertexLayout m_layout;
		void *m_vertices;
		size_t m_vert_count;
		uint32_t *m_indices;
		size_t m_index_count;
	};

	template<EVertexLayout Layout>
	inline bool CMesh::resize(size_t count)
	{
		using vertex_t = CVertexLayout<Layout>::vertex_t;
		if (!reserve(count, sizeof(vertex_t)))
		{
			m_layout = VF_NONE;
			return false;
		}
		m_layout = Layout;
		return true;
	}

	template<EVertexLayout Layout>
	inline void CMesh::set_vertices(std::initializer_list<typename CVertexLayout<Layout>::vertex_t>&& verts)
	{
		if (!resize<Layout>(verts.size()))
		{
			return;
		}
		using vertex_t = CVertexLayout<Layout>::vertex_t;
		vertex_t *data = reinterpret_cast<vertex_t*>(m_vertices);
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
		CTriangleMesh(float radius);
	};

} // namespace wyc