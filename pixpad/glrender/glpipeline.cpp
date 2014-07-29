#include "glpipeline.h"

#include "util/log.h"

namespace wyc
{
	const std::unordered_map<std::type_index, GLenum> xgl_pipeline::ms_gltype = {
		{ std::type_index(typeid(float)), GL_FLOAT },
		{ std::type_index(typeid(double)), GL_DOUBLE },
		{ std::type_index(typeid(int)), GL_INT },
		{ std::type_index(typeid(unsigned int)), GL_UNSIGNED_INT },
		{ std::type_index(typeid(short)), GL_SHORT },
		{ std::type_index(typeid(unsigned short)), GL_UNSIGNED_SHORT },
		{ std::type_index(typeid(char)), GL_BYTE },
		{ std::type_index(typeid(unsigned char)), GL_UNSIGNED_BYTE },
	};

	xgl_pipeline::xgl_pipeline()
	{
		m_vertex_array = 0;
		m_vertex_buffer = 0;
		m_index_buffer = 0;
		m_index_type = GL_UNSIGNED_INT;
		m_index_count = 0;
		m_attribs = 0;
		m_attrib_count = 0;
		m_program = 0;
	}

	xgl_pipeline::~xgl_pipeline()
	{
		if (m_vertex_array)
			del_hw_buffers();
		if (m_program) {
			glDeleteProgram(m_program);
			m_program = 0;
		}
	}

	void xgl_pipeline::del_hw_buffers()
	{
		assert(m_vertex_array && m_vertex_buffer && m_index_buffer);
		glDeleteVertexArrays(1, &m_vertex_array);
		GLuint vbo[2] = { m_vertex_buffer, m_index_buffer };
		glDeleteBuffers(2, vbo);
		m_vertex_array = 0;
		m_vertex_buffer = 0;
		m_index_buffer = 0;
	}

	bool xgl_pipeline::new_hw_buffers()
	{
		assert(!m_vertex_array && !m_vertex_buffer && !m_index_buffer);
		GLuint vao;
		glGenVertexArrays(1, &vao);
		if (!vao)
			return false;
		GLuint vbo[2];
		glGenBuffers(2, vbo);
		if (!vbo[0] || !vbo[1]) {
			glDeleteVertexArrays(1, &vao);
			return false;
		}
		m_vertex_array = vao;
		m_vertex_buffer = vbo[0];
		m_index_buffer = vbo[1];
		return true;
	}

	void xgl_pipeline::set_viewport(unsigned width, unsigned height)
	{

	}

	bool xgl_pipeline::set_material(const std::string &name)
	{
		if (m_program) {
			glDeleteProgram(m_program);
			m_program = 0;
		}
		std::string path;
		std::string src = "material/";
		src += name;
		GLuint shader[2];
		// vertex shader
		path = src;
		path += ".vs";
		shader[0] = glsl_load_file(GL_VERTEX_SHADER, path.c_str());
		if (!shader[0]) {
			error("Failed to load vertex shader: %s", path.c_str());
			return false;
		}
		// fragment shader
		path = src;
		path += ".fs";
		shader[1] = glsl_load_file(GL_FRAGMENT_SHADER, path.c_str());
		if (!shader[1]) {
			error("Failed to load fragment shader: %s", path.c_str());
			glDeleteShader(shader[0]);
			return false;
		}
		m_program = glsl_build_shader(shader, 2);
		glDeleteShader(shader[0]);
		glDeleteShader(shader[1]);
		if (!m_program) {
			error("Failed to build program");
			return false;
		}
		const xvertex_attrib *attrib = m_attribs;
		for (unsigned i = 0; i < m_attrib_count; ++i, ++attrib)
		{
			glBindAttribLocation(m_program, i, ATTRIBUTE_NAMES[attrib->attrib_type]);
		}
		glUseProgram(m_program);
		return true;
	}

	void xgl_pipeline::draw(xvertex_buffer *vertices, xindex_buffer *indices)
	{
		if (!vertices)
			return;
		if (!m_vertex_array) {
			if (!new_hw_buffers())
				return;
		}
		glBindVertexArray(m_vertex_array);
		glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer);
		glBufferData(GL_ARRAY_BUFFER, vertices->size_in_bytes(), vertices->get_data(), GL_STATIC_DRAW);
		// bind vetex attributes
		size_t stride = vertices->stride();
		unsigned count = vertices->attrib_count();
		const xvertex_attrib* attrib = vertices->get_attribs();
		m_attribs = attrib;
		m_attrib_count = count;
		for (unsigned i = 0; i < count; ++i, ++attrib)
		{
			glVertexAttribPointer(i, attrib->component, ms_gltype.at(attrib->type), GL_TRUE, stride, (GLvoid*)attrib->offset);
			glEnableVertexAttribArray(i);
		}
		m_index_count = 0;
		if (indices) {
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_index_buffer);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices->size_in_bytes(), indices->get_data(), GL_STATIC_DRAW);
			m_index_type = ms_gltype.at(indices->type());
			m_index_count = indices->size();
		}
	}

	void xgl_pipeline::flush()
	{
		if (!m_program || !m_vertex_array || !m_index_count)
			return;
		GLuint uf;
		uf = glGetUniformLocation(m_program, "transform");
		if (-1 != uf)
			glUniformMatrix4fv(uf, 1, GL_TRUE, m_transform.data());
		uf = glGetUniformLocation(m_program, "projection");
		if (-1 != uf)
			glUniformMatrix4fv(uf, 1, GL_TRUE, m_projection.data());
		glDrawElements(GL_TRIANGLES, m_index_count, m_index_type, 0);
	}


}; // namespace wyc
