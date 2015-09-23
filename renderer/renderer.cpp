#include "renderer.h"

namespace wyc
{
	renderer::renderer() :
		m_cmd_queue(1024),
		m_cmd_alloc(page_size(), 16)
	{
	}

	renderer::~renderer()
	{
	}

	void renderer::get_ready()
	{
		auto future = m_is_ready.get_future();
		future.wait();
	}

	void renderer::set_ready()
	{
		m_is_ready.set_value();
	}

	void renderer::present()
	{
		auto *cmd = new_command<cmd_present>();
		m_is_done = std::move(cmd->is_done.get_future());
		enqueue(cmd);
	}

} // namespace wyc