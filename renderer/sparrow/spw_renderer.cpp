#include "spw_renderer.h"
#include "platform_info.h"
#include "log.h"
#include "spw_command.h"

#ifdef _DEBUG
#define CHECK_RENDER_TARGET(rt) if(!(rt)) {throw std::exception("Render target is not available.");}
#else
#define CHECK_RENDER_TARGET(rt)
#endif

namespace wyc
{

#define GET_HANDLER(cmd_type) &spw_handler<cmd_type>

	static render_command::handler_t spw_cmd_map[CMD_COUNT] = {
		GET_HANDLER(cmd_present),
		GET_HANDLER(cmd_clear),
	};

	spw_renderer::spw_renderer() : 
		m_rt(nullptr), 
		m_cmd_queue(1024), 
		m_cmd_alloc(page_size(), 16)
	{
		m_cmd_buffer.reserve(128);
	}

	spw_renderer::~spw_renderer()
	{
		m_rt = nullptr;
	}

	void spw_renderer::set_render_target(std::shared_ptr<render_target> rt)
	{
		m_rt = std::dynamic_pointer_cast<spw_render_target>(rt);
		if (!m_rt)
		{
			throw std::exception("Expect wyc::spr_render_target.");
		}
	}

	std::shared_ptr<render_target> spw_renderer::get_render_target()
	{
		return m_rt;
	}

	void spw_renderer::process()
	{
		if (m_cmd_queue.batch_dequeue(m_cmd_buffer))
		{
			for (auto cmd : m_cmd_buffer)
			{
				spw_cmd_map[cmd->get_tid()](this, cmd);
			}
			m_cmd_buffer.clear();
		}
	}

	void spw_renderer::present()
	{
		spw_present();
	}

} // namespace wyc
