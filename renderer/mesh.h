#pragma once

#include <iterator>
#include <vector>
#include <array>
#include <initializer_list>
#include "OpenEXR/ImathVec.h"
#include "OpenEXR/ImathColor.h"
#include "vertex_buffer.h"

namespace wyc
{
	enum EVertexLayout
	{
		VF_NONE = 0,
		VF_P3C3,
		VF_P3S2,
		VF_P3N3,
		VF_P3C3N3,
		VF_P3S2N3,
	};

	template<EVertexLayout Layout>
	struct CVertexLayout
	{
		using vertex_t = void;
		static const int attr_count = 0;
		static const VertexAttribute *attr_table;
	};

	struct VertexP3C3
	{
		Imath::V3f pos;
		Imath::C3f color;
	};

	template<>
	struct CVertexLayout<VF_P3C3>
	{
		using vertex_t = VertexP3C3;
		static const int attr_count = 2;
		static const VertexAttribute attr_table[attr_count];
	};

	struct VertexP3S2
	{
		Imath::V3f pos;
		Imath::V2f uv;
	};

	struct VertexP3C3N3
	{
		Imath::V3f pos;
		Imath::C3f color;
		Imath::V3f normal;
	};

	struct VertexP3S2N3
	{
		Imath::V3f pos;
		Imath::V2f uv;
		Imath::V3f normal;
	};

#define LAYOUT(f) \
	template<>\
	struct CVertexLayout<VF_##f>\
	{\
		using vertex_t = Vertex##f;\
	}
	
	LAYOUT(P3S2);
	LAYOUT(P3C3N3);
	LAYOUT(P3S2N3);

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