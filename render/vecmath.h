#ifndef WYC_HEADER_VECMATH
#define WYC_HEADER_VECMATH

#include <OpenEXR/ImathVec.h>
#include <OpenEXR/ImathMatrix.h>

typedef IMATH_NAMESPACE::Vec2<float> vec2f_t;
typedef IMATH_NAMESPACE::Vec3<float> vec3f_t;
typedef IMATH_NAMESPACE::Vec4<float> vec4f_t;

typedef IMATH_NAMESPACE::Matrix33<float> mat3f_t;
typedef IMATH_NAMESPACE::Matrix44<float> mat4f_t;

namespace wyc
{
	template<typename T>
	inline T deg2rad(T angle)
	{
		return T(angle * M_PI / 180.0);
	}

	// OpenGL orthograph matrix
	void set_orthograph(mat4f_t &proj, float xmin, float ymin, float zmin, float xmax, float ymax, float zmax);

	// OpenGL perspective matrix
	// aspect = width/height
	void set_perspective(mat4f_t &proj, float yfov, float aspect, float near_plane, float far_plane);


} // namespace wyc

#endif // WYC_HEADER_VECMATH
