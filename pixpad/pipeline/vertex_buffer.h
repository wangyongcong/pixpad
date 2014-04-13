#ifndef WYC_HEADER_VERTEX_BUFFER
#define WYC_HEADER_VERTEX_BUFFER

#include <cassert>
#include <cstddef>
#include <typeindex>
#include <vector>
#include <unordered_map>
#include "mathex/vecmath.h"

namespace wyc
{

	enum VERTEX_ATTRIBUTE
	{
		VERTEX_POSITION = 0,
		VERTEX_COLOR,
		VERTEX_NORMAL,
		VERTEX_TANGENT,
		VERTEX_UV,

		VERTEX_ATTRIBUTE_COUNT,
	};

	struct xvertex_attrib
	{
		VERTEX_ATTRIBUTE attrib_type;
		uint8_t component;
		std::type_index type;
		size_t offset;
	};

	struct vertex_p3
	{
		xvec3f_t position;
	};

	struct vertex_p3c3
	{
		xvec3f_t position;
		xvec3f_t color;
	};

	struct vertex_p3uv
	{
		xvec3f_t position;
		xvec2f_t uv;
	};

	template<typename VERTEX> 
	inline const xvertex_attrib* vertex_layout(unsigned &count) { return 0; }

	template<> inline const xvertex_attrib* vertex_layout<vertex_p3>(unsigned &count)
	{
		static xvertex_attrib _layout[1] = {
			{ VERTEX_POSITION, 3, std::type_index(typeid(float)), offsetof(vertex_p3, position) },
		};
		count = 1;
		return _layout;
	}

	template<> inline const xvertex_attrib* vertex_layout<vertex_p3c3>(unsigned &count)
	{
		static xvertex_attrib _layout[2] = {
			{ VERTEX_POSITION, 3, std::type_index(typeid(float)), offsetof(vertex_p3c3, position) },
			{ VERTEX_COLOR, 3, std::type_index(typeid(float)), offsetof(vertex_p3c3, color) },
		};
		count = 2;
		return _layout;
	}

	class xbasic_storage
	{
	public:
		xbasic_storage() : m_type(typeid(void))
		{
			m_data = nullptr;
			m_stride = 0;
			m_size = 0;
		}
		virtual ~xbasic_storage()
		{
			if (m_data) {
				delete[] m_data;
				m_data = nullptr;
			}
		}
		xbasic_storage(const xbasic_storage&) = delete;
		xbasic_storage& operator = (const xbasic_storage&) = delete;

		template<typename T>
		xbasic_storage& operator = (std::initializer_list<T> init_list) {
			if (storage<T>(init_list.size())) {
				T* ptr = reinterpret_cast<T*>(m_data);
				for (auto iter = init_list.begin(), end = init_list.end(); iter != end; ++iter, ++ptr)
					*ptr = *iter;
			}
			return *this;
		}
		template<typename T>
		bool storage(size_t count)
		{
			if (m_data)
				delete[] m_data;
			m_data = new T[count];
			m_type = std::type_index(typeid(T));
			m_size = count;
			m_stride = sizeof(T);
			return true;
		}
		void release() {
			if (!m_data)
				return;
			delete[] m_data;
			m_data = nullptr;
			m_type = typeid(void);
			m_size = 0;
			m_stride = 0;
		}
		const void* get_data() const {
			return m_data;
		}
		size_t size() const {
			return m_size;
		}
		size_t stride() const {
			return m_stride;
		}
		std::type_index type() const {
			return m_type;
		}
		size_t size_in_bytes() const
		{
			return m_size*m_stride;
		}
		template<typename T>
		T* get_as() {
			if (std::type_index(typeid(T)) != m_type) return 0;
			return reinterpret_cast<T*>(m_data);
		}
		template<typename T>
		const T* get_as() const {
			if (std::type_index(typeid(T)) != m_type) return 0;
			return reinterpret_cast<T*>(m_data);
		}
	protected:
		void *m_data;
		std::type_index m_type;
		size_t m_size;
		size_t m_stride;
	};

