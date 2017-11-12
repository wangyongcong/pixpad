#include "mesh.h"

#include <cassert>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <ImathVec.h>
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

	bool CMesh::load_obj(const std::string & path)
	{
		std::ifstream fin(path, std::ios_base::in);
		if (!fin.is_open())
		{
			log_error("Can't open file: %s", path.c_str());
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

		auto read_vector3 = [&ss](std::vector<Imath::V3f> &pool) {
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

		auto read_face = [&ss, &token, &faces, null_index]() {
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
			m_vb.set_attribute(ATTR_UV0, 2);
		if (has_normal)
			m_vb.set_attribute(ATTR_NORMAL, 3);
		// alloc buffer
		m_vb.resize(faces.size());
		// copy data
		auto pos = m_vb.get_attribute(ATTR_POSITION).begin();
		auto uv = m_vb.get_attribute(ATTR_UV0).begin();
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

	bool CMesh::load_ply(const std::string & path)
	{
		std::ifstream fin(path, std::ios_base::in);
		if (!fin.is_open())
		{
			log_error("Can't open file: %s", path.c_str());
			return false;
		}

		std::string line;
		std::getline(fin, line);
		if (line != "ply") {
			log_error("Invalid ply file: %s", path.c_str());
			return false;
		}
		std::string tags[] = {
			"comment",
			"format",
			"element",
			"property",
			"end_header",
		};
		enum PLY_TAG {
			COMMENT = 0,
			FORMAT,
			ELEMENT,
			PROPERTY,
			END_HEADER,
		};
		std::string element_types[] = {
			"vertex",
			"tristrips",
		};
		enum PLY_ELEMENT {
			VERTEX = 0,
			TRISTRIPS,
		};
		/*
		# name        type        number of bytes
		# ---------------------------------------
		# char       character                 1
		# uchar      unsigned character        1
		# short      short integer             2
		# ushort     unsigned short integer    2
		# int        integer                   4
		# uint       unsigned integer          4
		# float      single-precision float    4
		# double     double-precision float    8
		*/
		struct PlyProperty
		{
			std::string name;
			uint8_t size;
			bool is_int;
			PlyProperty *next;
		};
		struct PlyElement
		{
			std::string name;
			unsigned count;
			unsigned prop_count;
			PlyProperty *prop_lst;
			PlyElement *next;
		};
		auto clean_up = [](PlyElement *elem_lst) 
		{
			PlyElement *iter;
			PlyProperty *prop;
			while (elem_lst) {
				iter = elem_lst;
				while (iter->prop_lst) {
					prop = iter->prop_lst;
					iter->prop_lst = prop->next;
					delete prop;
				}
				elem_lst = elem_lst->next;
				delete iter;
			}
		};
		bool is_little_endian = true;
		bool is_binary = true;
		std::string format, version, value;
		PlyElement *elem_lst = nullptr;
		size_t beg, end;
		while (fin) {
			std::getline(fin, line);
			if (0 == line.compare(0, tags[END_HEADER].size(), tags[END_HEADER]))
				break;
			if (0 == line.compare(0, tags[COMMENT].size(), tags[COMMENT]))
				continue;
			end = line.find(' ');
			if (end == std::string::npos)
				continue;
			log_info(line.c_str());
			if (0 == line.compare(0, end, tags[FORMAT]))
			{
				beg = end + 1;
				end = line.find(' ', beg);
				if (end == std::string::npos) {
					log_error("ply loader: unknown format [%s]", line.c_str());
					return false;
				}
				format = line.substr(beg, end - beg);
				if (format == "ascii")
					is_binary = false;
				else if (format == "is_big_endian")
					is_little_endian = false;
				beg = end + 1;
				version = line.substr(beg);
			}
			else if (0 == line.compare(0, end, tags[ELEMENT]))
			{
				PlyElement *elem = new PlyElement;
				elem->next = elem_lst;
				elem_lst = elem;
				elem->prop_count = 0;
				elem->prop_lst = nullptr;
				// element name
				beg = end + 1;
				end = line.find(' ', beg);
				if (end == std::string::npos) {
					log_error("ply loader: invalid element [%s]", line.c_str());
					clean_up(elem_lst);
					return false;
				}
				elem->name = line.substr(beg, end - beg);
				// element count
				value = line.substr(end + 1);
				try {
					elem->count = std::stoi(value);
				}
				catch (std::invalid_argument) {
					log_error("ply loader: invalid element [%s]", line.c_str());
					clean_up(elem_lst);
					return false;
				}
			}
			else if (0 == line.compare(0, end, tags[PROPERTY]))
			{
				PlyProperty *prop = new PlyProperty;
				prop->next = elem_lst->prop_lst;
				elem_lst->prop_lst = prop;
				elem_lst->prop_count += 1;
				// property type
				beg = end + 1;
				end = line.find(' ', beg);
				if (end == std::string::npos) {
					log_error("ply loader: invalid property [%s]", line.c_str());
					clean_up(elem_lst);
					return false;
				}
				value = line.substr(beg, end - beg);
				if (value == "float") {
					prop->is_int = false;
					prop->size = 4;
				}
				else if (value == "double") {
					prop->is_int = false;
					prop->size = 8;
				}
				// property name
				prop->name = line.substr(end + 1);
			}
		}
		clean_up(elem_lst);
		return true;
	}

	void CMesh::create_triangle(float r)
	{
		struct Vertex {
			Imath::V3f pos;
			Imath::C3f color;
		};
		VertexAttrib attrib_array[] = {
			{ATTR_POSITION, 3, offsetof(Vertex, pos)},
			{ATTR_COLOR, 3, offsetof(Vertex, color)},
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
			Imath::V3f pos;
			Imath::C3f color;
		};
		VertexAttrib attrib_array[] = {
			{ ATTR_POSITION, 3, offsetof(Vertex, pos) },
			{ ATTR_COLOR, 3, offsetof(Vertex, color) },
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
			Imath::V3f pos;
			Imath::C3f color;
		};
		VertexAttrib attrib_array[] = {
			{ ATTR_POSITION, 3, offsetof(Vertex, pos) },
			{ ATTR_COLOR, 3, offsetof(Vertex, color) },
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
			Imath::V3f pos;
			Imath::C4f color;
			Imath::V2f uv;
		};
		VertexAttrib attrib_array[] = {
			{ATTR_POSITION, 3, offsetof(Vertex, pos)},
			{ATTR_COLOR, 4, offsetof(Vertex, color)},
			{ATTR_UV0, 2, offsetof(Vertex, color)},
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

	void CMesh::create_sphere(float r)
	{
	}

} // namespace wyc