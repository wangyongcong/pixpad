#pragma once

#include <cstdint>
#include <string>

namespace wyc
{
	extern std::string g_empty_string;

	typedef float float32_t;

	inline bool have_state(unsigned st, unsigned flag) {
		return flag == (st & flag);
	}
	inline void add_state(unsigned &st, unsigned flag) {
		st |= flag;
	}
	inline void set_state(unsigned &st, unsigned mask, unsigned flag) {
		st &= ~mask;
		st |= flag;
	}
	inline void remove_state(unsigned &st, unsigned flag) {
		st &= ~flag;
	}

	inline bool is_power2(unsigned val)
	{
		return (val&(val - 1)) == 0;
	}

	// return the minimal r, which (r = 2^a and r >= val)
	unsigned minimal_power2(unsigned val);

	// return log2(val), if val = 2^a (a >= 0)
	uint32_t log2p2(uint32_t val);

	// wstring to UTF-8 string
	bool wstr_to_utf8(const std::wstring& in_str, std::string& out_str);
	// UTF-8 string to wstring
	bool utf8_to_wstr(const std::string& in_str, std::wstring& out_str);
	// wstring to ANSI string (CP936)
	bool wstr_to_ansi(const std::wstring& in_str, std::string& out_str);
	// ANSI string (CP936) to wstring
	bool ansi_to_wstr(const std::string& in_str, std::wstring& out_str);

	const char* format_memory_size(size_t size, unsigned &out_size);
	const char* format_memory_size(size_t size, float &out_size);

	std::string get_file_ext(std::string file_path);

	class StringSplitter
	{
	public:
		StringSplitter(const std::string &str, char sep);
		~StringSplitter();

		StringSplitter(StringSplitter&) = delete;
		StringSplitter& operator= (const StringSplitter&) = delete;
		StringSplitter(StringSplitter&& other) noexcept;
		StringSplitter& operator= (StringSplitter&& other) noexcept;

		unsigned size() const
		{
			return m_count;
		}
		const char** tokens() const
		{
			return m_token_list;
		}

		class Iterator : public std::forward_iterator_tag
		{
		public:
			Iterator(const char** token_list, unsigned index)
				: m_token_list(token_list), m_index(index)
			{
			}

			bool operator== (const Iterator& other) const
			{
				assert(m_token_list == other.m_token_list);
				return m_index == other.m_index;
			}

			bool operator!= (const Iterator& other) const
			{
				assert(m_token_list == other.m_token_list);
				return m_index != other.m_index;
			}

			Iterator& operator++ ()
			{
				++m_index;
				return *this;
			}

			Iterator operator++ (int)
			{
				Iterator tmp = *this;
				++m_index;
				return tmp;
			}

			const char* operator* () const
			{
				return m_token_list[m_index];
			}

		private:
			const char** m_token_list;
			unsigned m_index;
		};

		Iterator begin() const
		{
			return Iterator{m_token_list, 0};
		}

		Iterator end() const
		{
			return Iterator(m_token_list, m_count);
		}

	private:
		const char** m_token_list;
		char* m_buff;
		unsigned m_count;
	};
} // end of namespace wyc

