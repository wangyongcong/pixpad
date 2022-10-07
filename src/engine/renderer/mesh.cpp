#include "renderer/mesh.h"
#include <cassert>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <ImathColor.h>
#include "stb/stb_log.h"
#include "common/utility.h"
#include "common/log_macros.h"
#include "ply.h"

namespace wyc
{
	CMesh::CMesh()
		: m_vb()
		, m_ib()
		, m_primitive_type(PRIM_TYPE_TRIANGLE)
	{
	}

	CMesh::~CMesh()
	{

	}

	bool CMesh::load(const std::string& file_path)
	{
		std::string ext = get_file_ext(file_path);
		if(ext == "obj")
		{
			return load_obj(file_path);
		}
		else if(ext == "ply")
		{
			return load_ply(file_path);
		}
		LogError("Unkonwn mesh file type: %s", file_path);
		return false;
	}

	bool CMesh::load_obj(const std::string & path)
	{
		std::ifstream fin(path, std::ios_base::in);
		if (!fin.is_open())
		{
			log_error("Can't open file: %s", path);
			return false;
		}
		std::string token;
		size_t sz = 512;
		char *line = new char[sz];
		size_t line_no = 0;
		std::stringstream ss;
		std::unordered_map<std::string, std::string> mtl_lib;
		std::vector<vec3f> vertices;
		std::vector<vec2f> texcoords;
		std::vector<vec3f> normals;
		std::vector<vec3f> parameter;
		std::vector<vec3i> faces;
		std::string name;
		constexpr int null_index = std::numeric_limits<int>::max();
		int err = 0;

		auto read_vector3 = [&ss](std::vector<vec3f> &pool) {
			vec3f vec = { 0, 0, 0 };
			int i = 0;
			while (ss && i < 3)
			{
				ss >> vec[i++];
			}
			pool.push_back(vec);
		};

		auto read_vector2 = [&ss](std::vector<vec2f> &pool) {
			vec2f vec = { 0, 0 };
			int i = 0;
			while (ss && i < 2)
			{
				ss >> vec[i++];
			}
			pool.push_back(vec);
		};

		auto read_face = [=, &ss, &token, &faces]() {
			std::string str;
			std::istringstream tmp;
			int vert_cnt = 0;
			while (ss >> token)
			{
				++vert_cnt;
				tmp.str(token);
				tmp.clear();
				vec3i vi = { null_index, null_index, null_index };
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
		m_vb.set_attribute(ATTR_POSITION, TinyImageFormat_R32G32B32_SFLOAT);
		bool has_uv = vi.y != null_index, has_normal = vi.z != null_index;
		if (has_uv)
			m_vb.set_attribute(ATTR_UV0, TinyImageFormat_R32G32_SFLOAT);
		if (has_normal)
			m_vb.set_attribute(ATTR_NORMAL, TinyImageFormat_R32G32B32_SFLOAT);
		// alloc buffer
		m_vb.resize((unsigned)faces.size());
		// copy data
		auto pos = m_vb.get_attribute(ATTR_POSITION).begin();
		auto uv = m_vb.get_attribute(ATTR_UV0).begin();
		auto normal = m_vb.get_attribute(ATTR_NORMAL).begin();
		int i = 0;
		for (const vec3i &index : faces)
		{
			if (index.x <= 0)
				i = index.x + (int)vertices.size();
			else
				i -= 1;
			*pos++ = vertices[i];
			if (has_uv) {
				if (index.y <= 0)
					i = index.y + (int)texcoords.size();
				else
					i -= 1;
				*uv++ = texcoords[i];
			}
			if (has_normal) {
				if (index.z <= 0)
					i = index.z + (int)normals.size();
				else
					i -= 1;
				*normal++ = normals[i];
			}
		}
		return true;
	}

	bool CMesh::load_ply(const std::string & path)
	{
		CPlyFile ply(path);
		if (!ply) {
			log_error("[PLY] fail to load [%s]: %s", path, ply.get_error_desc());
			return false;
		}
		m_vb.clear();
		auto prop = ply.get_vertex_property();
		unsigned channel = 0;
		unsigned prop_size = 0;
		unsigned undefined_attr = 0;
		bool is_undefined = false;
		bool is_float = false;
		const PlyProperty* last;
		while(prop)
		{
			if(prop->is_vector("x", "y", "z", &last) && prop->is_float())
			{
				if(is_undefined)
				{
					m_vb.set_attribute(ATTR_UNDEFINED, channel, prop_size, is_float);
					is_undefined = false;
				}
				m_vb.set_attribute(ATTR_POSITION, TinyImageFormat_R32G32B32_SFLOAT);
				prop = last;
			}
			else if(prop->is_vector("nx", "ny", "nz", &last) && prop->is_float())
			{
				if(is_undefined)
				{
					m_vb.set_attribute(ATTR_UNDEFINED, channel, prop_size, is_float);
					is_undefined = false;
				}
				m_vb.set_attribute(ATTR_NORMAL, TinyImageFormat_R32G32B32_SFLOAT);
				prop = last;
			}
			else if(prop->is_vector("red", "green", "blue", &last))
			{
				if(is_undefined)
				{
					m_vb.set_attribute(ATTR_UNDEFINED, channel, prop_size, is_float);
					is_undefined = false;
				}
				prop = last;
				TinyImageFormat format;
				if(prop->name == "alpha")
				{
					format = prop->is_float() ? TinyImageFormat_R32G32B32A32_SFLOAT : TinyImageFormat_R8G8B8A8_UINT;
					prop = prop->next;
				}
				else
				{
					format = prop->is_float() ? TinyImageFormat_R32G32B32_SFLOAT : TinyImageFormat_R8G8B8_UINT;
				}
				m_vb.set_attribute(ATTR_COLOR, format);
			}
			else if(is_undefined)
			{
				if(prop->is_float() != is_float || prop_size + prop->size >= 256)
				{
					assert(prop_size < 256);
					m_vb.set_attribute(ATTR_UNDEFINED, channel, prop_size, is_float);
					channel = 1;
					prop_size = prop->size;
					is_float = prop->is_float();
				}
				else
				{
					channel += 1;
					prop_size += prop->size;
				}
				prop = prop->next;
			}
			else
			{
				is_undefined = true;
				undefined_attr += 1;
				channel = 1;
				prop_size = prop->size;
				is_float = prop->is_float();
				prop = prop->next;
			}
		}
		ply.detail();
		if(is_undefined)
		{
			m_vb.set_attribute(ATTR_UNDEFINED, channel, prop_size, is_float);
		}
		if(!ply.load())
		{
			log_error("[PLY] fail to load data [%s]: %s", path, ply.get_error_desc());
			ply.detail();
			return false;
		}
		unsigned vertex_count = ply.vertex_count();
		m_vb.resize(vertex_count);
		if(vertex_count != ply.read_vertex(m_vb.data(), m_vb.data_size()))
		{
			log_error("[PLY] fail to read vertex data [%s]", path);
			ply.detail();
			return false;
		}
		unsigned index_count = ply.face_count() * 3;
		m_ib.resize(index_count, vertex_count);
		if(index_count != ply.read_face(m_ib.data(), m_ib.data_size(), m_ib.stride()))
		{
			log_error("[PLY] fail to read vertex index data [%s]", path);
			ply.detail();
			return false;
		}
		if(undefined_attr > 0)
		{
			log_warning("[PLY] has undefined vertex attributes");
			ply.detail();
		}
		log_debug("[PLY] file loaded: vertex %d, indices %d", m_vb.size(), m_ib.size());
		return true;
	}

	void CMesh::create_triangle(float r)
	{
		struct Vertex {
			vec3f pos;
			color3f color;
		};
		VertexAttribute attrib_array[] = {
			{ATTR_POSITION, TinyImageFormat_R32G32B32_SFLOAT },
			{ATTR_COLOR, TinyImageFormat_R32G32B32_SFLOAT },
		};
		const float sin30 = 0.5f, cos30 = 0.866f;
		Vertex vertices[] = {
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
		};
		set_vertices(attrib_array, 2, vertices, 3);
	}

	void CMesh::create_quad(float r)
	{
		struct Vertex {
			vec3f pos;
			color3f color;
		};
		VertexAttribute attrib_array[] = {
			{ ATTR_POSITION, TinyImageFormat_R32G32B32_SFLOAT },
			{ ATTR_COLOR, TinyImageFormat_R32G32B32_SFLOAT },
		};
		Vertex vertices[] = {
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
		};
		set_vertices(attrib_array, 2, vertices, 3);
	}

	void CMesh::create_box(float r)
	{
		struct Vertex {
			vec3f pos;
			color3f color;
		};
		VertexAttribute attrib_array[] = {
			{ ATTR_POSITION, TinyImageFormat_R32G32B32_SFLOAT },
			{ ATTR_COLOR, TinyImageFormat_R32G32B32_SFLOAT },
		};

		Vertex vertices[] = {
			// front face
			{ { -r, -r,  r },{ 1, 1, 1 } },
			{ { r, -r,  r },{ 1, 1, 1 } },
			{ { r, r,  r },{ 1, 1, 1 } },
			{ { -r, r,  r },{ 1, 1, 1 } },
			// back face
			{ { -r, -r, -r },{ 1, 1, 1 } },
			{ { r, -r, -r },{ 1, 1, 1 } },
			{ { r, r, -r },{ 1, 1, 1 } },
			{ { -r, r, -r },{ 1, 1, 1 } }
		};
		set_vertices(attrib_array, 2, vertices, 8);
		set_indices({
			// front face
			0, 1, 2, 0, 2, 3,
			// right face
			1, 5, 2, 5, 6, 2,
			// back face
			5, 4, 7, 5, 7, 6,
			// left face
			4, 0, 7, 0, 3, 7,
			// top face
			3, 2, 7, 2, 6, 7,
			// bottom face
			4, 5, 0, 5, 1, 0,
		});
	}

	void CMesh::create_uv_box(float r)
	{
		struct Vertex {
			vec3f pos;
			color4f color;
			vec2f uv;
		};
		VertexAttribute attrib_array[] = {
			{ATTR_POSITION, TinyImageFormat_R32G32B32_SFLOAT },
			{ATTR_COLOR, TinyImageFormat_R32G32B32A32_SFLOAT },
			{ATTR_UV0, TinyImageFormat_R32G32_SFLOAT },
		};
		Vertex vertices[16] = {
			{ { -r, -r, -r },{ 1, 1, 1, 1 },{ 0, 0 } },
			{ { -r,  r, -r },{ 1, 1, 1, 1 },{ 0, 1 } },
			{ { -r, -r,  r },{ 1, 1, 1, 1 },{ 1, 0 } },
			{ { -r,  r,  r },{ 1, 1, 1, 1 },{ 1, 1 } },
			{ {  r, -r,  r },{ 1, 1, 1, 1 },{ 0, 0 } },
			{ {  r,  r,  r },{ 1, 1, 1, 1 },{ 0, 1 } },
			{ {  r, -r, -r },{ 1, 1, 1, 1 },{ 1, 0 } },
			{ {  r,  r, -r },{ 1, 1, 1, 1 },{ 1, 1 } },

			{ { -r,  r,  r },{ 1, 1, 1, 1 },{ 0, 0 } },
			{ {  r,  r,  r },{ 1, 1, 1, 1 },{ 0, 1 } },
			{ { -r,  r, -r },{ 1, 1, 1, 1 },{ 1, 0 } },
			{ {  r,  r, -r },{ 1, 1, 1, 1 },{ 1, 1 } },
			{ { -r, -r, -r },{ 1, 1, 1, 1 },{ 0, 0 } },
			{ {  r, -r, -r },{ 1, 1, 1, 1 },{ 0, 1 } },
			{ { -r, -r,  r },{ 1, 1, 1, 1 },{ 1, 0 } },
			{ {  r, -r,  r },{ 1, 1, 1, 1 },{ 1, 1 } },
		};
		set_vertices(attrib_array, 3, vertices, 16);
		set_indices({
			0, 2, 3, 0, 3, 1,
			2, 4, 5, 2, 5, 3,
			4, 6, 7, 4, 7, 5,
			8, 9, 11, 8, 11, 10,
			13, 12, 10, 13, 10, 11,
			15, 14, 12, 15, 12, 13,
		});
	}

	// icosphere generation based on the implementation of https://github.com/caosdoar/spheres
	void CMesh::create_sphere(float r, uint8_t smoothness)
	{
		// generate icosahedron
		const float t = float((1.0 + std::sqrt(5.0)) / 2.0);
		std::vector<vec3f> vertices = {
			{ -1.0f,  t, 0.0f },
			{  1.0f,  t, 0.0f },
			{ -1.0f, -t, 0.0f },
			{  1.0f, -t, 0.0f },
			{ 0.0f, -1.0f,  t },
			{ 0.0f,  1.0f,  t },
			{ 0.0f, -1.0f, -t },
			{ 0.0f,  1.0f, -t },
			{  t, 0.0f, -1.0f },
			{  t, 0.0f,  1.0f },
			{ -t, 0.0f, -1.0f },
			{ -t, 0.0f,  1.0f },
		};
		for (auto &v : vertices) {
			v.normalize();
		}
		std::vector<uint32_t> faces = {
			0, 11, 5,
			0, 5, 1,
			0, 1, 7,
			0, 7, 10,
			0, 10, 11,
			1, 5, 9,
			5, 11, 4,
			11, 10, 2,
			10, 7, 6,
			7, 1, 8,
			3, 9, 4,
			3, 4, 2,
			3, 2, 6,
			3, 6, 8,
			3, 8, 9,
			4, 9, 5,
			2, 4, 11,
			6, 2, 10,
			8, 6, 7,
			9, 8, 1,
		};
		// mesh subdivision
		std::unordered_map<uint64_t, uint32_t> divisions;
		std::vector<uint32_t> faces_out;

		// divide triangle edge
		auto subdivide_edge = [&divisions, &vertices] (uint32_t f0, uint32_t f1) -> uint32_t {
			uint64_t key;
			if (f0 > f1)
				std::swap(f0, f1);
			key = (uint64_t(f1) << 32) | f0;
			auto iter = divisions.find(key);
			if (iter != divisions.end())
				return iter->second;
			auto v3 = vertices[f0] + vertices[f1];
			v3 *= 0.5f;
			v3.normalize();
			auto i = (unsigned)vertices.size();
			vertices.emplace_back(v3);
			divisions.emplace(key, i);
			return i;
		};

		// loop over triangle list, divide each triangle into 4 sub triangles
		// final triangle count = 20 * (4 ^ smoothness)
		if (smoothness < 1)
			smoothness = 1;
		else if (smoothness > 4)
			smoothness = 4;
		for (unsigned l = 0; l < smoothness; ++l)
		{
			for (uint32_t i = 2; i < faces.size(); i += 3)
			{
				const uint32_t f0 = faces[i - 2];
				const uint32_t f1 = faces[i - 1];
				const uint32_t f2 = faces[i];

				const uint32_t f3 = subdivide_edge(f0, f1);
				const uint32_t f4 = subdivide_edge(f1, f2);
				const uint32_t f5 = subdivide_edge(f2, f0);

				faces_out.push_back(f0); faces_out.push_back(f3); faces_out.push_back(f5);
				faces_out.push_back(f3); faces_out.push_back(f1); faces_out.push_back(f4);
				faces_out.push_back(f4); faces_out.push_back(f2); faces_out.push_back(f5);
				faces_out.push_back(f3); faces_out.push_back(f4); faces_out.push_back(f5);
			}
			std::swap(faces, faces_out);
			faces_out.clear();
		}

		// build vertex buffer
		struct Vertex {
			vec3f pos;
			vec3f normal;
		};
		VertexAttribute attrib_array[] = {
			{ ATTR_POSITION, TinyImageFormat_R32G32B32_SFLOAT },
			{ ATTR_NORMAL, TinyImageFormat_R32G32B32_SFLOAT },
		};
		set_vertices(attrib_array, 2, (Vertex*)0, (unsigned)vertices.size());
		unsigned i = 0;
		for (auto &v : m_vb)
		{
			const auto &pos = vertices[i++];
			v = Vertex{ pos * r, pos };
		}
		m_ib.resize(faces.size(), vertices.size());
		if(m_ib.is_short())
		{
			uint16_t* buf = m_ib.data<uint16_t>();
			for (uint32_t v : faces)
				*buf++ = (uint16_t)v;
		}
		else
		{
			uint32_t* buf = m_ib.data<uint32_t>();
			memcpy(buf, faces.data(), sizeof(uint32_t) * faces.size());
		}
	}

} // namespace wyc
