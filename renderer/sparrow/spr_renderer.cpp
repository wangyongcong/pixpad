#include "spr_renderer.h"

#include "OpenEXR/ImathColorAlgo.h"

#ifdef _DEBUG
#define CHECK_RENDER_TARGET(rt) if(!(rt)) {throw std::exception("Render target is not available.");}
#else
#define CHECK_RENDER_TARGET(rt)
#endif

namespace wyc
{
	spr_renderer::spr_renderer() : m_rt(nullptr), m_cmd_queue(1024)
	{
		this->new_command<cmd_clear>(0);
	}

	spr_renderer::~spr_renderer()
	{
		m_rt = nullptr;
	}

	void spr_renderer::set_render_target(std::shared_ptr<render_target> rt)
	{
		m_rt = std::dynamic_pointer_cast<spr_render_target>(rt);
		if (!m_rt)
		{
			throw std::exception("Expect wyc::spr_render_target.");
		}
	}

	std::shared_ptr<render_target> spr_renderer::get_render_target()
	{
		return m_rt;
	}

	void spr_renderer::clear(const Imath::C3f & c)
	{
		CHECK_RENDER_TARGET(m_rt)

		xsurface& surf = m_rt->get_color_buffer();
		uint32_t v = Imath::rgb2packed(c);
		surf.clear(v);
	}

} // namespace wyc
