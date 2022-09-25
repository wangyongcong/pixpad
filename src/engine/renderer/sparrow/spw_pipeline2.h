#pragma once
//#include <enoki/array.h>
#include "renderer/mesh.h"
#include "renderer/sparrow/material.h"

namespace wyc {

class CSpwTilePipeline
{
public:
	CSpwTilePipeline();
	~CSpwTilePipeline();
	virtual void draw(const CMesh *mesh, const CMaterial *material);
	
protected:
};

} // namespace wyc
