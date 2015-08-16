#pragma once

namespace wyc
{
	class application
	{
	public:
		static inline application* get_instance();
		virtual void start() = 0;
		virtual void close() = 0;
		virtual void update() = 0;
		virtual void render() = 0;
		virtual void on_event(void *ev) = 0;
		virtual bool on_command(int cmd_id, int cmd_event) = 0;
	};

} // endof namespace wyc