#ifndef WYC_HEADER_PIPELINE
#define WYC_HEADER_PIPELINE

#include "vertex_buffer.h"
#include "mathex/transform.h"

namespace wyc
{
	class xpipeline
	{
	public:
		xpipeline();
		virtual ~xpipeline() {};
		virtual void set_viewport(unsigned width, unsigned height) = 0;
		// commit triangles to draw
		virtual bool set_material(const std::string &name) = 0;
		virtual void draw(xvertex_buffer *vertices, xindex_buffer *indices) = 0;
		virtual void flush() = 0;
		// set/get transform matrix
		void set_transform(const xmat4f_t &transform);
		const xmat4f_t& get_transform() const;
		// set/get projection matrix
		void set_perspective(float fov, float aspect, float dist_near, float dist_far);
		const xmat4f_t& get_projection() const;

	protected:
		xmat4f_t m_projection;
		xmat4f_t m_transform;
	};

	inline void xpipeline::set_transform(const xmat4f_t &transform)
	{
		m_transform = transform;
	}

	inline const xmat4f_t& xpipeline::get_transform() const
	{
		return m_transform;
	}

	inline void xpipeline::set_perspective(float vfov, float aspect, float dist_near, float dist_far)
	{
		wyc::set_perspective(m_projection, vfov, aspect, dist_near, dist_far);
	}

	inline const xmat4f_t& xpipeline::get_projection() const
	{
		return m_projection;
	}


}; // end of namespace wyc

#endif // WYC_HEADER_PIPELINE