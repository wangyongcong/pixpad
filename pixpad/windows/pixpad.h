#include "win_app.h"
#include <tuple>
#include <algorithm>
#include "glrender.h"
#include "glpipeline.h"
#include "swpipeline.h"

namespace wyc
{

	enum RENDER_MODE {
		SOFTWARE_RENDERER = 0,
		OPENGL_RENDERER,

		RENDERER_COUNT
	};

	enum MODEL_ID {
		REGULAR_TRIANGLE = 0,
		SQUARE_PLANE,

		MODEL_COUNT,
	};

	class xapp_pixpad : public xwin_app
	{
	public:
		xapp_pixpad();
		virtual ~xapp_pixpad();
		virtual void on_start();
		virtual void on_close();
		virtual void on_render();
		virtual bool on_command(int cmd_id, int cmd_event);
		virtual void on_resize(unsigned width, unsigned height);
		virtual void on_mouse_button_down(MOUSE_BUTTON button, int x, int y);
		virtual void on_mouse_button_up(MOUSE_BUTTON button, int x, int y);
		virtual void on_mouse_move(int x, int y);
		virtual void on_key_down(int keycode);
		void set_render_mode(RENDER_MODE mode);

		// renderer
		wyc::xpipeline *pipelines[RENDERER_COUNT];
		RENDER_MODE render_mode = OPENGL_RENDERER;
		// models
		wyc::xmat4f_t transform;
		std::pair<wyc::xvertex_buffer*, wyc::xindex_buffer*> models[MODEL_COUNT];
		MODEL_ID model_id = REGULAR_TRIANGLE;
		std::string material;
		// app state
		bool is_drag_mode = false;
		std::pair<int, int> drag_start_pos;
		wyc::xvec4f_t rotate;
	};

}; // end of namespace wyc