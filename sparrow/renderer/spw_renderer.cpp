#include "spw_renderer.h"
#include "log.h"
#include "spw_command.h"

#ifdef _DEBUG
#define CHECK_RENDER_TARGET(rt) if(!(rt)) {throw std::exception("Render target is not available.");}
#else
#define CHECK_RENDER_TARGET(rt)
#endif

namespace wyc
{

	extern spw_command_handler spw_cmd_map[];

	CSpwRenderer::CSpwRenderer() :
		m_rt(nullptr)
	{
		m_cmd_buffer.reserve(128);
	}

	CSpwRenderer::~CSpwRenderer()
	{
		m_rt = nullptr;
	}

	void CSpwRenderer::set_render_target(std::shared_ptr<CRenderTarget> rt)
	{
		m_rt = std::dynamic_pointer_cast<CSpwRenderTarget>(rt);
		if (!m_rt)
		{
			throw std::exception("Expect wyc::spr_render_target.");
		}
	}

	std::shared_ptr<CRenderTarget> CSpwRenderer::get_render_target()
	{
		return m_rt;
	}

	void CSpwRenderer::process()
	{
		if (m_cmd_queue.batch_dequeue(m_cmd_buffer, m_cmd_buffer.capacity()))
		{
			for (auto &cmd : m_cmd_buffer)
			{
				spw_cmd_map[cmd->get_tid()](this, cmd);
			}
			m_cmd_buffer.clear();
		}
	}

} // namespace wyc
