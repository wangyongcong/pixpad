#ifndef WYC_HEADER_VECMATH
#define WYC_HEADER_VECMATH

#include <vector>
#include <OpenEXR/ImathVec.h>
#include <OpenEXR/ImathMatrix.h>

typedef IMATH_NAMESPACE::Vec2<float> vec2f_t;
typedef IMATH_NAMESPACE::Vec3<float> vec3f_t;
typedef IMATH_NAMESPACE::Vec4<float> vec4f_t;

typedef IMATH_NAMESPACE::Matrix33<float> mat3f_t;
typedef IMATH_NAMESPACE::Matrix44<float> mat4f_t;

template <class T>
inline T operator ^ (const IMATH_NAMESPACE::Vec4<T> &v1, const IMATH_NAMESPACE::Vec3<T> &v2)
{
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w;
}

template <class T>
inline T operator ^ (const IMATH_NAMESPACE::Vec3<T> &v1, const IMATH_NAMESPACE::Vec4<T> &v2)
{
	return v2 ^ v1;
}

namespace wyc
{
	template<typename T>
	inline T deg2rad(T angle)
	{
		return T(angle * M_PI / 180.0);
	}

	// OpenGL orthograph matrix
	void set_orthograph(mat4f_t &proj, float left_plane, float bottom_plane, float near_plane, 
		float right_plane, float top_plane, float far_plane);

	// OpenGL perspective matrix
	// aspect: width/height
	// near_plane/far_plane: distance of near plane and far plane
	void set_perspective(mat4f_t &proj, float yfov, float aspect, float near_plane, float far_plane);
	
	// Clip polygons by planes
	// planes: planes used to clip the polygon
	// vertices: vertices of the polygon, when return, it will contain the result
	void clip_polygons(const std::vector<vec4f_t> &planes, std::vector<vec3f_t> &vertices);

} // namespace wyc

#endif // WYC_HEADER_VECMATH
