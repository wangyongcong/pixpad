#pragma once
#include "tinyimageformat/tinyimageformat_base.h"
#include "tinyimageformat/tinyimageformat_query.h"

namespace wyc
{

	enum EAttributeUsage
	{
		ATTR_UNDEFINED,
		ATTR_POSITION,
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
		TinyImageFormat format;
		uint8_t channel:4;
		bool is_float:1;
		uint8_t size;
		uint8_t stream_index;
		uint16_t offset;
		
		VertexAttribute(EAttributeUsage usage, TinyImageFormat format, uint8_t stream_index=0)
			: usage(usage), stream_index(stream_index), offset(0)
		{
			set_format(format);
		}

		void set_format(TinyImageFormat format)
		{
			this->format = format;
			uint32_t ch = TinyImageFormat_ChannelCount(format);
			uint32_t sz = TinyImageFormat_BitSizeOfBlock(format) / 8;
			assert(ch < 16 && sz < 256);
			channel = ch;
			is_float = TinyImageFormat_IsFloat(format);
			size = (uint8_t)sz;
		}
	};

} // namespace wyc