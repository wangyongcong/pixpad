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


	CTriangleMesh::CTriangleMesh(float radius)
	{
		const float sin30 = 0.5f, cos30 = 0.866f;
		set_vertices<VF_P3C3>({
			{
				{ 0, radius, 0 },
				{ 1.0, 0, 0 },
			},
			{
				{ -radius * cos30, -radius * sin30, 0 },
				{ 0, 1.0, 0 },
			},
			{
				{ radius * cos30, -radius * sin30, 0 },
				{ 0, 0, 1.0 },
			},
		});
	}

} // namespace wyc