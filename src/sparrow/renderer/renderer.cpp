#include "renderer.h"

namespace wyc
{
	CRenderer::CRenderer()
	: m_cmd_alloc(page_size(), 16)
	, m_cmd_queue(1024)
	{
	}

	CRenderer::~CRenderer()
	{
	}

	void CRenderer::wait_for_ready()
	{
		auto future = m_is_ready.get_future();
		future.wait();
	}

	void CRenderer::set_ready()
	{
		m_is_ready.set_value();
	}

	void CRenderer::present()
	{
		auto *cmd = new_command<cmd_present>();
		m_is_done = cmd->is_done.get_future();
		enqueue(cmd);
	}

} // namespace wyc
