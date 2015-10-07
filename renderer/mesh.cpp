#include "mesh.h"

#include <cassert>

#include "OpenEXR/ImathVec.h"

namespace wyc
{
	CMesh::CMesh() : 
		m_layout(VF_NONE), 
		m_vertices(nullptr), 
		m_vert_count(0), 
		m_indices(nullptr), 
		m_index_count(0)
	{
	}

	CMesh::~CMesh()
	{
		clear();
	}

	void CMesh::clear()
	{
		m_layout = VF_NONE;
		if (m_vertices)
		{
			free(m_vertices);
			m_vertices = nullptr;
			m_vert_count = 0;
		}
		if (m_indices)
		{
			free(m_indices);
			m_indices = nullptr;
			m_index_count = 0;
		}
	}

	void CMesh::set_indices(std::initializer_list<uint32_t>&& indices)
	{
		if (m_indices)
		{
			free(m_indices);
			m_indices = nullptr;
			m_index_count = 0;
		}
		size_t cnt = indices.size();
		if (!cnt)
		{
			return;
		}
		uint32_t *data= static_cast<uint32_t*>(malloc(sizeof(uint32_t) * cnt));
		if (!data)
		{
			return;
		}
		m_indices = data;
		m_index_count = cnt;
		for (auto &v : indices)
		{
			*data++ = v;
		}
	}


	CTriangleMesh::CTriangleMesh()
	{
		set_vertices<VF_P3C3>({
			{
				{ 0, 0, 0 },
				{ 1.0, 0, 0 },
			},
			{
				{ 0, 0, 0 },
				{ 0, 1.0, 0 },
			},
			{
				{ 0, 0, 0 },
				{ 0, 0, 1.0 },
			},
		});

		set_indices({ 0, 1, 2 });

		const VertexP3C3 *verts = get_vertices<VF_P3C3>();
		size_t vert_cnt = get_vertex_count();
		size_t cnt = get_index_count();
		const uint32_t *indices = get_indices();
		for (size_t i = 2; i < cnt; i += 3)
		{
			uint32_t v1, v2, v3;
			v1 = indices[i - 2];
			v2 = indices[i - 1];
			v3 = indices[i];
			assert(v1 < vert_cnt);
			assert(v2 < vert_cnt);
			assert(v3 < vert_cnt);
			verts[v1];
			verts[v2];
			verts[v3];
		}
	}

} // namespace wyc