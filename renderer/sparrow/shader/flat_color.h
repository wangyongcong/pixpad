#pragma once
#include <OpenEXR/ImathMatrix.h>
#include <vertex_layout.h>
#include <shader.h>
#include <material.h>
#include <mathex/mathfwd.h>
#include <mathex/ImathMatrixExt.h>

namespace wyc
{
	class CMaterialFlatColor : public CMaterial
	{
	public:
		// material property
		Color4f color;
		typedef Vec4f VertexOut;
		struct ShaderOutput {
			size_t vertex_size;
			size_t pos_offset;
		};
		static ShaderOutput ms_shader_output;
		// shader interface
		virtual bool bind_vertex(const CVertexBuffer &vb) override;
		virtual void vertex_shader(const float *vertex_in, float *vertex_out, Vec4f &clip_pos) const override;
		virtual bool fragment_shader(const float *vertex_out, Color4f &frag_color) const override;
		virtual const ShaderOutput& get_output_layout() const {
			return ms_shader_output;
		}

	private:
		struct {
			const Vec3f *pos_stream;
			size_t pos_stride;
			size_t count;
		} m_input;
	};

	class CMaterialPhoneColor : public CMaterial
	{
	public:
		struct VertexIn {
			const Vec3f *pos;
			const Color4f *color;
		};

		struct VertexOut {
			Vec4f pos;
			Color4f color;
		};

		virtual const AttribDefine& get_attrib_define() const
		{
			static AttribSlot ls_in_attribs[] = {
				{ ATTR_POSITION, 3 },
				{ ATTR_COLOR, 4 },
			};
			static AttribSlot ls_out_attirbs[] = {
				{ ATTR_POSITION, 4 },
				{ ATTR_COLOR, 4 },
			};
			static AttribDefine ls_attrib_define = {
				ls_in_attribs,
				2,
				7,
				ls_out_attirbs,
				2,
				8,
			};
			return ls_attrib_define;
		}

		virtual const std::unordered_map<std::string, CUniform>& get_uniform_define() const
		{
			DECLARE_UNIFORM_MAP(CMaterialPhoneColor) {
				MAKE_UNIFORM(Matrix44f, mvp_matrix)
			};
			return UNIFORM_MAP;
		}

		virtual void vertex_shader(const void *vertex_in, void *vertex_out) const override
		{
			VertexIn *in = (VertexIn*)vertex_in;
			Vec4f pos(*in->pos);
			pos.z = -1.0f;
			pos = this->mvp_matrix * pos;
			VertexOut *out = (VertexOut*)vertex_out;
			out->pos = pos;
			out->color = *in->color;
		}

		virtual bool fragment_shader(const void *vertex_out, Color4f &frag_color) const override
		{
			const VertexOut* out = reinterpret_cast<const VertexOut*>(vertex_out);
			frag_color = out->color;
			return true;
		}

	protected:
		Matrix44f mvp_matrix;
	};

	template<class VertexIn, class VertexOut>
	class CShaderFlatColor : public IShaderProgram
	{
	public:
		const CMaterialFlatColor *material;

		virtual bool bind_vertex(const CVertexBuffer &vb) override
		{
			if (vb.attrib_component(ATTR_POSITION) < 3)
				return false;
			m_input.pos_stream = reinterpret_cast<const Vec3f*>(vb.attrib_stream(ATTR_POSITION));
			m_input.pos_stride = vb.attrib_stride(ATTR_POSITION);
			m_input.count = vb.size();
			return true;
		}

		virtual void vertex_shader(const float *vertex_in, float *vertex_out, Vec4f &clip_pos) const override 
		{
			const VertexIn* in = reinterpret_cast<const VertexIn*>(vertex_in);
			VertexOut* out = reinterpret_cast<VertexOut*>(vertex_out);
			Vec4f pos(in->pos);
			pos.z = -1.0f;
			pos = material->mvp_matrix * pos;
			clip_pos = pos;
			out->pos = in->pos;
		}

		virtual bool fragment_shader(const float *vertex_out, Color4f &frag_color) const override 
		{
			const VertexOut* vout = reinterpret_cast<const VertexOut*>(vertex_out);
			frag_color = { material->color.r, material->color.g, material->color.b, material->color.a };
			return true;
		}

		virtual size_t get_vertex_stride() const override {
			return VertexOut::Layout::component;
		}

		virtual size_t get_vertex_size() const override {
			return sizeof(VertexOut);
		}

	private:
		struct {
			const Vec3f *pos_stream;
			size_t pos_stride;
			size_t count;
		} m_input;

	};

} // namespace