	class xvertex_buffer : public xbasic_storage
	{
		typedef xbasic_storage base;
	public:
		xvertex_buffer()
		{
			m_attribs = 0;
			m_attrib_cnt = 0;
		}
		template<typename T>
		bool storage(size_t vertex_count)
		{
			m_attribs = vertex_layout<T>(m_attrib_cnt);
			if (!m_attribs)
				return false;
			return base::storage<T>(vertex_count);
		}
		const xvertex_attrib* get_attribs() const
		{
			return m_attribs;
		}
		unsigned attrib_count() const
		{
			return m_attrib_cnt;
		}
		size_t set_position(std::initializer_list<float[3]> values) {
			if (!m_data)
				return 0;
			const xvertex_attrib &attr = m_attribs[VERTEX_POSITION];
			if (attr.component < 3 || attr.type != typeid(float))
				return 0;
			float *ptr = (float*)((uint8_t*)m_data + attr.offset);
			size_t cnt = 0;
			for (auto &v : values)
			{
				ptr[0] = v[0];
				ptr[1] = v[1];
				ptr[2] = v[2];
				ptr = (float*)((uint8_t*)ptr + m_stride);
				++cnt;
				if (cnt >= m_size)
					break;
			}
			return cnt;
		}
	private:
		const xvertex_attrib *m_attribs;
		unsigned m_attrib_cnt;
	};

	class xindex_buffer : public xbasic_storage
	{
		typedef xbasic_storage base;
	public:
		using base::operator=;
		template<typename T>
		bool storage(size_t vertex_count)
		{
			if (!is_available_type<T>())
				return false;
			return base::storage<T>(vertex_count);
		}
		template<typename T>
		size_t set(std::initializer_list<T> values)
		{
			if (m_type != typeid(T) || !m_data)
				return 0;
			T *ptr = reinterpret_cast<T*>(m_data);
			size_t cnt = 0;
			for (auto &v : values)
			{
				*ptr = v;
				++ptr;
				++cnt;
				if (cnt >= m_size)
					break;
			}
			return cnt;
		}
		template<typename T>
		static inline bool is_available_type() {
			return false;
		}
		template<> static inline bool is_available_type<unsigned char>() { return true; }
		template<> static inline bool is_available_type<unsigned short>() { return true; }
		template<> static inline bool is_available_type<unsigned int>() { return true; }
		template<> static inline bool is_available_type<unsigned long>() { return true; }
	};

	template<typename VERTEX>
	void gen_regular_triangle(float r, xvertex_buffer &vertices, xindex_buffer &indices);

	template<typename VERTEX>
	void gen_plane(float w, float h, xvertex_buffer &vertices, xindex_buffer &indices);

//
// Template implementations
//

	template<typename VERTEX>
	void gen_regular_triangle(float r, xvertex_buffer &vertices, xindex_buffer &indices)
	{
		vertices.storage<VERTEX>(3);
		double radian = 30 * 3.1415926 / 180;
		float x = float(r * std::cos(radian));
		float y = float(r * std::sin(radian));
		vertices.set_position({
			{ 0, -r, 0 }, { x, y, 0 }, {-x, y, 0}
		});

		indices.storage<uint8_t>(3);
		indices.set<uint8_t>({ 0, 1, 2 });
	}

	template<typename VERTEX>
	void gen_plane(float w, float h, xvertex_buffer &vertices, xindex_buffer &indices)
	{
		vertices.storage<VERTEX>(4);
		float x = w*0.5f;
		float y = h*0.5f;
		VERTEX *v = vertices.get_as<VERTEX>();
		v[0].position.set(-x, -y, 0);
		v[1].position.set(x, -y, 0);
		v[2].position.set(x, y, 0);
		v[3].position.set(-x, y, 0);

		indices.storage<uint8_t>(6);
		indices.set<uint8_t>({
			0, 1, 3,
			3, 1, 2,
		});
	}

}; // namespace wyc

#endif WYC_HEADER_VERTEX_BUFFER