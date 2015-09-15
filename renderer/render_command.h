#pragma once

#include <cstdint>
#include <functional>

#include "OpenEXR/ImathColor.h"

#include "util.h"

namespace wyc
{
	class command_allocator
	{
	public:
		command_allocator(size_t max_size, size_t capacity);
		~command_allocator();
		// Allocate raw memory
		void* alloc(size_t sz);
		// Free memory
		void free(void *ptr, size_t sz);
		// Recycle all allocated memory 
		void clear();
	private:
		DISALLOW_COPY_MOVE_AND_ASSIGN(command_allocator)

		struct chunk
		{
			chunk * next;
			void * memory;
			size_t count;
			size_t freed;
		};
		std::vector<chunk> m_chunks;
		size_t m_max_size;
	};


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