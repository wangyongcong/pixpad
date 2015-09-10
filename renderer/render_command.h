#pragma once

#include <cstdint>
#include <functional>

#include "OpenEXR/ImathColor.h"

namespace wyc
{
	struct render_command;
	typedef std::function<bool(render_command*)> command_handler;

	struct render_command
	{
		uint64_t id;
		render_command *next;
		command_handler handler;
	};

	struct cmd_clear : public render_command
	{
		unsigned type;
		Imath::C3f value;
	};

}  // namespace wyc