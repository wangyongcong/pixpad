#pragma once
#include <vector>
#include <iterator>
#include <type_traits>
#include "stb_log.h"

class CConsoleLogger : public wyc::ILogger
{
public:
	static void init() {
		auto ptr = new CConsoleLogger(256);
		LOGGER_SET(ptr);
	}

	virtual void write(wyc::ELogLevel lvl, const char *fmt, ...) override;

	void output(const char *buf)
	{
		if (m_end < m_max_line) {
			m_log_buf.emplace_back(buf);
			m_end += 1;
		}
		else {
			m_log_buf[m_end % m_max_line] = buf;
			m_end += 1;
			m_beg += 1;
		}
	}

	class iterator : public std::iterator<std::forward_iterator_tag, std::string>
	{
	public:
		iterator(size_t p = 0, const std::vector<std::string> *b = nullptr) : pos(p), buf(b)
		{
		}
		inline const std::string& operator * ()
		{
			return (*buf)[pos % buf->size()];
		}
		inline const std::string* operator -> ()
		{
			return &(*buf)[pos % buf->size()];
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
		const std::vector<std::string> *buf;
	};

	inline iterator begin() const {
		return{ m_beg, &m_log_buf };
	}

	inline iterator end() const {
		return{ m_end, nullptr };
	}

private:
	CConsoleLogger(size_t max_line)
		: m_beg(0)
		, m_end(0)
		, m_max_line(max_line)
	{
		if (m_max_line < 1)
			m_max_line = 1;
		m_log_buf.reserve(m_max_line);
	}
	constexpr static size_t TEXT_BUFFER_SIZE = 1024;
	char m_buf[TEXT_BUFFER_SIZE];
	std::vector<std::string> m_log_buf;
	size_t m_beg, m_end, m_max_line;
};

