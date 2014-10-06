#ifndef WYC_HEADER_LOGGER
#define WYC_HEADER_LOGGER

#include <cstdint>
#include <string>
#include <cstdarg>

namespace wyc
{

enum LOG_LEVEL
{
	LOG_DEBUG,
	LOG_INFO,
	LOG_WARN,
	LOG_ERROR,
	LOG_FATAL,

	LOG_LEVEL_COUNT,
};

#define DEFAULT_ROTATE_SIZE (4*1024*1024)

class xlogger
{
public:
	xlogger();
	~xlogger();
	bool create(const char* log_name, const char* save_path=0, size_t rotate_size = 0, LOG_LEVEL lvl = LOG_DEBUG);
	void flush();
	void debug(const char *fmt, ...);
	void info (const char *fmt, ...);
	void warn (const char *fmt, ...);
	void error(const char *fmt, ...);
	void fatal(const char *fmt, ...);
	LOG_LEVEL get_level() const;
	size_t get_rotation_size() const;
private:
	FILE *m_hfile;
	std::string m_path;
	std::string m_logname;
	std::string m_curfile;
	size_t m_cur_size;
	size_t m_rotate_size;
	unsigned m_rotate_cnt;
	LOG_LEVEL m_level;

	void _write(LOG_LEVEL lvl, const char* record, size_t size);
	void _write(LOG_LEVEL lvl, const char *fmt, va_list args);
};

inline void xlogger::debug (const char* fmt, ...) {
	if (m_hfile && m_level <= LOG_DEBUG) {
		va_list args;
		va_start(args, fmt);
		_write(LOG_DEBUG, fmt, args);
		va_end(args);
	}
}

inline void xlogger::info(const char* fmt, ...) {
	if (m_hfile && m_level <= LOG_INFO) {
		va_list args;
		va_start(args, fmt);
		_write(LOG_INFO, fmt, args);
		va_end(args);
	}
}

inline void xlogger::warn(const char* fmt, ...) {
	if (m_hfile && m_level <= LOG_WARN) {
		va_list args;
		va_start(args, fmt);
		_write(LOG_WARN, fmt, args);
		va_end(args);
	}
}

inline void xlogger::error(const char* fmt, ...) {
	if (m_hfile && m_level <= LOG_ERROR) {
		va_list args;
		va_start(args, fmt);
		_write(LOG_ERROR, fmt, args);
		va_end(args);
	}
}

inline void xlogger::fatal(const char* fmt, ...) {
	if (m_hfile && m_level <= LOG_FATAL) {
		va_list args;
		va_start(args, fmt);
		_write(LOG_FATAL, fmt, args);
		va_end(args);
	}
}

inline void xlogger::flush()
{
	if (m_hfile)
		fflush(m_hfile);
}

}; // end of namespace wyc

#endif // WYC_HEADER_LOGGER
