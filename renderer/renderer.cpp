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

} // namespace wyc