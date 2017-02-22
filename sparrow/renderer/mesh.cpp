#include "mesh.h"

#include <cassert>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <OpenEXR/ImathVec.h>
#include "log.h"
#include "util.h"
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

	bool CMesh::load_obj(const std::wstring & w_file_path)
	{
		std::string path;
		if (!wstr2str(path, w_file_path))
		{
			log_error("Invalid file path");
			return false;
		}
		std::ifstream fin(path, std::ios_base::in);
		if (!fin.is_open())
		{
			log_error("Can't open file: %d, %s", 100, path.c_str());
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
					log_error("Buffer overflow, line [%d] maybe too long", line_no);
					break;
				}
			}
			ss.str(line);
			ss.clear();
			ss >> token;
			if (ss.fail())
			{
				++err;
				log_error("Bad token: [%d] %s", line_no, line);
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
					log_error("Expect mtl name: [%d] %s", line_no, line);
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
					log_error("Invalid mtl name: [%d] %s", line_no, line);
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
					log_error("Expect object name: [%d] %s", line_no, line);
					break;
				}
			}
			else if (token == "g")
			{// polygon group name
				ss >> name;
				if (ss.fail())
				{
					++err;
					log_error("Expect group name: [%d] %s", line_no, line);
					break;
				}
			}
			else if (token == "usemtl")
			{
				ss >> name;
				if (ss.fail())
				{
					++err;
					log_error("Expect mtl name: [%d] %s", line_no, line);
					break;
				}
				if (mtl_lib.find(name) == mtl_lib.end())
				{
					++err;
					log_error("Unknown mtl name: [%d] %s", line_no, line);
					break;
				}
			}
			else if (token == "f")
			{// face
				if (read_face() != 3)
				{
					++err;
					log_error("Face should have 3 vertices: [%d] %s", line_no, line);
					break;
				}
			}
			else
			{
				++err;
				log_error("Unknown token: [%d] %s", line_no, line);
				break;
			}
		}
		fin.close();
		delete[] line;
		if (err)
		{
			log_error("Found %d errors", err);
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
		auto normal = m_vb.get_attribute(ATTR_NORMAL).begin();
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

	void CMesh::create_triangle(float r)
	{
		const float sin30 = 0.5f, cos30 = 0.866f;
		set_vertices<VertexP3C3>({
			{
				{ 0, r, 0 },
				{ 1.0f, 0, 0 },
			},
			{
				{ -r * cos30, -r * sin30, 0 },
				{ 0, 1.0f, 0 },
			},
			{
				{ r * cos30, -r * sin30, 0 },
				{ 0, 0, 1.0f },
			},
		});
	}

	void CMesh::create_quad(float r)
	{
		set_vertices<VertexP3C3>({
			{
				{ -r, -r, 0 },
				{ 0, 1, 1 },
			},
			{
				{ r, -r, 0 },
				{ 1, 0, 1 },
			},
			{
				{ r, r, 0 },
				{ 1, 1, 0 },
			},
			{
				{ -r, -r, 0 },
				{ 0, 1, 1 },
			},
			{
				{ r, r, 0 },
				{ 1, 1, 0 },
			},
			{
				{ -r, r, 0 },
				{ 0, 0, 0 },
			},
		});
	}

	void CMesh::create_box(float r)
	{
		Imath::V3f verts[] = {
			// front face
			{ -r, -r,  r }, { r, -r,  r }, { r, r,  r }, { -r, r,  r },
			// back face
			{ -r, -r, -r }, { r, -r, -r }, { r, r, -r }, { -r, r, -r }
		};
		unsigned indices[] = {
			// front face
			0, 1, 2, 0, 2, 3,
			// right face
			1, 5, 6, 1, 6, 2,
			// back face
			5, 4, 7, 5, 7, 6,
			// left face
			4, 0, 3, 4, 3, 7,
			// top face
			3, 2, 6, 3, 6, 7,
			// bottom face
			4, 5, 1, 4, 1, 0,
		};
		m_vb.clear();
		m_vb.set_attribute(ATTR_POSITION, 3);
		m_vb.set_attribute(ATTR_COLOR, 3);
		m_vb.resize(8);
		auto pos = m_vb.get_attribute(ATTR_POSITION).begin();
		auto color = m_vb.get_attribute(ATTR_COLOR).begin();
		Imath::V3f c = { 1, 1, 1 };
		for (int i = 0; i < 8; ++i, ++pos, ++color)
		{
			*pos = verts[i];
			*color = c;
		}
		m_ib.resize(36, indices);
	}

	void CMesh::create_sphere(float r)
	{
	}

} // namespace wyc