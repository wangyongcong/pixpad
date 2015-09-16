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
		command_allocator(size_t chunk_size, size_t granularity);
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
			uint8_t * memory = nullptr;
			unsigned used = 0;
			unsigned capacity = 0;
		};
		struct note_t
		{
			note_t *next;
		};
		std::vector<chunk_t> m_chunks;
		note_t *m_raw_mem;
		note_t *m_free;
		size_t m_chunk_size;
		size_t m_grain;
		size_t m_grain_mask;

		bool new_chunk(chunk_t *ptr_chunk);
		void release_chunks();
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
		unsigned type;
		Imath::C3f value;
	};

}  // namespace wyc

#include "unitest.h"
UNIT_TEST(render_command)
