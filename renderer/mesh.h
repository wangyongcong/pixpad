#pragma once

#include <iterator>
#include <vector>
#include <array>
#include <initializer_list>
#include "vertex_buffer.h"

namespace wyc
{
	class CMesh
	{
	public:
		CMesh();
		CMesh(const CMesh& rhs) = delete;
		CMesh& operator = (const CMesh& rhs) = delete;
		~CMesh();
		template<EVertexLayout Layout>
		void set_vertices(std::initializer_list<typename CVertexLayout<Layout>::vertex_t>&& verts);
		size_t vertex_count() const;
		CVertexBuffer& vertex_buffer();
		const CVertexBuffer& vertex_buffer() const;

		// load from .obj file
		bool load_obj(const std::wstring &filepath);

	protected:
		CVertexBuffer m_vb;
	};

	template<EVertexLayout Layout>
	inline void CMesh::set_vertices(std::initializer_list<typename CVertexLayout<Layout>::vertex_t>&& verts)
	{
		using layout_t = CVertexLayout<Layout>;
		using vertex_t = layout_t::vertex_t;

		m_vb.clear();

		if (!layout_t::attr_count || !verts.size())
		{
			return;
		}
		for (auto &attr : layout_t::attr_table)
		{
			m_vb.set_attribute(attr.usage, attr.elem_cnt);
		}
		m_vb.resize(verts.size());
		assert(m_vb.vertex_size() == sizeof(vertex_t));
		auto out = m_vb.begin();
		for (auto &v : verts)
		{
			*out = v;
			++out;
		}
	}

	inline size_t CMesh::vertex_count() const
	{
		return m_vb.size();
	}

	inline CVertexBuffer & CMesh::vertex_buffer()
	{
		return m_vb;
	}

	inline const CVertexBuffer & CMesh::vertex_buffer() const
	{
		return m_vb;
	}

	class CTriangleMesh : public CMesh
	{
	public:
		CTriangleMesh(float radius);
	};

} // namespace wyc