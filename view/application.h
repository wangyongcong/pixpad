#pragma once

namespace wyc
{
	class application
	{
	public:
		static application* get_instance();

		virtual ~application() {}

		virtual void start() = 0;
		virtual void close() = 0;
		virtual void resize(unsigned view_w, unsigned view_h) = 0;
		virtual void on_event(void *ev) = 0;
		virtual bool on_command(int cmd_id, int cmd_event) = 0;

		virtual const std::wstring& name() const = 0;
		virtual void get_window_size(size_t &width, size_t &height) const = 0;
	};

} // endof namespace wyc