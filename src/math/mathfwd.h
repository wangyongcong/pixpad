#ifndef WYC_HEADER_MATHFWD
#define WYC_HEADER_MATHFWD

namespace wyc
{
	template<class T, int D> struct xvector;
	template<class T> struct xvector < T, 2 > ;
	template<class T> struct xvector < T, 3 > ;
	template<class T> struct xvector < T, 4 > ;

	template<class T, int R, int C> struct xmatrix;
	template<class T> struct xmatrix < T, 2, 2 >;
	template<class T> struct xmatrix < T, 3, 3 >;
	template<class T> struct xmatrix < T, 4, 4 >;

	typedef xvector<int, 2> vec2i;
	typedef xvector<int, 3> vec3i;
	typedef xvector<int, 4> vec4i;

	typedef xvector<float, 2> vec2f;
	typedef xvector<float, 3> vec3f;
	typedef xvector<float, 4> vec4f;

	typedef xvector<double, 2> vec2d;
	typedef xvector<double, 3> vec3d;
	typedef xvector<double, 4> vec4d;

	typedef xmatrix<float, 2, 2> mat2f;
	typedef xmatrix<float, 3, 3> mat3f;
	typedef xmatrix<float, 4, 4> mat4f;

	typedef xmatrix<double, 2, 2> mat2d;
	typedef xmatrix<double, 3, 3> mat3d;
	typedef xmatrix<double, 4, 4> mat4d;

};

#endif // WYC_HEADER_MATHFWD