#pragma once

#include "renderer.h"
#include "spr_render_target.h"
#include "util.h"

namespace wyc
{
	class spr_renderer : public renderer
	{
	public:
		spr_renderer();
		virtual ~spr_renderer() override;
		virtual void set_render_target(std::shared_ptr<render_target> rt);
		virtual std::shared_ptr<render_target> get_render_target();
		virtual void clear(const Imath::C3f &c);

	private:
		DISALLOW_COPY_MOVE_AND_ASSIGN(spr_renderer);

		std::shared_ptr<spr_render_target> m_rt;
	};

} // namespace wyc
