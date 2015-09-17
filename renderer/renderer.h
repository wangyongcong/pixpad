#pragma once

#include "OpenEXR/ImathColor.h"

#include "render_target.h"
#include "render_command.h"

namespace wyc
{
	class renderer
	{
	public:
		virtual ~renderer() {}
		virtual void set_render_target(std::shared_ptr<render_target> rt) = 0;
		virtual std::shared_ptr<render_target> get_render_target() = 0;
		virtual void process() = 0;
		virtual void present() = 0;
		virtual bool enqueue(render_command *cmd) = 0;
	};

} // namespace wyc