#include "render_command.h"

#include <cassert>
#include <algorithm>

#include "unitest.h"
#include "log.h"

namespace wyc
{

	CCommandAllocator::CCommandAllocator(size_t chunk_size, unsigned short granularity)
	{
		constexpr size_t align = 8;
		constexpr size_t align_mask = ~(align - 1);
		granularity = (granularity + align - 1) & align_mask;
		assert(granularity > 1);
		m_grain = granularity;
		unsigned slot_cnt = (chunk_size + granularity - 1) / granularity;
		m_head_size = (sizeof(chunk_t) + granularity - 1) / granularity;
		assert(m_head_size < slot_cnt);
		m_capacity = slot_cnt - m_head_size;
		m_chunk_size = slot_cnt * granularity;
		m_chunk_count = 0;
		m_root = nullptr;
		new_chunk();
		log_debug("grain: %d, capacity: %d, chunk size: %d", m_grain, m_capacity, m_chunk_size);
	}

	CCommandAllocator::~CCommandAllocator()
	{
		clear();
	}

	void * CCommandAllocator::alloc(size_t sz)
	{
		unsigned slot_cnt = (sz + m_grain - 1) / m_grain;
		if (slot_cnt > m_capacity)
			return nullptr;
		if (m_root->used + slot_cnt > m_capacity)
		{
			if (!new_chunk())
				return nullptr;
		}
		uint8_t *ptr_raw = reinterpret_cast<uint8_t*>(m_root);
		ptr_raw += m_root->used * m_grain;
		m_root->used += slot_cnt;
		//debug("size: %d (%d), %d/%d", sz, slot_cnt, m_root->used, m_capacity);
		return ptr_raw;
	}

	void CCommandAllocator::reset()
	{
		chunk_t *ptr = m_root;
		while (ptr)
		{
			ptr->used = m_head_size;
			ptr = ptr->next;
		}
	}

	void CCommandAllocator::clear()
	{
		chunk_t *ptr;
		while (m_root)
		{
			ptr = m_root;
			m_root = m_root->next;
			free(ptr);
#ifdef _DEBUG
			m_chunk_count -= 1;
#endif
		}
		assert(0 == m_chunk_count);
		m_chunk_count = 0;
	}

	CCommandAllocator::chunk_t* CCommandAllocator::new_chunk()
	{
		chunk_t* ptr = static_cast<chunk_t*>(malloc(m_chunk_size));
		if (!ptr)
			return nullptr;
		ptr->used = m_head_size;
		ptr->next = m_root;
		m_root = ptr;
		m_chunk_count += 1;
		log_debug("new chunk %d", m_chunk_count);
		return ptr;
	}

} // namespace wyc

UNIT_TEST_BEG(render_command)

void test()
{
	wyc::CCommandAllocator allocator(4096, sizeof(wyc::RenderCommand));
	uint8_t sz_list[] = {
		16, 24, 48, 60, 72, 84, 96
	};
	unsigned count = 200;
	std::vector<uint8_t*> box;
	box.reserve(count);
	constexpr unsigned cnt = sizeof(sz_list) / sizeof(uint8_t);
	for (unsigned i = 0; i < count; ++i)
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
	allocator.clear();
}

UNIT_TEST_END