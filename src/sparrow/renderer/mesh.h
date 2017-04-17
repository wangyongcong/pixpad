#pragma once

#include <iterator>
#include <vector>
#include <array>
#include <initializer_list>
#include <OpenEXR/ImathMatrix.h>
#include "mathfwd.h"
#include "vertex_buffer.h"
#include "vertex_layout.h"
#include "index_buffer.h"

namespace wyc
{
	class CMesh
	{
	public:
		CMesh();
		CMesh(const CMesh& rhs) = delete;
		CMesh& operator = (const CMesh& rhs) = delete;
		~CMesh();
		template<typename Vertex>
		void set_vertices(std::initializer_list<Vertex>&& verts);
		template<typename Vertex>
		void set_vertices(const VertexAttrib *attrib_array, unsigned attrib_count, const Vertex* vertices, unsigned vertex_count);
		size_t vertex_count() const;
		CVertexBuffer& vertex_buffer();
		const CVertexBuffer& vertex_buffer() const;
		CIndexBuffer& index_buffer();
		const CIndexBuffer& index_buffer() const;
		inline bool has_index() const {
			return m_ib.size() > 0;
		}
		// load from .obj file
		bool load_obj(const std::wstring &filepath);

		// create simple geometry mesh
		void create_triangle(float r);
		void create_quad(float r);
		void create_box(float r);
		void create_uv_box(float r);
		void create_sphere(float r);

	private:
		CVertexBuffer m_vb;
		CIndexBuffer m_ib;
		Matrix44f m_transform;
	};

	template<typename Vertex>
	inline void CMesh::set_vertices(std::initializer_list<Vertex>&& verts)
	{
		using layout_t = Vertex::Layout;
		using vertex_t = Vertex;

		m_vb.clear();

		if (!layout_t::attr_count || !verts.size())
		{
			return;
		}
		for (auto &attr : layout_t::attr_table)
		{
			m_vb.set_attribute(attr.usage, attr.component);
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

	template<typename Vertex>
	inline void CMesh::set_vertices(const VertexAttrib * attrib_array, unsigned attrib_count, const Vertex * vertices, unsigned vertex_count)
	{
		m_vb.clear();
		for (auto i = 0u; i < attrib_count; ++i) {
			auto &attrib = attrib_array[i];
			m_vb.set_attribute(attrib.usage, attrib.component);
		}
		m_vb.resize(vertex_count);
		auto out = m_vb.begin();
		for (auto i = 0u; i < vertex_count; ++i, ++out) {
			*out = vertices[i];
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

	inline CIndexBuffer & CMesh::index_buffer()
	{
		return m_ib;
	}

	inline const CIndexBuffer & CMesh::index_buffer() const
	{
		return m_ib;
	}

} // namespace wyc