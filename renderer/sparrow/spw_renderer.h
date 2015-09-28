#pragma once

#include "renderer.h"
#include "spw_render_target.h"
#include "ring_queue.h"
#include "render_command.h"
#include "util.h"

namespace wyc
{
	class CSpwRenderer : public CRenderer
	{
	public:
		CSpwRenderer();
		virtual ~CSpwRenderer() override;
		
		virtual void set_render_target(std::shared_ptr<CRenderTarget> rt) override;
		virtual std::shared_ptr<CRenderTarget> get_render_target() override;
		virtual void process() override;

		// Internal implementation of render result presentation.
		std::function<void(void)> spw_present;
	
	protected:
		DISALLOW_COPY_MOVE_AND_ASSIGN(CSpwRenderer)

		template<class Command>
		friend void spw_handler(CSpwRenderer*, RenderCommand*);

		std::shared_ptr<CSpwRenderTarget> m_rt;
		std::vector<RenderCommand*> m_cmd_buffer;
	};

	template<class Command>
	inline void spw_handler(CSpwRenderer * renderer, RenderCommand * _cmd)
	{
		static_assert(0, "Not implemented.");
	}

} // namespace wyc
