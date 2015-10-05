#include "mesh.h"

#include "OpenEXR/ImathVec.h"

namespace wyc
{
	CMesh::CMesh() : m_layout(VF_NONE), m_vertices(nullptr), m_vert_count(0)
	{
	}

	CMesh::~CMesh()
	{
		clear();
	}

	void CMesh::clear()
	{
		if (m_vertices)
		{
			free(m_vertices);
			m_vertices = 0;
			m_vert_count = 0;
		}
		m_layout = VF_NONE;
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
	}

} // namespace wyc