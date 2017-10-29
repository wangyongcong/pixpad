#pragma once
#include <vector>
#include <iterator>
#include <type_traits>
#include "log.h"

class CConsoleLogger : public wyc::ILogger
{
public:
	static void init() {
		auto ptr = new CConsoleLogger(256);
		LOGGER_SET(ptr);
	}

	virtual void write(wyc::ELogLevel lvl, const char *fmt, ...) override {
		int cnt = std::snprintf(m_buf, TEXT_BUFFER_SIZE, "[%s] ", LOG_TAG(lvl));
		va_list args;
		va_start(args, fmt);
		cnt += std::vsnprintf(m_buf + cnt, TEXT_BUFFER_SIZE - cnt, fmt, args);
		va_end(args);
		m_buf[cnt++] = '\n';
		m_buf[cnt] = 0;
		output(m_buf);
	}
	void output(const char *buf)
	{
		if (m_end < m_max_line) {
			m_log_buf.emplace_back(m_buf);
			m_end += 1;
		}
		else {
			m_log_buf[m_end % m_max_line] = m_buf;
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
	constexpr static size_t TEXT_BUFFER_SIZE = 256;
	char m_buf[TEXT_BUFFER_SIZE];
	std::vector<std::string> m_log_buf;
	size_t m_beg, m_end, m_max_line;
};

