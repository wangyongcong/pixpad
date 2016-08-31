#include "scene_obj.h"
#include "mtl_flat_color.h"

namespace wyc
{
	CSceneObj::CSceneObj()
		: m_pid(0)
	{
		m_transform.makeIdentity();
	}

	CSceneObj::~CSceneObj()
	{
	}

	void CSceneObj::render(std::shared_ptr<CRenderer> renderer, std::shared_ptr<CCamera> camera)
	{
	}

} // namespace wyc