#include <algorithm>
#include <cassert>
#include <chrono>
#define LOGGER_IMPLEMENTATION
#include "log.h"

#ifdef WIN32
#include <windows.h>
#endif

#define TEXT_BUFF_SIZE 255  // internal string buf size
#define DEFAULT_ROTATE_SIZE (4*1024*1024)  // file logger rotate size

namespace wyc
{

class CLoggerImpl : public ILogger
{
public:
	CLoggerImpl();
	virtual ~CLoggerImpl();
	virtual void write(ELogLevel lvl, const char *fmt, ...) override;
	virtual void output(const char* buf, size_t size);
private:
	char *m_buf;
};

CLoggerImpl::CLoggerImpl()
{
	m_buf = new char[TEXT_BUFF_SIZE+2];
}

CLoggerImpl::~CLoggerImpl()
{
	delete[] m_buf;
	m_buf = 0;
}

void CLoggerImpl::write(ELogLevel lvl, const char * fmt, ...)
{
	//auto now = std::chrono::system_clock::now();
	int cnt = std::snprintf(m_buf, TEXT_BUFF_SIZE, "[%s] ", LOG_TAG(lvl));
	va_list args;
	va_start(args, fmt);
	cnt += std::vsnprintf(m_buf + cnt, TEXT_BUFF_SIZE - cnt, fmt, args);
	va_end(args);
	m_buf[cnt++] = '\n';
	m_buf[cnt] = 0;
	output(m_buf, cnt);
}

void CLoggerImpl::output(const char* buf, size_t size)
{
	printf(buf);
}

//-----------------------------------------------------

class CFileLoggerImpl : public CLoggerImpl
{
public:
	CFileLoggerImpl(const char* log_name, const char* save_path = 0, size_t rotate_size = 0);
	virtual ~CFileLoggerImpl();
	virtual void output(const char* buf, size_t size);

private:
	FILE *m_hfile;
	std::string m_path;
	std::string m_logname;
	std::string m_curfile;
	size_t m_cur_size;
	size_t m_rotate_size;
	unsigned m_rotate_cnt;

	bool create(const char* log_name, const char* save_path = 0, size_t rotate_size = 0);
	void rotate();
};

CFileLoggerImpl::CFileLoggerImpl(const char* log_name, const char* save_path, size_t rotate_size)
	: CLoggerImpl()
	, m_hfile(0)
	, m_cur_size(0)
	, m_rotate_size(DEFAULT_ROTATE_SIZE)
	, m_rotate_cnt(0)
{
	create(log_name, save_path, rotate_size);
}

CFileLoggerImpl::~CFileLoggerImpl()
{
	if (m_hfile)
	{
		fflush(m_hfile);
		fclose(m_hfile);
		m_hfile = 0;
	}
}

bool CFileLoggerImpl::create(const char* log_name, const char* save_path, size_t rotate_size)
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

void CFileLoggerImpl::rotate()
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

void CFileLoggerImpl::output(const char* buf, size_t size)
{
	if (!m_hfile)
		return;
	if (m_cur_size + size >= m_rotate_size)
		rotate();
	m_cur_size += size;
	fprintf(m_hfile, buf);
}

//-----------------------------------------------------

class CDebugLoggerImpl : public CLoggerImpl
{
public:
	CDebugLoggerImpl();
	virtual void output(const char* buf, size_t size) override;

private:
	bool m_is_debug_mode;
};

CDebugLoggerImpl::CDebugLoggerImpl()
	: CLoggerImpl()
	, m_is_debug_mode(false)
{
#ifdef WIN32
	m_is_debug_mode = IsDebuggerPresent() ? true : false;
#endif
}

void CDebugLoggerImpl::output(const char* buf, size_t size)
{
#ifdef WIN32
	if (m_is_debug_mode)
		OutputDebugStringA(buf);
	else
		printf(buf);
#else
	printf(buf);
#endif
}

//-----------------------------------------------------

bool init_std_log()
{
	auto ptr = new CLoggerImpl();
	LOGGER_SET(ptr);
	return true;
}

bool init_file_log(const char *log_name, const char* save_path, size_t rotate_size)
{
	auto ptr = new CFileLoggerImpl(log_name, save_path, rotate_size);
	LOGGER_SET(ptr);
	return true;
}

bool init_debug_log()
{
	auto ptr = new CDebugLoggerImpl();
	LOGGER_SET(ptr);
	return true;
}

}; // end of namesapce wyc
