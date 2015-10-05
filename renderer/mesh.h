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
		typename CVertexLayout<Layout>::vertex_t* get_vertices();
		void set_indices(std::initializer_list<uint32_t>&& indices);

	protected:
		EVertexLayout m_layout;
		void *m_vertices;
		size_t m_vert_count;
	};

	template<EVertexLayout Layout>
	inline void CMesh::set_vertices(std::initializer_list<typename CVertexLayout<Layout>::vertex_t>&& verts)
	{
		if (m_vertices)
		{
			clear();
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

	class CTriangleMesh : public CMesh
	{
	public:
		CTriangleMesh();
	};

} // namespace wyc