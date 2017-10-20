#pragma once
#include <vector>
#include <iterator>
#include <type_traits>
#include "log.h"

class CConsoleLogger : public wyc::CLogger
{
public:
	static void init() {
		if (wyc::g_log) {
			delete wyc::g_log;
		}
		wyc::g_log = new CConsoleLogger(256);
	}

	virtual void write(const char* record, size_t size)
	{
		if (m_end < m_max_line) {
			m_log_buff.emplace_back(record);
			m_end += 1;
		}
		else {
			m_log_buff[m_end % m_max_line] = record;
			m_end += 1;
			m_beg += 1;
		}
	}

	class iterator : public std::iterator<std::forward_iterator_tag, std::string>
	{
	public:
		iterator(size_t p = 0, const std::vector<std::string> *b = nullptr) : pos(p), buff(b)
		{
		}
		inline const std::string& operator * ()
		{
			return (*buff)[pos % buff->size()];
		}
		inline const std::string* operator -> ()
		{
			return &(*buff)[pos % buff->size()];
		}
		inline bool operator != (const iterator &other) const
		{
			return pos != other.pos;
		}
		inline iterator& operator ++ ()
		{
			pos += 1;
			return *this;
		}
	private:
		size_t pos;
		const std::vector<std::string> *buff;
	};

	inline iterator begin() const {
		return{ m_beg, &m_log_buff };
	}

	inline iterator end() const {
		return{ m_end, nullptr };
	}

private:
	CConsoleLogger(size_t max_line)
		: CLogger()
		, m_beg(0)
		, m_end(0)
		, m_max_line(max_line)
	{
		if (m_max_line < 1)
			m_max_line = 1;
		m_log_buff.reserve(m_max_line);
	}

	std::vector<std::string> m_log_buff;
	size_t m_beg, m_end, m_max_line;
};

