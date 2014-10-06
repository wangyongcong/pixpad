#include <algorithm>
#include <ctime>
#include <cassert>
#include <sys/timeb.h>
#include "logger.h"

namespace wyc
{

const char* s_log_lvl_tag[wyc::LOG_LEVEL_COUNT] = {
	"DEBUG",		
	"INFO",		
	"WARN",	
	"ERROR",
	"FATAL",
};

xlogger::xlogger()
{
	m_hfile = NULL;
	m_cur_size = 0;
	m_rotate_size = DEFAULT_ROTATE_SIZE;
	m_rotate_cnt = 0;
	m_level = LOG_DEBUG;
}

xlogger::~xlogger()
{
	if (m_hfile) {
		fclose(m_hfile);
		m_hfile = 0;
	}
}

bool xlogger::create(const char* log_name, const char* save_path, size_t rotate_size, LOG_LEVEL lvl)
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
	m_level = lvl;
	return true;
}

void xlogger::_write(LOG_LEVEL lvl, const char* record, size_t size)
{
	assert(m_hfile);
	timeb rawtime;	
	ftime(&rawtime);
	rawtime.time = std::time(NULL);
	tm *t = std::localtime(&rawtime.time);
	size += 35; // timestamp(25) and level tag name(7)
	if (m_cur_size + size >= m_rotate_size)
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
	m_cur_size += size;
	fprintf(m_hfile, "[%04d-%02d-%02d %02d:%02d:%02d.%03d] [%s] %s\n",
		t->tm_year+1900, t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec, rawtime.millitm,
		s_log_lvl_tag[lvl], record);
}

void xlogger::_write(LOG_LEVEL lvl, const char *fmt, va_list args)
{
	assert(m_hfile);
	char buff[256];
	int sz = 255;
	int cnt = ::vsprintf_s(buff, sz, fmt, args);
	if (cnt < 0) 
		return;
	assert(cnt <= sz);
	buff[cnt] = 0;
	_write(lvl, buff, cnt);
}

}; // end of namesapce wyc
