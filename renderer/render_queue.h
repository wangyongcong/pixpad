#pragma once

#include "ring_queue.h"
#include "renderer.h"
#include "render_command.h"

namespace wyc
{
	template<class Renderer>
	class render_queue : public ring_queue<render_command*>
	{
	public:
		render_queue(size_t queue_size);
		~render_queue();

		template<class Command>
		Command* new_command(uint64_t commad_id);
	private:
		void *alloc(size_t sz);
	};

	template<class Renderer>
	inline render_queue<Renderer>::render_queue(size_t queue_size) : ring_queue<render_command*>(queue_size)
	{
	}

	template<class Renderer>
	inline render_queue<Renderer>::~render_queue()
	{
	}

	template<class Renderer>
	inline void * render_queue<Renderer>::alloc(size_t sz)
	{
		return NULL;
	}

	template<class Renderer>
	template<class Command>
	inline Command * render_queue<Renderer>::new_command(uint64_t commad_id)
	{
		static_assert(std::is_pod(Command));
		void *ptr = alloc(sizeof(Command));
		if (!ptr)
			return nullptr;
		Command *cmd = new(ptr) Command();
		cmd->id = commad_id;
		cmd->handler = &execute<Renderer, Command>;
		return cmd;
	}

} // namespace wyc