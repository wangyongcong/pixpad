#pragma once

#include "render_target.h"

#include "OpenEXR/ImathColor.h"

namespace wyc
{
	class renderer
	{
	public:
		virtual ~renderer() {}
		virtual void set_render_target(std::shared_ptr<render_target> rt) = 0;
		virtual std::shared_ptr<render_target> get_render_target() = 0;
		virtual void clear(const Imath::C3f &c) = 0;
	};

} // namespace wyc