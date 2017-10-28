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
extern const char* s_log_tags[LOG_LEVEL_COUNT];
#define LOG_TAG(i) wyc::s_log_tags[i]

class ILogger
{
public:
	virtual void write(ELogLevel lvl, const char *fmt, ...) = 0;
	virtual ELogLevel get_level() const = 0;
	virtual void set_level(ELogLevel lv) = 0;
};
extern ILogger* g_logger;

#define LOGGER_INIT \
wyc::ILogger* wyc::g_logger = nullptr; \
const char* wyc::s_log_lvl_tag[wyc::LOG_LEVEL_COUNT] = {\
	"DEBUG",\
	"INFO",\
	"WARNING",\
	"ERROR",\
	"FATAL",\
};
#define LOGGER_GET(T) (dynamic_cast<T*>(wyc::g_logger))
#define LOGGER_SETUP(v) {if(wyc::g_logger) delete wyc::g_logger; wyc::g_logger = v;}
#define LOGGER_CHECK() (wyc::g_logger != nullptr)
#define LOGGER_WRITE (wyc::g_logger->write)

#ifdef NDEBUG
#define log_debug(fmt,...)
#define log_info(fmt,...) 
#define log_warn(fmt,...) 
#define log_error(fmt,...)
#define log_fatal(fmt,...)
#else
#define log_debug(fmt,...) (LOGGER_WRITE(wyc::LOG_DEBUG, fmt, __VA_ARGS__))
#define log_info(fmt,...)  (LOGGER_WRITE(wyc::LOG_INFO, fmt, __VA_ARGS__))
#define log_warn(fmt,...)  (LOGGER_WRITE(wyc::LOG_WARN, fmt, __VA_ARGS__))
#define log_error(fmt,...) (LOGGER_WRITE(wyc::LOG_ERROR, fmt, __VA_ARGS__))
#define log_fatal(fmt,...) (LOGGER_WRITE(wyc::LOG_FATAL, fmt, __VA_ARGS__))
#endif

// log to std output
class CLogger
{
public:
	static bool init(ELogLevel lvl);
};

// log to file
class CFileLogger
{
public:
	static bool init(ELogLevel lvl, const char* log_name, const char* save_path = 0, size_t rotate_size = 0);
};

// log to IDE debug window
class CDebugLogger
{
public:
	static bool init(ELogLevel lvl);
};

}; // end of namespace wyc

#endif // WYC_HEADER_LOGGER
