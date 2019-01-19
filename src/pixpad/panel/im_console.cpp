#define _CRT_SECURE_NO_WARNINGS
#include <unordered_map>
#include <string>
#include <sstream>
#include <iomanip>
#include "imgui.h"
#define STB_LOG_IMPLEMENTATION
#include "stb_log.h"
#include "util.h"
#include "shellcmd.h"


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
	
	typedef std::vector<std::pair<const char*, size_t>>::const_iterator iterator;
	
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
			size_t size = required_size + sizeof(BufferHead);
			size = wyc::next_power2(size);
			char *new_buf = new char[size];
			BufferHead *head = (BufferHead*)new_buf;
			head->buf = new_buf;
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
		m_log_data.emplace_back(m_cur_buf->buf, len);
		m_cur_buf->buf += size;
	}

private:
	struct BufferHead {
		char *buf;
		char *end;
		BufferHead *_next;
	};
	BufferHead *m_cur_buf;
	std::vector<std::pair<const char*, size_t>> m_log_data;
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
		ImGui::SetNextWindowSize(ImVec2(float(m_width), float(m_height)), ImGuiCond_Always);
		if (!ImGui::Begin("console", 0, m_flags))
		{
			ImGui::End();
			return false;
		}

		const auto &style = ImGui::GetStyle();
		float progress_bar_height = 4;
		ImGui::SetCursorPosX(style.WindowPadding.x * 0.5f);
		ImGui::ProgressBar(0, ImVec2(ImGui::GetWindowWidth() - style.WindowPadding.x, progress_bar_height), "");
		ImGui::Separator();

		// BEGIN log
		ImGui::BeginChild("console_log", ImVec2(0, -ImGui::GetItemsLineHeightWithSpacing() - progress_bar_height - style.ItemSpacing.y), 
			false, ImGuiWindowFlags_HorizontalScrollbar);
		if(m_log) {
			m_log->process();
			for(auto s : *m_log) {
				draw_log(s.first);
			}
		}
		ImGui::EndChild(); // END log

		// BEGIN input
		ImGui::Separator();
		ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth());
		if (ImGui::InputText("", m_input_beg, m_input_max, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory,
			[] (ImGuiTextEditCallbackData *ctx) -> int {
			((CImConsole*)ctx)->on_input_end();
			return 0;
		}, (void*)this))
		{
			if (m_input_beg[0]) {
				log_info(std::string(m_input_buf));
				process_input();
			}
			m_input_beg[0] = 0;
		}
		ImGui::PopItemWidth();

		 // Demonstrate keeping auto focus on the input box
		if (ImGui::IsItemHovered() || (ImGui::IsRootWindowOrAnyChildFocused() && !ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0)))
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
			int max_len = 4;
			for (auto &it : m_commands)
			{
				int s = it.first.size();
				if (s > max_len)
					max_len = s;
			}
			// tab size 4
			int c = (max_len + 4) & ~3;
			std::stringstream ss;
			ss << std::left << std::setfill(' ');
			ss << "- " << std::setw(c) << "help" << "show help message" << std::setw(0);
			log_info(ss.str().c_str());
			ss.str("");
			for (auto &it : m_commands) {
				ss << "- " << std::setw(c) << it.first << it.second->description() << std::setw(0);
				log_info(ss.str().c_str());
				ss.str("");
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

	void draw_log(const std::string &str)
	{
		static const std::pair<std::string, ImVec4> s_tag_color[] = {
			std::make_pair(std::string("# "), ImVec4(0.0f, 1.0f, 0.0f, 1.0f)),
			std::make_pair(std::string("[ERROR]") , ImVec4(1.0f, 0.4f, 0.4f, 1.0f)),
			std::make_pair(std::string("[WARNING]"), ImVec4(1.0f, 1.0f, 0.0f, 1.0f)),
		};
		for (auto &tag : s_tag_color) {
			if (!str.compare(0, tag.first.size(), tag.first)) {
				ImGui::PushStyleColor(ImGuiCol_Text, tag.second);
				ImGui::TextUnformatted(str.c_str());
				ImGui::PopStyleColor();
				return;
			}
		}
		ImGui::TextUnformatted(str.c_str());
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
	int m_width, m_height;
	ImGuiWindowFlags m_flags;
	static constexpr size_t INPUT_BUFF_SIZE = 256;
	char m_input_buf[INPUT_BUFF_SIZE];
	char *m_input_beg;
	size_t m_input_max;
	std::unordered_map<std::string, wyc::IShellCommand*> m_commands;
	CImLog *m_log;

	CImConsole()
	{
//		m_width = int(AppConfig::window_width * 0.382f), m_height = int(AppConfig::window_height);
		m_width = 200;
		m_height = 400;
		m_flags = ImGuiWindowFlags_NoResize
			| ImGuiWindowFlags_NoMove;
		const char *prompt = "# ";
		size_t prompt_len = strlen(prompt);
		strncpy(m_input_buf, prompt, INPUT_BUFF_SIZE);
		m_input_beg = m_input_buf + prompt_len;
		m_input_max = INPUT_BUFF_SIZE - prompt_len;
		m_log = new CImLog;
		add_log_handler(m_log);
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
	return console.draw();
}

