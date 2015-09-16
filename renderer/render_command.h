#pragma once

#include <cstdint>
#include <functional>
#include <vector>

#include "OpenEXR/ImathColor.h"

#include "util.h"

namespace wyc
{
	class command_allocator
	{
	public:
		command_allocator(size_t chunk_size, unsigned short granularity);
		~command_allocator();
		// Allocate raw memory
		void* alloc(size_t sz);
		// Recycle all allocated memory 
		void reset();
		// Release physical memory
		void clear();

	private:
		DISALLOW_COPY_MOVE_AND_ASSIGN(command_allocator)

		struct chunk_t
		{
			chunk_t * next = nullptr;
			unsigned short used = 0;
		};

		chunk_t* m_root;
		size_t m_chunk_size;
		unsigned short m_chunk_count;
		unsigned short m_grain;
		unsigned short m_capacity;
		unsigned short m_head_size;

		chunk_t* new_chunk();
	};


	typedef uint64_t command_id;

	class renderer;
	struct render_command;
	using command_handler = bool(*) (renderer*, render_command*);

	struct render_command
	{
		command_id id = 0;
		render_command *next = nullptr;
		command_handler handler;
	};

	struct cmd_clear : public render_command
	{
		unsigned type = 0;
		Imath::C3f clear_color;
	};

}  // namespace wyc

#include "unitest.h"
UNIT_TEST(render_command)
