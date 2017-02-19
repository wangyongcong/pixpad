#ifndef WYC_HEADER_LOGGER
#define WYC_HEADER_LOGGER

#include <cstdint>
#include <string>
#include <cstdarg>

namespace wyc
{

enum ELogLevel
{
	LOG_DEBUG,
	LOG_INFO,
	LOG_WARN,
	LOG_ERROR,
	LOG_FATAL,

	LOG_LEVEL_COUNT,
};

void init_debug_log();

void init_file_log(const char* log_name);

// macro for logging
#define log_debug(fmt,...) (wyc::g_log->debug(fmt,__VA_ARGS__))
#define log_info(fmt,...) (wyc::g_log->info(fmt,__VA_ARGS__))
#define log_warn(fmt,...) (wyc::g_log->warn(fmt,__VA_ARGS__))
#define log_error(fmt,...) (wyc::g_log->error(fmt,__VA_ARGS__))
#define log_fatal(fmt,...) (wyc::g_log->fatal(fmt,__VA_ARGS__))

#define DEFAULT_ROTATE_SIZE (4*1024*1024)

class CLogger
{
public:
	CLogger(ELogLevel lvl = LOG_DEBUG);
	virtual ~CLogger();
	virtual void write(const char* record, size_t size)
	{
		printf(record);
	}
	virtual void flush() {}

	void debug(const char *fmt, ...);
	void info(const char *fmt, ...);
	void warn(const char *fmt, ...);
	void error(const char *fmt, ...);
	void fatal(const char *fmt, ...);

	ELogLevel get_level() const
	{
		return m_level;
	}
	void set_level(ELogLevel lv)
	{
		m_level = lv;
	}

protected:
	ELogLevel m_level;
	char *m_buff;

	void format_write(ELogLevel lvl, const char *fmt, va_list args);
};

inline void CLogger::debug (const char* fmt, ...) {
	if (m_level <= LOG_DEBUG) {
		va_list args;
		va_start(args, fmt);
		format_write(LOG_DEBUG, fmt, args);
		va_end(args);
	}
}

inline void CLogger::info(const char* fmt, ...) {
	if (m_level <= LOG_INFO) {
		va_list args;
		va_start(args, fmt);
		format_write(LOG_INFO, fmt, args);
		va_end(args);
	}
}

inline void CLogger::warn(const char* fmt, ...) {
	if (m_level <= LOG_WARN) {
		va_list args;
		va_start(args, fmt);
		format_write(LOG_WARN, fmt, args);
		va_end(args);
	}
}

inline void CLogger::error(const char* fmt, ...) {
	if (m_level <= LOG_ERROR) {
		va_list args;
		va_start(args, fmt);
		format_write(LOG_ERROR, fmt, args);
		va_end(args);
	}
}

inline void CLogger::fatal(const char* fmt, ...) {
	if (m_level <= LOG_FATAL) {
		va_list args;
		va_start(args, fmt);
		format_write(LOG_FATAL, fmt, args);
		va_end(args);
	}
}

// global logger
extern CLogger *g_log;

}; // end of namespace wyc

#endif // WYC_HEADER_LOGGER
