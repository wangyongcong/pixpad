#pragma once
#include <type_traits>
#include <OpenEXR/ImathVec.h>
#include <OpenEXR/ImathColor.h>

namespace wyc
{

	enum EAttribUsage
	{
		ATTR_POSITION = 0,
		ATTR_COLOR,
		ATTR_UV0,
		ATTR_UV1,
		ATTR_NORMAL,
		ATTR_TANGENT,
		ATTR_USAGE_0,
		ATTR_USAGE_1,
		ATTR_USAGE_2,
		ATTR_USAGE_3,
		ATTR_MAX_COUNT,
	};

	struct VertexAttrib
	{
		EAttribUsage usage;
		uint8_t component;
		uint16_t offset;
	};

} // namespace wyc