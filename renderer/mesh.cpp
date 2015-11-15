#include "mesh.h"

#include <cassert>
#include <fstream>
#include <locale>
#include <unordered_map>
#include <vector>
#include <sstream>

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
		std::ifstream fin(path, std::ios_base::in);
		if (!fin.is_open())
		{
			error("Can't open file: %d, %s", 100, path.c_str());
			return false;
		}
		std::string token;
		size_t sz = 512;
		char *line = new char[sz];
		size_t line_no = 0;
		std::stringstream ss;
		std::unordered_map<std::string, std::string> mtl_lib;
		std::vector<Imath::V3f> vertices;
		std::vector<Imath::V3f> texcoords;
		std::vector<Imath::V3f> normals;
		std::vector<Imath::V3f> parameter;
		std::vector<Imath::V3i> faces;
		std::string name;
		constexpr int null_index = std::numeric_limits<int>::max();
		int err = 0;

		auto read_vector3 = [&ss] (std::vector<Imath::V3f> &pool) {
			Imath::V3f vec = { 0, 0, 0 };
			int i = 0;
			while (ss && i < 3)
			{
				ss >> vec[i++];
			}
			pool.push_back(vec);
		};

		auto read_face = [&ss, &token, &faces, null_index] () {
			std::string str;
			std::istringstream tmp;
			int vert_cnt = 0;
			while (ss >> token)
			{
				++vert_cnt;
				tmp.str(token);
				tmp.clear();
				Imath::V3i vi = { null_index, null_index, null_index };
				for (int i = 0; std::getline(tmp, str, '/') && i < 3; ++i)
				{
					if (!str.empty())
					{
						vi[i] = std::stoi(str);
					}
				}
				faces.push_back(vi);
			}
			return vert_cnt;
		};

		while (!(fin.eof() || fin.bad()))
		{
			fin.getline(line, sz);
			++line_no;
			if (line[0] == 0 || line[0] == '#')
			{// skip empty line or comment
				continue;
			}
			if (fin.fail())
			{// not enough buffer
				constexpr size_t max_buff_size = 4096;
				size_t new_size;
				char *new_buff;
				do {
					fin.clear();
					new_size = sz << 1;
					new_buff = new char[new_size];
					memcpy(new_buff, line, sz);
					delete[] line;
					line = new_buff;
					fin.getline(line + sz, new_size - sz);
					sz = new_size;
				} while (fin.fail() && sz < max_buff_size);
				if (fin.fail())
				{
					++err;
					error("Buffer overflow, line [%d] maybe too long", line_no);
					break;
				}
			}
			ss.str(line);
			ss.clear();
			ss >> token;
			if (ss.fail())
			{
				++err;
				error("Bad token: [%d] %s", line_no, line);
				break;
			}
			if (token[0] == '#')
			{ // commnent, skip the rest
				continue;
			}
			else if (token == "mtllib")
			{
				ss >> name;
				if (ss.fail())
				{
					++err;
					error("Expect mtl name: [%d] %s", line_no, line);
					break;
				}
				size_t pos = name.rfind('.');
				if (pos != std::string::npos)
				{
					mtl_lib[name.substr(0, pos)] = name;
				}
				else
				{
					++err;
					error("Invalid mtl name: [%d] %s", line_no, line);
					break;
				}
			}
			else if (token == "v")
			{// vertex
				read_vector3(vertices);
			}
			else if (token == "vt")
			{// texture coordinate
				read_vector3(texcoords);
			}
			else if (token == "vn")
			{// vertex normal
				read_vector3(normals);
			}
			else if (token == "vp")
			{// vertex in parameter space
				read_vector3(parameter);
			}
			else if (token == "o")
			{// object name
				ss >> name;
				if (ss.fail())
				{
					++err;
					error("Expect object name: [%d] %s", line_no, line);
					break;
				}
			}
			else if (token == "g")
			{// polygon group name
				ss >> name;
				if (ss.fail())
				{
					++err;
					error("Expect group name: [%d] %s", line_no, line);
					break;
				}
			}
			else if (token == "usemtl")
			{
				ss >> name;
				if (ss.fail())
				{
					++err;
					error("Expect mtl name: [%d] %s", line_no, line);
					break;
				}
				if (mtl_lib.find(name) == mtl_lib.end())
				{
					++err;
					error("Unknown mtl name: [%d] %s", line_no, line);
					break;
				}
			}
			else if (token == "f")
			{// face
				if (read_face() != 3)
				{
					++err;
					error("Face should have 3 vertices: [%d] %s", line_no, line);
					break;
				}
			}
			else
			{
				++err;
				error("Unknown token: [%d] %s", line_no, line);
				break;
			}
		}
		fin.close();
		delete[] line;
		if (err)
		{
			error("Found %d errors", err);
			return false;
		}
		// interpret data
		if (faces.empty())
		{
			return true;
		}
		auto vi = faces[0];
		if (vi.y == null_index)
		{
			return false;
		}
		else if (vi.z == null_index)
		{
			if (!resize<VF_P3S2>(faces.size()))
			{
				return false;
			}
			using vertex_t = CVertexLayout<VF_P3S2>::vertex_t;
			vertex_t *out = reinterpret_cast<vertex_t*>(m_vertices);
			for (auto &vi : faces)
			{
				out->pos = vertices[vi.x];
				Imath::V3f &tmp = texcoords[vi.y];
				out->uv.x = tmp.x;
				out->uv.y = tmp.y;
				++out;
			}
		}
		else
		{
			if (!resize<VF_P3S2N3>(faces.size()))
			{
				return false;
			}
			using vertex_t = CVertexLayout<VF_P3S2N3>::vertex_t;
			vertex_t *out = reinterpret_cast<vertex_t*>(m_vertices);
			for (auto &vi : faces)
			{
				out->pos = vertices[vi.x];
				Imath::V3f &tmp = texcoords[vi.y];
				out->uv.x = tmp.x;
				out->uv.y = tmp.y;
				out->normal = normals[vi.z];
				++out;
			}

		}

		return true;
	}

	bool CMesh::reserve(size_t count, size_t vert_size)
	{
		if (m_vertices)
		{
			free(m_vertices);
			m_vertices = nullptr;
			m_vert_count = 0;
		}
		if (!count)
		{
			return false;
		}
		m_vertices = malloc(count * vert_size);
		if (!m_vertices)
		{
			return false;
		}
		m_vert_count = count;
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