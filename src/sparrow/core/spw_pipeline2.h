#pragma once
//#include <enoki/array.h>
#include "mesh.h"
#include "material.h"

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
