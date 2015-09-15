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
		command_allocator(size_t min_size, size_t max_size, size_t chunk_size, size_t chunk_align);
		~command_allocator();
		// Allocate raw memory
		void* alloc(size_t sz);
		// Free memory
		void free(void *ptr, size_t sz);
		// Recycle all allocated memory 
		void clear();
	private:
		DISALLOW_COPY_MOVE_AND_ASSIGN(command_allocator)

		struct chunk_t
		{
			chunk_t * next = nullptr;
			void * memory = nullptr;
			size_t used = 0;
			size_t capacity = 0;
		};
		std::vector<chunk_t> m_chunks;
		size_t m_max_size;
		size_t m_chunk_align;
		size_t m_chunk_mask;
		size_t m_chunk_size;
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