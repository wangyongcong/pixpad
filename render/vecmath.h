#ifndef WYC_HEADER_VECMATH
#define WYC_HEADER_VECMATH

#include <vector>
#include "mathfwd.h"

namespace wyc
{

#ifndef M_PI
#define M_PI    3.14159265358979323846
#endif

#ifndef M_PI_2
#define M_PI_2  1.57079632679489661923 // pi/2
#endif

// Napier's constant, its symbol (e) honors Euler
#ifndef M_E
#define M_E		2.71828182845904523536
#endif

	template<class T>
	inline T deg2rad(T angle)
	{
		return T(angle * M_PI / 180.0);
	}

	template<class VEC>
	inline VEC intersect(const VEC &p1, typename VEC::scalar_t d1, const VEC &p2, typename VEC::scalar_t d2)
	{
		typename VEC::scalar_t t = d1 / (d1 - d2);
		if (d1 < 0)
			t = std::ceil(t * 1000) * 0.001f;
		else
			t = std::floor(t * 1000) * 0.001f;
		return p1 + (p2 - p1) * t;
	}

	// OpenGL orthograph matrix
	void set_orthograph(mat4f &proj, float left_plane, float bottom_plane, float near_plane, 
		float right_plane, float top_plane, float far_plane);

	// OpenGL perspective matrix
	// yfov: vertical FOV angle in degree
	// aspect: width/height
	// near_plane/far_plane: distance of near plane and far plane
	void set_perspective(mat4f &proj, float yfov, float aspect, float near_plane, float far_plane);

	//
	// 2D homogeneous transform matrix
	//
	// translate (x, y)
	void set_translate(mat3f &m, float x, float y);
	// scale (sx, sy)
	void set_scale(mat3f &m, float sx, float sy);
	// rotate radian counter-clockwise
	void set_rotate(mat3f &m, float radian);

	//
	// 3D homogeneous transform matrix
	//
	// translate (x, y, z)
	void set_translate(mat4f &m, float x, float y, float z);
	// scale (sx, sy, sz)
	void set_scale(mat4f &m, float sx, float sy, float sz);
	// rotate radian counter-clockwise around the normalized vector N
	void set_rotate(mat4f &m, const vec3f &n, float radian);
	// rotate radian counter-clockwise around +X axis (1, 0, 0)
	void set_rotate_x(mat4f &m, float radian);
	// rotate radian counter-clockwise around +Y axis (0, 1, 0)
	void set_rotate_y(mat4f &m, float radian);
	// rotate radian counter-clockwise around +Z axis (0, 0, 1)
	void set_rotate_z(mat4f &m, float radian);

	
	// Clip polygon by planes
	// planes: planes used to clip the polygon
	// vertices: vertices of the polygon, when return, it will contain the result
	void clip_polygon(const std::vector<vec4f> &planes, std::vector<vec3f> &vertices);

	// Clip polygon in homogeneous clip space
	void clip_polygon_homo(std::vector<vec4f> &vertices);

} // namespace wyc

#endif // WYC_HEADER_VECMATH
