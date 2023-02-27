#pragma once
#include "engine.h"
#include "renderer/sparrow/renderer.h"
#include "renderer/sparrow/spw_render_target.h"
#include "renderer/sparrow/thread/ring_queue.h"
#include "renderer/sparrow/render_command.h"
#include "renderer/sparrow/spw_pipeline.h"
#include "common/utility.h"

namespace wyc
{
	class WYCAPI CSpwRenderer : public CRenderer
	{
	public:
		CSpwRenderer();
		virtual ~CSpwRenderer() override;
		virtual void set_render_target(std::shared_ptr<CRenderTarget> rt) override;
		virtual std::shared_ptr<CRenderTarget> get_render_target() override;
		virtual void process() override;
		void set_pipeline(std::shared_ptr<CSpwPipeline> pipeline);
		std::shared_ptr<CSpwPipeline> get_pipeline();
		// Internal implementation of render result presentation.
		std::function<void(void)> spw_present;
	
	protected:
		DISALLOW_COPY_MOVE_AND_ASSIGN(CSpwRenderer)

		template<class Command>
		friend void spw_handler(CSpwRenderer*, RenderCommand*);

		std::shared_ptr<CSpwRenderTarget> m_rt;
		std::shared_ptr<CSpwPipeline> m_pipeline;
		std::vector<RenderCommand*> m_cmd_buffer;
	};

	template<class Command>
	inline void spw_handler(CSpwRenderer * renderer, RenderCommand * _cmd)
	{
		throw "Not implemented.";
	}

	using spw_command_handler = void(*) (CSpwRenderer*, RenderCommand*);

	inline void CSpwRenderer::set_pipeline(std::shared_ptr<CSpwPipeline> pipeline)
	{
		m_pipeline = pipeline;
		if (m_rt)
			m_pipeline->set_render_target(m_rt);
	}

	inline std::shared_ptr<CSpwPipeline> wyc::CSpwRenderer::get_pipeline()
	{
		return m_pipeline;
	}

} // namespace wyc