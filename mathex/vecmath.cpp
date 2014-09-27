#include "vecmath.h"

namespace wyc
{
	void set_orthograph(xmat4f_t &proj, float xmin, float ymin, float zmin, float xmax, float ymax, float zmax)
	{
		proj.identity();
		proj.m00 = 2.0f / (xmax - xmin);
		proj.m11 = 2.0f / (ymax - ymin);
		proj.m22 = -2.0f / (zmax - zmin);
		proj.m03 = -(xmax + xmin) / (xmax - xmin);
		proj.m13 = -(ymax + ymin) / (ymax - ymin);
		proj.m23 = -(zmax + zmin) / (zmax - zmin);
	}

	void set_perspective(xmat4f_t &proj, float fov, float aspect, float fnear, float ffar)
	{
		if (fnear<0)
			fnear = -fnear;
		if (ffar<0)
			ffar = -ffar;
		if (fnear>ffar) {
			float tmp = fnear;
			fnear = ffar;
			ffar = tmp;
		}
		float xmin, xmax, ymin, ymax;
		ymax = fnear*tan(DEG_TO_RAD(fov*0.5f));
		ymin = -ymax;
		xmax = ymax*aspect;
		xmin = -xmax;
		proj.m00 = 2 * fnear / (xmax - xmin);
		proj.m01 = 0;
		proj.m02 = (xmax + xmin) / (xmax - xmin);
		proj.m03 = 0;
		proj.m10 = 0;
		proj.m11 = 2 * fnear / (ymax - ymin);
		proj.m12 = (ymax + ymin) / (ymax - ymin);
		proj.m13 = 0;
		proj.m20 = 0;
		proj.m21 = 0;
		proj.m22 = (ffar + fnear) / (fnear - ffar);
		proj.m23 = 2 * ffar*fnear / (fnear - ffar);
		proj.m30 = 0;
		proj.m31 = 0;
		proj.m32 = -1;
		proj.m33 = 0;
	}

	void set_ui_projection(xmat4f_t &proj, float screen_width, float screen_height, float z_range)
	{
		set_orthograph(proj, 0, 0, 0, screen_width, screen_height, z_range * 2);
		wyc::xmat4f_t mat_ui;
		mat_ui.identity();
		wyc::xvec4f_t vec;
		vec = { 0, -1, 0, screen_height };
		mat_ui.set_row(1, vec);
		vec = { 0, 0, 1, -z_range };
		mat_ui.set_row(2, vec);
		proj.mul(mat_ui);
	}

} // namespace wyc

