#include "mesh.h"

#include <cassert>
#include <fstream>
#include <locale>
#include <unordered_map>
#include <vector>

#include "OpenEXR/ImathVec.h"
#include "log.h"

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

	bool CMesh::load_obj(const std::wstring & filepath)
	{
		std::wstring_convert<std::codecvt<wchar_t, char, std::mbstate_t>> cvt;
		std::string path =cvt.to_bytes(filepath);
		std::ifstream in(path, std::ios_base::in);
		if (!in.is_open())
		{
			error("Can't open file: %s", path);
			return false;
		}
		std::vector<std::string> mtl_list;
		std::string token;
		size_t sz = 512;
		char *line = new char[sz];
		while (!(in.eof() || in.bad()))
		{
			in.getline(line, sz);
			if (in.fail())
			{
				constexpr size_t max_buff_size = 4096;
				size_t new_size;
				char *new_buff;
				do {
					in.clear();
					new_size = sz << 1;
					new_buff = new char[new_size];
					memcpy(new_buff, line, sz);
					delete[] line;
					line = new_buff;
					in.getline(line + sz, new_size - sz);
					sz = new_size;
				} while (in.fail() && sz < max_buff_size);
				if (in.fail())
				{
					error("Buffer overflow");
					break;
				}
			}
			std::stringstream ss;
			in >> token;
			if (in.fail())
			{
				error("Bad token");
				break;
			}
			if (token[0] == '#')
			{ // commnent, skip the rest line
				in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
				continue;
			}
			if (token == "mtllib")
			{
				in >> token;
				if (in.fail())
				{
					error("Bad token: expect mtllib name");
					break;
				}
				mtl_list.push_back(token);
			}
			else if (token == "v")
			{
			}
			else if (token == "vt")
			{

			}
			else if (token == "vn")
			{

			}
			else if (token == "o")
			{// object name

			}
			else if (token == "g")
			{// polygon group name

			}
		}
		in.close();
		delete[] line;
		return true;
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