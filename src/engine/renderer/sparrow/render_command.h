#pragma once

#include <cstdint>
#include <functional>
#include <vector>
#include <future>
#include <ImathForward.h>
#include <ImathColor.h>
#include "common/util_macros.h"
#include "renderer/mesh.h"
#include "renderer/sparrow/material.h"

namespace wyc
{
	class CCommandAllocator
	{
	public:
		CCommandAllocator(unsigned chunk_size, unsigned short granularity);
		~CCommandAllocator();
		// Allocate raw memory
		void* alloc(unsigned sz);
		// Recycle all allocated memory 
		void reset();
		// Release physical memory
		void clear();

	private:
		DISALLOW_COPY_MOVE_AND_ASSIGN(CCommandAllocator)

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

	class CRenderer;

	struct RenderCommand
	{
		using handler_t = void (*) (CRenderer*, RenderCommand*);
		command_id id = 0;
		RenderCommand *next = nullptr;

		inline void set_tid(unsigned short tid)
		{
			this->id |= tid & 0x3FF;
		}
		inline unsigned short get_tid() const
		{
			return this->id & 0x3FFF;
		}
	};

	enum ECommandType
	{
		CMD_TEST = 0,
		CMD_PRESENT = 1,
		CMD_CLEAR,
		CMD_DRAW_MESH,

		CMD_COUNT
	};

#define RENDER_CMD(cmd_name) struct cmd_name : public RenderCommand
#define CMD_TID(id) static const ECommandType tid = id

	RENDER_CMD(cmd_test)
	{
		CMD_TID(CMD_TEST);
		std::vector<int> *jobs;
	};

	RENDER_CMD(cmd_present)
	{
		CMD_TID(CMD_PRESENT);
		std::promise<void> is_done;
	};

	RENDER_CMD(cmd_clear)
	{
		CMD_TID(CMD_CLEAR);
		unsigned type = 0;
		color4f color = { 0, 0, 0, 1.f };
		float clear_z = 1;
	};

	RENDER_CMD(cmd_draw_mesh)
	{
		CMD_TID(CMD_DRAW_MESH);
		const CMesh *mesh = nullptr;
		const CMaterial *material = nullptr;

	};

}  // namespace wyc

#include "common/unitest.h"
UNIT_TEST(render_command)
