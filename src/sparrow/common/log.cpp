#include <algorithm>
#include <cassert>
#include <chrono>
#include "log.h"

#ifdef WIN32
#include <windows.h>
#endif

#define TEXT_BUFF_SIZE 255


namespace wyc
{

const char* s_log_lvl_tag[wyc::LOG_LEVEL_COUNT] = {
	"DEBUG",		
	"INFO",		
	"WARNING",	
	"ERROR",
	"FATAL",
};

class CLoggerImpl : CLogger
{
public:
	CLoggerImpl(ELogLevel lvl);
	virtual ~CLoggerImpl();
	virtual void write(const char* buf, size_t size);
	virtual void flush() {}
	virtual ELogLevel get_level() const {
		return m_level;
	}
	virtual void set_level(ELogLevel lv) {
		m_level = lv;
	}
	void format_write(ELogLevel lvl, const char *fmt, ...);
private:
	ELogLevel m_level;
	char *m_buff;
};

bool CLogger::init(ELogLevel lvl)
{
	if (s_instance) {
		delete s_instance;
	}
	s_instance = new CLogger(lvl);
	return true;
}

CLoggerImpl::CLoggerImpl(ELogLevel lvl)
{
	m_level = lvl;
	m_buff = new char[TEXT_BUFF_SIZE+2];
}

CLoggerImpl::~CLoggerImpl()
{
	delete[] m_buff;
	m_buff = 0;
}

void CLoggerImpl::write(const char* buf, size_t size)
{
	printf(buf);
}

void CLoggerImpl::format_write(ELogLevel lvl, const char * fmt, ...)
{
	if (m_level < lvl)
		return;
	auto now = std::chrono::system_clock::now();
	auto buf = m_buff;
	int cnt = 0;
	cnt += ::sprintf_s(buf, TEXT_BUFF_SIZE, "[%s] ", s_log_lvl_tag[lvl]);
	va_list args;
	va_start(args, fmt);
	cnt += ::vsprintf_s(buf + cnt, TEXT_BUFF_SIZE - cnt, fmt, args);
	va_end(args);
	buf[cnt++] = '\n';
	buf[cnt] = 0;
	write(buf, cnt);
}

//-----------------------------------------------------
#define DEFAULT_ROTATE_SIZE (4*1024*1024)

bool CFileLogger::init(ELogLevel lvl, const char *log_name, const char* save_path, size_t rotate_size)
{
	if (s_instance) {
		delete s_instance;
	}
	s_instance = new CFileLogger(lvl, log_name, save_path, rotate_size);
	return true;
}

CFileLogger::CFileLogger(ELogLevel lvl, const char* log_name, const char* save_path, size_t rotate_size)
	: CLogger(lvl)
{
	m_hfile = 0;
	m_cur_size = 0;
	m_rotate_size = DEFAULT_ROTATE_SIZE;
	m_rotate_cnt = 0;
	create(log_name, save_path, rotate_size);
}

CFileLogger::~CFileLogger()
{
	if (m_hfile)
	{
		fclose(m_hfile);
		m_hfile = 0;
	}
}

bool CFileLogger::create(const char* log_name, const char* save_path, size_t rotate_size)
{
	if (!save_path || 0 == save_path[0])
		m_path = ".\\logs\\";
	else {
		m_path = save_path;
		std::replace(m_path.begin(), m_path.end(), '/', '\\');
		if (m_path[m_path.size() - 1] != '\\')
			m_path += "\\";
	}
	m_logname = log_name;
	m_curfile = m_path;
	m_curfile += m_logname;
	m_curfile += ".log";
	if (m_hfile) {
		fclose(m_hfile);
		m_hfile = 0;
	}
	m_hfile = _fsopen(m_curfile.c_str(), "w", _SH_DENYWR);
	if (!m_hfile) {
		// build the directory tree
		std::string cmd = "mkdir ";
		cmd += m_path;
		if (std::system(cmd.c_str()))
			return false;
		// try again!
		m_hfile = _fsopen(m_curfile.c_str(), "w", _SH_DENYWR);
		if (!m_hfile)
			return false;
	}
	m_cur_size = 0;
	if (!rotate_size)
		m_rotate_size = DEFAULT_ROTATE_SIZE;
	else
		m_rotate_size = rotate_size;
	m_rotate_cnt = 0;
	return true;
}

void CFileLogger::rotate()
{
	// rotate
	fclose(m_hfile);
	m_hfile = 0;
	m_rotate_cnt += 1;
	std::string bak_file = m_path;
	bak_file += m_logname;
	bak_file += std::to_string(m_rotate_cnt);
	bak_file += ".log";
	if (std::rename(m_curfile.c_str(), bak_file.c_str())) {
		// failed to rename	
		std::remove(bak_file.c_str());
		// try again
		if (std::rename(m_curfile.c_str(), bak_file.c_str()))
		{
			// failed
		}
	}
	m_cur_size = 0;
	m_hfile = _fsopen(m_curfile.c_str(), "w", _SH_DENYWR);
	if (!m_hfile)
		return;

}

void CFileLogger::write(const char* record, size_t size)
{
	if (!m_hfile)
		return;
	if (m_cur_size + size >= m_rotate_size)
		rotate();
	m_cur_size += size;
	fprintf(m_hfile, record);
}

void CFileLogger::flush()
{
	if (m_hfile)
		fflush(m_hfile);
}

//-----------------------------------------------------

bool CDebugLogger::init(ELogLevel lvl)
{
	if (s_instance) {
		delete s_instance;
	}
	s_instance = new CDebugLogger(lvl);
	return true;
}

CDebugLogger::CDebugLogger(ELogLevel lvl)
	: CLogger(lvl)
	, m_is_debug_mode(false)
{
#ifdef WIN32
	m_is_debug_mode = IsDebuggerPresent() ? true : false;
#endif
}

void CDebugLogger::write(const char* record, size_t size)
{
#ifdef WIN32
	if (m_is_debug_mode)
		OutputDebugStringA(record);
	else
		printf(record);
#else
	printf(record);
#endif
}


}; // end of namesapce wyc
