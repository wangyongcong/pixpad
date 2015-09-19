#pragma once

#include "renderer.h"
#include "spw_render_target.h"
#include "ring_queue.h"
#include "render_command.h"
#include "util.h"

namespace wyc
{
	class spw_renderer : public renderer
	{
	public:
		spw_renderer();
		virtual ~spw_renderer() override;
		
		virtual void set_render_target(std::shared_ptr<render_target> rt) override;
		virtual std::shared_ptr<render_target> get_render_target() override;
		virtual void process() override;
		virtual void present() override;

		// Internal implementation of render result presentation.
		std::function<void(void)> spw_present;
	
	protected:
		DISALLOW_COPY_MOVE_AND_ASSIGN(spw_renderer)

		template<class Command>
		friend bool spw_handler(spw_renderer*, render_command*);

		std::shared_ptr<spw_render_target> m_rt;
		std::vector<render_command*> m_cmd_buffer;
	};

	template<class Command>
	inline bool spw_handler(spw_renderer * renderer, render_command * _cmd)
	{
		static_assert(0, "Not implemented.");
	}

} // namespace wyc
