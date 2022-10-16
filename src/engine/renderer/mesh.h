#pragma once
#include "engine.h"
#include <iterator>
#include <vector>
#include <array>
#include <initializer_list>
#include <ImathMatrix.h>
#include "common/common_macros.h"
#include "mathex/vecmath.h"
#include "vertex_buffer.h"
#include "vertex_layout.h"
#include "index_buffer.h"

namespace wyc
{

enum EPrimitiveType
{
	PRIM_TYPE_TRIANGLE,

	PRIM_TYPE_COUNT
};

class WYCAPI CMesh
{
	DISALLOW_COPY_MOVE_AND_ASSIGN(CMesh)
public:
	CMesh();
	~CMesh();
	template<typename Vertex>
	void set_vertices(const VertexAttribute *attrib_array, unsigned attrib_count, const Vertex* vertices, unsigned vertex_count);
	size_t vertex_count() const;
	VertexBuffer& vertex_buffer();
	const VertexBuffer& vertex_buffer() const;
	void set_indices(std::initializer_list<unsigned>&& indices);
	IndexBuffer& index_buffer();
	const IndexBuffer& index_buffer() const;
	bool has_index() const;
	EPrimitiveType primitive_type() const;

	/**
	 * \brief load mesh data from file
	 * \param file_path mesh file path
	 * \return if success
	 */
	bool load(const std::string &file_path);

	// create simple geometry mesh
	void create_triangle(float r);
	void create_quad(float r);
	void create_box(float r);
	void create_uv_box(float r);
	void create_sphere(float r, uint8_t smoothness=2);

protected:
	// load from .obj file
	bool load_obj(const std::string &path);
	// load from .ply file
	bool load_ply(const std::string &path);

private:
	VertexBuffer m_vb;
	IndexBuffer m_ib;
	mat4f m_transform;
	EPrimitiveType m_primitive_type;
};

template<typename Vertex>
inline void CMesh::set_vertices(const VertexAttribute * attrib_array, unsigned attrib_count, const Vertex * vertices, unsigned vertex_count)
{
	m_vb.clear();
	for (auto i = 0u; i < attrib_count; ++i) {
		auto &attrib = attrib_array[i];
		m_vb.set_attribute(attrib.usage, attrib.format);
	}
	m_vb.resize(vertex_count);
	if (vertices) {
		auto out = m_vb.begin();
		for (auto i = 0u; i < vertex_count; ++i, ++out) {
			*out = vertices[i];
		}
	}
}

inline size_t CMesh::vertex_count() const
{
	return m_vb.size();
}

inline VertexBuffer & CMesh::vertex_buffer()
{
	return m_vb;
}

inline const VertexBuffer & CMesh::vertex_buffer() const
{
	return m_vb;
}

inline void CMesh::set_indices(std::initializer_list<unsigned>&& indices)
{
	m_ib.resize(indices.size(),	std::numeric_limits<unsigned>::max());
	uint32_t *buf = m_ib.data<uint32_t>();
	for (unsigned v : indices)
	{
		*buf++ = v;
	}
	assert(m_ib.is_valid_end(buf));
}

inline IndexBuffer & CMesh::index_buffer()
{
	return m_ib;
}

inline const IndexBuffer & CMesh::index_buffer() const
{
	return m_ib;
}

inline bool CMesh::has_index() const
{
	return m_ib.size() > 0;
}

inline EPrimitiveType CMesh::primitive_type() const
{
	return m_primitive_type;
}

} // namespace wyc
