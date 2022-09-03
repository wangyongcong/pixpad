#define _CRT_SECURE_NO_WARNINGS
#include <unordered_map>
#include <string>
#include <sstream>
#include <iomanip>
#include "imgui.h"
#define STB_LOG_IMPLEMENTATION
#include "stb/stb_log.h"
#include "utility.h"
#include "shellcmd.h"

#define IMLOG_MIN_BUFFER_SIZE 512

const char *IMLOG_PROMPT = "# ";
constexpr unsigned IMLOG_PROMPT_SIZE = 2;


struct ImLogEntry {
	ImLogEntry(int v1) : level(v1), str(nullptr), size(0)
	{
	}
	
	int level;
	const char *str;
	size_t size;
};


class CImLog : public CCustomLog
{
public:
	CImLog() : m_cur_buf(nullptr) {
	}
	
	virtual ~CImLog() {
		while(m_cur_buf) {
			BufferHead *head = m_cur_buf;
			m_cur_buf = m_cur_buf->_next;
			delete [] head;
		}
	}
	
	virtual void process_event(const LogData *log) override
	{
		m_log_data.emplace_back(log->level);
		CCustomLog::process_event(log);
	}
	
	typedef std::vector<ImLogEntry>::const_iterator iterator;
	
	inline iterator begin() const {
		return m_log_data.begin();
	}
	
	inline iterator end() const {
		return m_log_data.end();
	}
	
protected:
	virtual std::pair<char*, size_t> getstr(size_t required_size) override
	{
		if(!m_cur_buf || m_cur_buf->buf + required_size > m_cur_buf->end)
		{
			unsigned size = (unsigned)required_size + sizeof(BufferHead);
			size = wyc::minimal_power2(size);
			if (size < IMLOG_MIN_BUFFER_SIZE)
				size = IMLOG_MIN_BUFFER_SIZE;
			char *new_buf = new char[size];
			BufferHead *head = (BufferHead*)new_buf;
			head->buf = new_buf + sizeof(BufferHead);
			head->end = new_buf + size;
			if(m_cur_buf)
				head->_next = m_cur_buf;
			else
				head->_next = nullptr;
			
			m_cur_buf = head;
		}
		return {m_cur_buf->buf, m_cur_buf->end - m_cur_buf->buf};
	}
	
	virtual void setstr(size_t size) override {
		if(!m_cur_buf || m_cur_buf->buf + size > m_cur_buf->end) {
			assert(0);
			return;
		}
		size_t len = size - 1;
		if(m_cur_buf->buf[len] != 0) {
			assert(0);
			m_cur_buf->buf[len] = 0;
		}
		if(!m_log_data.empty()) {
			auto &last = m_log_data.back();
			last.str = m_cur_buf->buf;
			last.size = len;
			m_cur_buf->buf += size;
		}
	}

private:
	struct BufferHead {
		char *buf;
		char *end;
		BufferHead *_next;
	};
	
	BufferHead *m_cur_buf;
	std::vector<ImLogEntry> m_log_data;
};


class CImConsole
{
public:
	static inline CImConsole& singleton()
	{
		static CImConsole s_console;
		return s_console;
	}

	bool draw()
	{
		ImGuiIO &io = ImGui::GetIO();
		if(io.KeyCtrl && ImGui::IsKeyReleased(int('`')))
		{
			m_is_show = !m_is_show;
			ImGui::SetNextWindowCollapsed(!m_is_show);
		}
		
		if (!ImGui::Begin("console", 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
		{
			m_is_show = false;
			ImGui::End();
			return false;
		}

		m_is_show = true;
		const auto &style = ImGui::GetStyle();
		float progress_bar_height = 4;
		ImGui::SetCursorPosX(style.WindowPadding.x * 0.5f);
		ImGui::ProgressBar(0, ImVec2(ImGui::GetWindowWidth() - style.WindowPadding.x, progress_bar_height), "");
		ImGui::Separator();

		// BEGIN log
		ImGui::BeginChild("console_log", ImVec2(0, -ImGui::GetFrameHeightWithSpacing() - progress_bar_height - style.ItemSpacing.y), 
			false, ImGuiWindowFlags_HorizontalScrollbar);
		if(m_log) {
			m_log->process();
			for(auto &s : *m_log) {
				draw_log(s);
			}
		}
		ImGui::EndChild(); // END log

		// BEGIN input
		ImGui::Separator();
		ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth());
		int input_flag = 0;
		if(!io.KeyCtrl)
			input_flag = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory;
		if (ImGui::InputText("", m_input_beg, m_input_max, input_flag,
			[] (ImGuiTextEditCallbackData *ctx) -> int {
			((CImConsole*)ctx)->on_input_end();
			return 0;
		}, (void*)this))
		{
			if (m_input_beg[0]) {
				log_info("%s", std::string(m_input_buf));
				process_input();
			}
			m_input_beg[0] = 0;
		}
		ImGui::PopItemWidth();

		 // Demonstrate keeping auto focus on the input box
		if (ImGui::IsItemHovered() || (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && !ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0)))
			ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget
		// END input

		ImGui::End(); // END console
		return true;
	}

