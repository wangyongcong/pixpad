#include "render_command.h"

#include <cassert>
#include <algorithm>

namespace wyc
{

	command_allocator::command_allocator(size_t min_size, size_t max_size, size_t chunk_size, size_t chunk_align) :
		m_chunk_size(chunk_size),
		m_chunk_align(chunk_align)
	{
		// must be pow(2)
		assert((chunk_size & (chunk_size - 1)) == 0);
		assert((chunk_align & (chunk_align - 1)) == 0);
		// 8 bytes alignment
		m_chunk_mask = ~(chunk_align - 1);
		min_size = (min_size + chunk_align - 1) & m_chunk_mask;
		max_size = (max_size + chunk_align - 1) & m_chunk_mask;
		m_max_size = std::max<size_t>(min_size, max_size);
		unsigned cnt_chunk = m_max_size / min_size;
		m_chunks.resize(cnt_chunk);
	}

	command_allocator::~command_allocator()
	{
	}

	void * command_allocator::alloc(size_t sz)
	{
		sz = (sz + m_chunk_align - 1) & m_chunk_mask;
		unsigned idx = sz / m_chunk_align - 1;
		if (idx >= m_chunks.size())
		{
			return nullptr;
		}
		chunk_t *ptr_chunk = &m_chunks[idx];
		if (!ptr_chunk->memory)
		{
		}
		else
		{
			while (!ptr_chunk->capacity)
			{
				if (!ptr_chunk->next)
				{
					ptr_chunk->next = new chunk_t;
					ptr_chunk = ptr_chunk->next;
					break;
				}

			}
		}
		return nullptr;
	}

	void command_allocator::free(void * ptr, size_t sz)
	{
	}

	void command_allocator::clear()
	{
	}

} // namespace wyc