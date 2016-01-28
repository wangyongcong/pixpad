#include "mesh.h"

#include <cassert>
#include <fstream>
#include <locale>
#include <unordered_map>
#include <vector>
#include <sstream>

#include "OpenEXR/ImathVec.h"
#include "log.h"
#include "vertex_buffer.h"

namespace wyc
{
	CMesh::CMesh()
		: m_vb()
	{
	}

	CMesh::~CMesh()
	{
		
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
		std::vector<Imath::V2f> texcoords;
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

		auto read_vector2 = [&ss](std::vector<Imath::V2f> &pool) {
			Imath::V2f vec = { 0, 0 };
			int i = 0;
			while (ss && i < 2)
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
				read_vector2(texcoords);
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
		auto &vi = faces[0];
		m_vb.clear();
		m_vb.set_attribute(ATTR_POSITION, 3);
		bool has_uv = vi.y != null_index, has_normal = vi.z != null_index;
		if (has_uv) 
			m_vb.set_attribute(ATTR_TEXTURE, 2);
		if (has_normal)
			m_vb.set_attribute(ATTR_NORMAL, 3);
		// alloc buffer
		m_vb.resize(faces.size());
		// copy data
		auto pos = m_vb.get_attribute(ATTR_POSITION).begin();
		auto uv = m_vb.get_attribute(ATTR_TEXTURE).begin();
		auto normal = m_vb.get_attribute(ATTR_TEXTURE).begin();
		int i = 0;
		for (const Imath::V3i &index : faces)
		{
			if (index.x <= 0)
				i = index.x + vertices.size();
			else 
				i -= 1;
			*pos++ = vertices[i];
			if (has_uv) {
				if (index.y <= 0)
					i = index.y + texcoords.size();
				else
					i -= 1;
				*uv++ = texcoords[i];
			}
			if (has_normal) {
				if (index.z <= 0)
					i = index.z + normals.size();
				else
					i -= 1;
				*normal++ = normals[i];
			}
		}
		return true;
	}

	CTriangleMesh::CTriangleMesh(float radius)
	{
		const float sin30 = 0.5f, cos30 = 0.866f;
		set_vertices<VertexP3C3>({
			{
				{ 0, radius, 0 },
				{ 1.0f, 0, 0 },
			},
			{
				{ -radius * cos30, -radius * sin30, 0 },
				{ 0, 1.0f, 0 },
			},
			{
				{ radius * cos30, -radius * sin30, 0 },
				{ 0, 0, 1.0f },
			},
		});
	}

	CQuadMesh::CQuadMesh(float w, float h)
	{
		float x = w * 0.5f, y = h * 0.5f;
		set_vertices<VertexP3C3>({
			{
				{ -x, -y, 0 },
				{ 0, 1, 1 },
			},
			{
				{ x, -y, 0 },
				{ 1, 0, 1 },
			},
			{
				{ x, y, 0 },
				{ 1, 1, 0 },
			},
			{
				{ -x, -y, 0 },
				{ 0, 1, 1 },
			},
			{
				{ x, y, 0 },
				{ 1, 1, 0 },
			},
			{
				{ -x, y, 0 },
				{ 0, 0, 0 },
			},
		});
	}

} // namespace wyc