	void on_input_end()
	{
	}

	void process_input()
	{
		std::string cmd_name = strtok(m_input_beg, " ");
		if (cmd_name == "help")
		{
			if(m_commands.empty()) {
				log_info("No commands available.");
				return;
			}
			int max_len = 4;
			for (auto &it : m_commands)
			{
				int s = (int)it.first.size();
				if (s > max_len)
					max_len = s;
			}
			// tab size 4
			int c = (max_len + 4) & ~3;
			std::stringstream ss;
			ss << std::left << std::setfill(' ');
			ss << "- " << std::setw(c) << "help" << "show help message" << std::setw(0);
			log_info(ss.str().c_str());
			for (auto &it : m_commands) {
				ss.str("");
				ss << "- " << std::setw(c) << it.first << it.second->description() << std::setw(0);
				log_info(ss.str().c_str());
			}
			return;
		}
		auto iter = m_commands.find(cmd_name);
		if (iter == m_commands.end()) {
			log_error("Unkonwn command");
			return;
		}
		iter->second->execute(m_input_beg + cmd_name.size() + 1);
	}

	void draw_log(const ImLogEntry &log)
	{
		static ImVec4 COLOR_PROMPT = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
		static ImVec4 COLOR_ERROR = ImVec4(1.0f, 0.4f, 0.4f, 1.0f);
		static ImVec4 COLOR_WARNING = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
		
		ImVec4 *color = nullptr;
		if(strncmp(log.str, IMLOG_PROMPT, IMLOG_PROMPT_SIZE) == 0) {
			// prompt
			color = &COLOR_PROMPT;
		}
		else { // log level
			switch(log.level)
			{
				case LOG_ERROR:
					color = &COLOR_ERROR;
					break;
				case LOG_WARNING:
					color = &COLOR_WARNING;
					break;
				default:
					ImGui::TextUnformatted(log.str);
					return;
			}
		}
		ImGui::PushStyleColor(ImGuiCol_Text, *color);
		ImGui::TextUnformatted(log.str);
		ImGui::PopStyleColor();
	}

	bool add_command(wyc::IShellCommand *cmd)
	{
		if (!cmd)
			return false;
		auto ret = m_commands.emplace(std::make_pair(cmd->name(), cmd));
		if (!ret.second)
			return false;
		return true;
	}

	inline void del_command(const std::string &cmd_name)
	{
		m_commands.erase(cmd_name);
	}

private:
	static constexpr size_t INPUT_BUFF_SIZE = 256;
	char m_input_buf[INPUT_BUFF_SIZE];
	char *m_input_beg;
	size_t m_input_max;
	std::unordered_map<std::string, wyc::IShellCommand*> m_commands;
	CImLog *m_log;
	bool m_is_show;

	CImConsole()
	{
		strncpy(m_input_buf, IMLOG_PROMPT, INPUT_BUFF_SIZE);
		m_input_beg = m_input_buf + IMLOG_PROMPT_SIZE;
		m_input_max = INPUT_BUFF_SIZE - IMLOG_PROMPT_SIZE;
		m_log = new CImLog;
		add_log_handler(m_log);
		m_is_show = false;
	}
	
	~CImConsole()
	{
		if(m_log)
			delete m_log;
	}
};


bool console_register_command(wyc::IShellCommand *cmd)
{
	auto &console = CImConsole::singleton();
	return console.add_command(cmd);
}

void console_unregister_command(const std::string &cmd_name)
{
	auto &console = CImConsole::singleton();
	console.del_command(cmd_name);
}

bool show_console()
{
	auto &console = CImConsole::singleton();
	ImGui::SetNextWindowPos({0, 0}, ImGuiCond_Once);
	ImGui::SetNextWindowSize({300, 600}, ImGuiCond_Once);
	return console.draw();
}

