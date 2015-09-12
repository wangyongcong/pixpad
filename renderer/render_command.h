#pragma once

#include <cstdint>
#include <functional>

#include "OpenEXR/ImathColor.h"

namespace wyc
{
	typedef uint64_t command_id;

	class renderer;
	struct render_command;
	typedef std::function<bool(renderer*, render_command*)> command_handler;

	struct render_command
	{
		command_id id = 0;
		render_command *next = nullptr;
		command_handler handler;
	};

	struct cmd_clear : public render_command
	{
		unsigned type;
		Imath::C3f value;
	};

}  // namespace wyc