#define _CRT_SECURE_NO_WARNINGS
#include <algorithm>
#include <cassert>
#include <chrono>
#define WYC_LOG_IMPLEMENTATION
#include "log.h"
#if (defined _WIN32 || defined _WIN64)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#endif 

#define LOG_MAX_TEXT_BUFF_SIZE 1024*1024  // internal string buf size
#define LOG_DEFAULT_ROTATE_SIZE (4*1024*1024)  // file logger rotate size

namespace wyc
{
	//-----------------------------------------------------
	// common logger implementation
	//-----------------------------------------------------
	class CLoggerImpl : public ILogger
	{
	public:
		CLoggerImpl();
		virtual ~CLoggerImpl();
		virtual void write(ELogLevel lvl, const char *fmt, ...) override;
		virtual void output(const char* buf, size_t size);
	private:
		char *m_buf;
		int m_buf_size;
	};

	CLoggerImpl::CLoggerImpl()
		: m_buf_size(256)
	{
		m_buf = new char[m_buf_size];
	}

	CLoggerImpl::~CLoggerImpl()
	{
		delete[] m_buf;
		m_buf = 0;
	}

	void CLoggerImpl::write(ELogLevel lvl, const char * fmt, ...)
	{
		//auto now = std::chrono::system_clock::now();
		// keep space for LF
		int sz = m_buf_size - 1;
		char *wbuf = m_buf;
		int cnt = std::snprintf(wbuf, sz, "[%s] ", LOG_TAG(lvl));
		if (cnt < 0 || cnt >= sz) {
			// log tag should not produce any error
			return;
		}
		sz -= cnt;
		wbuf += cnt;
		va_list args;
		va_start(args, fmt);
		cnt = std::vsnprintf(wbuf, sz, fmt, args);
		va_end(args);
		if (cnt < 0) {
			// encoding error
			std::strcpy(m_buf, "[ERROR] LOG encoding error");
			return;
		}
		if (cnt >= sz) {
			// size not enough
			cnt += m_buf_size - 1 - sz;
			sz = m_buf_size;
			while (sz <= cnt)
				sz <<= 1;
			if (sz > LOG_MAX_TEXT_BUFF_SIZE) {
				std::strcpy(m_buf, "[ERROR] LOG buffer overflow");
				return;
			}
			delete[] m_buf;
			m_buf = new char[sz];
			m_buf_size = sz;
			// write again
			sz -= 1;
			wbuf = m_buf;
			cnt = std::snprintf(wbuf, sz, "[%s] ", LOG_TAG(lvl));
			assert(cnt > 0 && cnt < sz);
			sz -= cnt;
			wbuf += cnt;
			va_start(args, fmt);
			cnt = std::vsnprintf(wbuf, sz, fmt, args);
			va_end(args);
			assert(cnt > 0 && cnt < sz);
		}
		wbuf[cnt++] = '\n';
		wbuf[cnt] = 0;
		output(m_buf, cnt);
	}

	void CLoggerImpl::output(const char* buf, size_t size)
	{
		printf(buf);
	}

	//-----------------------------------------------------
	// file logger implementation
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
		, m_rotate_size(LOG_DEFAULT_ROTATE_SIZE)
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
			m_rotate_size = LOG_DEFAULT_ROTATE_SIZE;
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
	// debug logger implementation
	//-----------------------------------------------------
	class CDebugLoggerImpl : public CLoggerImpl
	{
	public:
		CDebugLoggerImpl();
		virtual void output(const char* buf, size_t size) override;

	private:
		bool m_is_debug_mode;
	};

	// debug logger implementation
	CDebugLoggerImpl::CDebugLoggerImpl()
		: CLoggerImpl()
		, m_is_debug_mode(false)
	{
#if (defined _WIN32 || defined _WIN64)
		m_is_debug_mode = IsDebuggerPresent() ? true : false;
#endif
	}

	void CDebugLoggerImpl::output(const char* buf, size_t size)
	{
#if (defined _WIN32 || defined _WIN64)
		if (m_is_debug_mode)
			OutputDebugStringA(buf);
		else
			printf(buf);
#else
		printf(buf);
#endif
	}

	//-----------------------------------------------------
	// global logger initialization
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