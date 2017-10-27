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

// macro for logging
#define log_debug(fmt,...) (wyc::CLogger::s_instance->format_write(LOG_DEBUG, fmt,__VA_ARGS__))
#define log_info(fmt,...) (wyc::CLogger::s_instance->format_write(LOG_INFO, fmt,__VA_ARGS__))
#define log_warn(fmt,...) (wyc::CLogger::s_instance->format_write(LOG_WARN, fmt,__VA_ARGS__))
#define log_error(fmt,...) (wyc::CLogger::s_instance->format_write(LOG_ERROR, fmt,__VA_ARGS__))
#define log_fatal(fmt,...) (wyc::CLogger::s_instance->format_write(LOG_FATAL, fmt,__VA_ARGS__))

class CLogger
{
public:
	static CLogger* s_instance;
	//static bool init(ELogLevel lvl);
	void format_write(ELogLevel lvl, const char *fmt, ...);
	virtual void write(const char* buf, size_t size) = 0;
	virtual void flush() = 0;
	virtual ELogLevel get_level() const = 0;
	virtual void set_level(ELogLevel lv) = 0;
};

class CFileLogger : public CLogger
{
public:
	static bool init(ELogLevel lvl, const char* log_name, const char* save_path = 0, size_t rotate_size = 0);
	virtual ~CFileLogger();
	virtual void write(const char* record, size_t size);
	virtual void flush();

private:
	FILE *m_hfile;
	std::string m_path;
	std::string m_logname;
	std::string m_curfile;
	size_t m_cur_size;
	size_t m_rotate_size;
	unsigned m_rotate_cnt;

	CFileLogger(ELogLevel lvl, const char* log_name, const char* save_path = 0, size_t rotate_size = 0);
	bool create(const char* log_name, const char* save_path = 0, size_t rotate_size = 0);
	void rotate();
};

class CDebugLogger : public CLogger
{
public:
	static bool init(ELogLevel lvl);
	virtual void write(const char* record, size_t size);
private:
	bool m_is_debug_mode;
	CDebugLogger(ELogLevel lvl);
};

}; // end of namespace wyc

#endif // WYC_HEADER_LOGGER
