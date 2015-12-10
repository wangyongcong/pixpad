#pragma once
#include <cstdint>

namespace wyc
{
	enum EAttributeUsage
	{
		ATTR_POSITION = 0,
		ATTR_COLOR,
		ATTR_TEXTURE,
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
		uint8_t elem_cnt;
		uint32_t offset;
	};

	class CVertexBuffer
	{
	public:
		CVertexBuffer();
		CVertexBuffer(const CVertexBuffer&) = delete;
		CVertexBuffer& operator = (const CVertexBuffer&) = delete;
		~CVertexBuffer();

		void set_attribute(EAttributeUsage usage, uint8_t element_count);
		void resize(unsigned vertex_count);

	protected:
		char *m_data;
		unsigned m_vert_size;
		unsigned m_vert_cnt;
		unsigned m_elem_cnt;
		VertexAttribute* m_attr_tbl[ATTR_MAX_COUNT];
	};



} // namespace wyc