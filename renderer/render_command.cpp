#include "render_command.h"

#include <cassert>
#include <algorithm>

#include "unitest.h"

namespace wyc
{

	command_allocator::command_allocator(size_t chunk_size, size_t granularity) :
		m_raw_mem(nullptr),
		m_free(nullptr)
	{
		constexpr size_t align = 8;
		constexpr size_t align_mask = ~(align - 1);
		granularity = (granularity + align - 1) & align_mask;
		assert(granularity > 0 && (granularity & (granularity - 1)) == 0);
		m_grain = granularity;
		m_grain_mask = ~(granularity - 1);
		m_chunk_size = (chunk_size + granularity - 1) & m_grain_mask;
		assert(granularity <= m_chunk_size);
		unsigned cnt_chunk = m_chunk_size / granularity;
		m_chunks.resize(cnt_chunk);
		size_t cap = granularity;
		for (auto &chk : m_chunks)
		{
			chk.capacity = m_chunk_size / cap;
			cap += granularity;
		}
	}

	command_allocator::~command_allocator()
	{
		release_chunks();
	}

	void * command_allocator::alloc(size_t sz)
	{
		sz = (sz + m_grain - 1) & m_grain_mask;
		unsigned idx = sz / m_grain - 1;
		if (idx >= m_chunks.size())
		{
			return nullptr;
		}
		auto ptr_chunk = &m_chunks[idx];
		if (!ptr_chunk->memory || ptr_chunk->used == ptr_chunk->capacity)
		{
			if (!new_chunk(ptr_chunk))
				return nullptr;
		}
		uint8_t *slot = ptr_chunk->memory + ptr_chunk->used * sz;
		assert(slot + sz <= ptr_chunk->memory + m_chunk_size);
		ptr_chunk->used += 1;
		return slot;
	}

	void command_allocator::reset()
	{
		m_free = m_raw_mem;
		for (auto &chk : m_chunks)
		{
			chk.used = 0;
			chk.memory = nullptr;
		}
	}

	void command_allocator::clear()
	{
		reset();
		release_chunks();
	}

	bool command_allocator::new_chunk(chunk_t * ptr_chunk)
	{
		if (m_free)
		{
			ptr_chunk->memory = reinterpret_cast<uint8_t*>(m_free);
			m_free = m_free->next;
			ptr_chunk->used = 0;
			return true;
		}
		note_t *ptr_raw = static_cast<note_t*>(malloc(m_chunk_size));
		ptr_raw->next = m_raw_mem;
		m_raw_mem = ptr_raw;
		ptr_chunk->memory = reinterpret_cast<uint8_t*>(ptr_raw);
		ptr_chunk->used = 0;
		return true;
	}

	void command_allocator::release_chunks()
	{
		m_free = m_raw_mem;
		while (m_free)
		{
			auto ptr = m_free;
			m_free = m_free->next;
			free(ptr);
		}
		m_raw_mem = nullptr;
	}

} // namespace wyc

UNIT_TEST_BEG(render_command)

void test()
{
	wyc::command_allocator allocator(1024, sizeof(wyc::render_command));
	uint8_t sz_list[] = {
		16, 24, 48, 60, 72, 84, 96
	};
	std::vector<uint8_t*> box;
	constexpr unsigned cnt = sizeof(sz_list) / sizeof(uint8_t);
	for (unsigned i = 0; i < 1024; ++i)
	{
		uint8_t sz = sz_list[i % cnt];
		uint8_t *ptr = static_cast<uint8_t*>(allocator.alloc(sz));
		memset(ptr, sz, sz);
		box.push_back(ptr);
	}
	for (auto ptr : box)
	{
		uint8_t sz = ptr[0];
		for (unsigned i = 0; i < sz; ++i)
		{
			assert(ptr[i] == sz);
		}
	}
	allocator.reset();
}

UNIT_TEST_END