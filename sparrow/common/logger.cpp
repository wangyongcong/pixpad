#include <algorithm>
#include <cassert>
#include <chrono>
#include "logger.h"

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

CLogger::CLogger(ELogLevel lvl)
{
	m_level = lvl;
	m_buff = new char[TEXT_BUFF_SIZE+2];
}

CLogger::~CLogger()
{
	delete[] m_buff;
	m_buff = 0;
}

void CLogger::format_write(ELogLevel lvl, const char *fmt, va_list args)
{
	auto now = std::chrono::system_clock::now();
	int cnt = 0;
	cnt += ::sprintf_s(m_buff, TEXT_BUFF_SIZE, "[%s] ", s_log_lvl_tag[lvl]);
	cnt += ::vsprintf_s(m_buff + cnt, TEXT_BUFF_SIZE - cnt, fmt, args);
	m_buff[cnt++] = '\n';
	m_buff[cnt] = 0;
	write(m_buff, cnt);
}


CFileLogger::CFileLogger(const char* log_name, const char* save_path, size_t rotate_size, ELogLevel lvl) : CLogger(lvl)
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

void CDebugLogger::write(const char * record, size_t size)
{
#ifdef WIN32
	OutputDebugStringA(record);
#else
	printf(record);
#endif
}

}; // end of namesapce wyc
