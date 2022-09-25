#pragma once
#include <type_traits>
#include <ImathVec.h>
#include <ImathColor.h>

namespace wyc
{

	enum EAttributeUsage
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

	struct VertexAttribute
	{
		EAttributeUsage usage;
		uint8_t component;
		uint16_t offset;
	};

} // namespace wyc