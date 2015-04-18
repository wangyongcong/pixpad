#ifndef WYC_HEADER_VECTOR
#define WYC_HEADER_VECTOR

#include <limits>

namespace wyc
{

// implement arithmetic operators
// T: vector type
// E: vector element type
#define vector_operator_helper(T, E) \
		inline T fast_length() const\
		{\
			return sqrt(length2());\
		}\
		friend inline T operator + (const T &vec, E scalar)\
		{\
			T res(vec); res += scalar; return res;\
		}\
		friend inline T operator + (E scalar, const T &vec)\
		{\
			return vec + scalar;\
		}\
		friend inline T operator - (const T &vec, E scalar)\
		{\
			return vec + (-scalar);\
		}\
		friend inline T operator * (const T &vec, E scalar)\
		{\
			T res(vec); res *= scalar; return res;\
		}\
		friend inline T operator * (E scalar, const T &vec)\
		{\
			return vec * scalar;\
		}\
		friend inline T operator / (const T &vec, E scalar)\
		{\
			T res(vec); res /= scalar; return res;\
		}\
		friend inline T operator + (T const& lhs, T const& rhs)\
		{\
			T res(lhs); res += rhs; return res;\
		}\
		friend inline T operator - (T const& lhs, T const& rhs)\
		{\
			T res(lhs); res -= rhs; return res;\
		}\
		friend inline T operator * (T const& lhs, T const& rhs)\
		{\
			T res(lhs); res *= rhs; return res;\
		}\
		friend inline T operator / (T const& lhs, T const& rhs)\
		{\
			T res(lhs); res /= rhs; return res;\
		}\
		friend inline E operator ^ (T const& lhs, T const& rhs)\
		{\
			return lhs.dot(rhs);\
		}\
		friend inline bool operator >= (T const& lhs, T const& rhs)\
		{\
			return !(lhs <  rhs);\
		}\
		friend inline bool operator <= (T const& lhs, T const& rhs)\
		{\
			return !(rhs <  lhs);\
		}\
		friend inline bool operator > (T const& lhs, T const& rhs)\
		{\
			return  (rhs <  lhs);\
		}\

	template<class T, int D>
	struct xvector
	{
		typedef T scalar_t;
		enum { DIMENSION = D };

		T _elem[D];

		xvector& operator = (T val)
		{
			unsigned i = 0;
			while (i<D)
				_elem[i++] = val;
			return *this;
		}
		inline xvector& operator = (const xvector &v)
		{
			memcpy(_elem, v._elem, sizeof(T)*D);
			return *this;
		}
		template<int D2>
		inline xvector& operator = (const xvector<T, D2> &v)
		{
			assert(D != D2);
			if (D > D2)
			{
				memcpy(this->_elem, v._elem, sizeof(T)*D2);
				for (int i = D2; i < D; ++i)
					this->_elem[i] = 0;
			}
			else
				memcpy(this->_elem, v._elem, sizeof(T)*D);
			return *this;
		}
		inline void zero()
		{
			memset(_elem, 0, sizeof(T)*D);
		}
		inline T operator [] (unsigned i) const
		{
			assert(i<D);
			return _elem[i];
		}
		inline T& operator [] (unsigned i)
		{
			assert(i<D);
			return _elem[i];
		}
		inline xvector operator - () const {
			xvector ret;
			for (int i = 0; i < D; ++i)
				ret._elem[i] = -_elem[i];
			return ret;
		}
		inline xvector& operator += (const xvector &v)
		{
			for (int i = 0; i<D; ++i)
				_elem[i] += v._elem[i];
			return *this;
		}
		inline xvector& operator -= (const xvector &v)
		{
			for (int i = 0; i<D; ++i)
				_elem[i] -= v._elem[i];
			return *this;
		}
		inline xvector& operator *= (const xvector &v)
		{
			for (int i = 0; i<D; ++i)
				_elem[i] *= v._elem[i];
			return *this;
		}
		inline xvector& operator /= (const xvector &v)
		{
			for (int i = 0; i<D; ++i)
				_elem[i] /= v._elem[i];
			return *this;
		}
		inline xvector& operator += (T scalar)
		{
			for (int i = 0; i<D; ++i)
				_elem[i] += scalar;
			return *this;
		}
		inline xvector& operator -= (T scalar)
		{
			return *this += -scalar;
		}
		inline xvector& operator *= (T scalar)
		{
			for (int i = 0; i<D; ++i)
				_elem[i] *= scalar;
			return *this;
		}
		inline xvector& operator /= (T scalar)
		{
			for (int i = 0; i<D; ++i)
				_elem[i] /= scalar;
			return *this;
		}
		xvector& reverse()
		{
			for (int i = 0; i<D; ++i)
				_elem[i] = -_elem[i];
			return *this;
		}
		xvector& reciprocal()
		{
			for (int i = 0; i<D; ++i)
				_elem[i] = 1.0f / _elem[i];
			return *this;
		}
		T dot(const xvector &v) const
		{
			T sum = 0;
			for (int i = 0; i<D; ++i)
				sum += _elem[i] * v._elem[i];
			return sum;
		}
		xvector cross(const xvector &v) const
		{
			xvector ret;
			ret.zero();
			return ret;
		}
		T length2() const
		{
			T sum = 0;
			for (int i = 0; i<D; ++i)
				sum += _elem[i] * _elem[i];
			return sum;
		}
		inline T length() const
		{
			return sqrt(length2());
		}
		void normalize()
		{
			T len = length();
			if (len == 0)
				return;
			*this /= len;
		}
		bool operator == (const xvector& v) const
		{
			for (int i = 0; i<D; ++i)
				if (_elem[i] != v._elem[i])
					return false;
			return true;
		}
		bool operator != (const xvector& v) const
		{
			for (int i = 0; i<D; ++i)
				if (_elem[i] != v._elem[i])
					return true;
			return false;
		}
		bool operator < (const xvector& v) const
		{
			for (int i = 0; i < D; ++i) 
				if (!(_elem[i] < v._elem[i]))
					return false;
			return true;
		}
		vector_operator_helper(xvector, T)
	};


	template<class T>
	struct xvector<T, 2>
	{
		typedef T scalar_t;
		enum { DIMENSION = 2 };
		T	x, y;
		inline xvector& operator = (T val)
		{
			x = y = val;
			return *this;
		}
		inline xvector& operator = (const xvector &v)
		{
			x = v.x;
			y = v.y;
			return *this;
		}
		template<int D2>
		inline xvector& operator = (const xvector<T, D2> &v)
		{
			assert(1<D2);
			x = v[0];
			y = v[1];
			return *this;
		}
		inline void zero()
		{
			x = y = 0;
		}
		inline T operator [] (unsigned i) const
		{
			assert(i<2);
			return (&x)[i];
		}
		inline T& operator [] (unsigned i)
		{
			assert(i<2);
			return (&x)[i];
		}
		inline xvector operator - () const
		{
			xvector ret;
			ret.x = -x;
			ret.y = -y;
			return ret;
		}
		inline xvector& operator += (const xvector &v)
		{
			x += v.x;
			y += v.y;
			return *this;
		}
		inline xvector& operator -= (const xvector &v)
		{
			x -= v.x;
			y -= v.y;
			return *this;
		}
		inline xvector& operator *= (const xvector &v)
		{
			x *= v.x;
			y *= v.y;
			return *this;
		}
		inline xvector& operator /= (const xvector &v)
		{
			x /= v.x;
			y /= v.y;
			return *this;
		}
		inline xvector& operator += (T scalar)
		{
			x += scalar;
			y += scalar;
			return *this;
		}
		inline xvector& operator -= (T scalar)
		{
			x -= scalar;
			y -= scalar;
			return *this;
		}
		inline xvector& operator *= (T scalar)
		{
			x *= scalar;
			y *= scalar;
			return *this;
		}
		inline xvector& operator /= (T scalar)
		{
			x /= scalar;
			y /= scalar;
			return *this;
		}
		inline xvector& reverse()
		{
			x = -x;
			y = -y;
			return *this;
		}
		inline xvector& reciprocal()
		{
			x = T(1) / x;
			y = T(1) / y;
			return *this;
		}
		inline T dot(const xvector &v) const
		{
			return (x*v.x + y*v.y);
		}
		T cross(const xvector &v1) const
		{
			return x*v1.y - y*v1.x;
		}
		inline T length2() const
		{
			return (x*x + y*y);
		}
		T length() const
		{
			T len = length2();
			if (len > std::numeric_limits<T>::min() * 2)
				return sqrt(len);
			T absx = abs(x);
			T absy = abs(y);
			if (absy < absx)
				len = absx;
			else
				len = absy;
			if (len == 0)
				return 0;
			absx /= len;
			absy /= len;
			return len * sqrt(absx*absx + absy*absy);
		}
		void normalize()
		{
			T len = length();
			if (len == 0)
				return;
			x /= len;
			y /= len;
		}
		bool operator == (const xvector& v) const
		{
			return x == v.x && y == v.y;
		}
		bool operator != (const xvector& v) const
		{
			return x != v.x || y != v.y;
		}
		inline bool operator < (const xvector& v) const
		{
			return x < v.x && y < v.y;
		}
		vector_operator_helper(xvector, T)
	};


	template<class T>
	struct xvector<T, 3>
	{
		typedef T scalar_t;
		enum { DIMENSION = 3 };
		T	x, y, z;
		inline xvector& operator = (T val)
		{
			x = y = z = val;
			return *this;
		}
		inline xvector& operator = (const xvector &v)
		{
			x = v.x;
			y = v.y;
			z = v.z;
			return *this;
		}
		template<int D2>
		xvector& operator = (const xvector<T, D2> &v)
		{
			assert(DIMENSION != D2);
			int cnt = DIMENSION < D2 ? DIMENSION : D2;
			T *elem = &x;
			for (int i = 0; i < cnt; ++i)
				elem[i] = v[i]
			return *this;
		}
		template<>
		xvector& operator = (const xvector<T, 2> &v)
		{
			x = v.x;
			y = v.y;
			z = 0;
			return *this;
		}
		template<>
		xvector& operator = (const xvector<T, 4> &v)
		{
			x = v.x;
			y = v.y;
			z = v.z;
			return *this;
		}
		inline void zero()
		{
			x = y = z = 0;
		}
		inline T operator [] (unsigned i) const
		{
			assert(i<3);
			return (&x)[i];
		}
		inline T& operator [] (unsigned i)
		{
			assert(i<3);
			return (&x)[i];
		}

		inline xvector operator - () const
		{
			xvector ret;
			ret.x = -x;
			ret.y = -y;
			ret.z = -z;
			return ret;
		}
		inline xvector& operator += (const xvector &v)
		{
			x += v.x;
			y += v.y;
			z += v.z;
			return *this;
		}
		inline xvector& operator -= (const xvector &v)
		{
			x -= v.x;
			y -= v.y;
			z -= v.z;
			return *this;
		}
		inline xvector& operator *= (const xvector &v)
		{
			x *= v.x;
			y *= v.y;
			z *= v.z;
			return *this;
		}
		inline xvector& operator /= (const xvector &v)
		{
			x /= v.x;
			y /= v.y;
			z /= v.z;
			return *this;
		}
		inline xvector& operator += (T scalar)
		{
			x += scalar;
			y += scalar;
			z += scalar;
			return *this;
		}
		inline xvector& operator -= (T scalar)
		{
			x -= scalar;
			y -= scalar;
			z -= scalar;
			return *this;
		}
		inline xvector& operator *= (T scalar)
		{
			x *= scalar;
			y *= scalar;
			z *= scalar;
			return *this;
		}
		inline xvector& operator /= (T scalar)
		{
			x /= scalar;
			y /= scalar;
			z /= scalar;
			return *this;
		}
		inline xvector& reverse()
		{
			x = -x;
			y = -y;
			z = -z;
			return *this;
		}
		inline xvector& reciprocal()
		{
			x = T(1) / x;
			y = T(1) / y;
			z = T(1) / z;
			return *this;
		}
		inline T dot(const xvector &v) const
		{
			return (x*v.x + y*v.y + z*v.z);
		}
		xvector cross(const xvector &v) const
		{
			xvector ret;
			ret.x = y*v.z - z*v.y;
			ret.y = z*v.x - x*v.z;
			ret.z = x*v.y - y*v.x;
			return ret;
		}
		inline T length2() const
		{
			return (x*x + y*y + z*z);
		}
		T length() const
		{
			T len = length2();
			if (len > std::numeric_limits<T>::min() * 2)
				return sqrt(len);
			T absx = abs(x);
			T absy = abs(y);
			T absz = abs(z);
			if (absy < absx)
				len = absx;
			else
				len = absy;
			if (len < absz)
				len = absz;
			if (len == 0)
				return 0;
			absx /= len;
			absy /= len;
			absz /= len;
			return len * sqrt(absx*absx + absy*absy + absz*absz);
		}
		void normalize()
		{
			T len = length();
			if (len == 0)
				return;
			x /= len;
			y /= len;
			z /= len;
		}
		inline bool operator == (const xvector& v) const
		{
			return x == v.x && y == v.y && z == v.z;
		}
		inline bool operator != (const xvector& v) const
		{
			return x != v.x || y != v.y || z != v.z;
		}
		inline bool operator < (const xvector& v) const
		{
			return x < v.x && y < v.y && z < v.z;
		}
		vector_operator_helper(xvector, T)
	};

	template<class T>
	struct xvector<T, 4>
	{
		typedef T scalar_t;
		enum { DIMENSION = 4 };
		T	x, y, z, w;

		inline xvector& operator = (T val)
		{
			x = y = z = w = val;
			return *this;
		}
		inline xvector& operator = (const xvector &v)
		{
			x = v.x;
			y = v.y;
			z = v.z;
			w = v.w;
			return *this;
		}
		template<int D2>
		xvector& operator = (const xvector<T, D2> &v)
		{
			assert(DIMENSION != D2);
			int cnt = DIMENSION < D2 ? DIMENSION : D2;
			T *elem = &x
			for (int i = 0; i < cnt; ++i)
				elem[i] = v[i]
			return *this;
		}
		template<>
		xvector& operator = (const xvector<T, 2> &v)
		{
			x = v.x;
			y = v.y;
			z = 0;
			w = 1;
			return *this;
		}
		template<>
		xvector& operator = (const xvector<T, 3> &v)
		{
			x = v.x;
			y = v.y;
			z = v.z;
			w = 1;
			return *this;
		}
		inline void zero()
		{
			x = y = z = w = 0;
		}
		inline T operator [] (unsigned i) const
		{
			assert(i<4);
			return (&x)[i];
		}
		inline T& operator [] (unsigned i)
		{
			assert(i<4);
			return (&x)[i];
		}
		inline xvector operator - () const
		{
			xvector ret;
			ret.x = -x;
			ret.y = -y;
			ret.z = -z;
			ret.w = -w;
			return ret;
		}
		inline xvector& operator += (const xvector &v)
		{
			x += v.x;
			y += v.y;
			z += v.z;
			w += v.w;
			return *this;
		}
		inline xvector& operator -= (const xvector &v)
		{
			x -= v.x;
			y -= v.y;
			z -= v.z;
			w -= v.w;
			return *this;
		}
		inline xvector& operator *= (const xvector &v)
		{
			x *= v.x;
			y *= v.y;
			z *= v.z;
			w *= v.w;
			return *this;
		}
		inline xvector& operator /= (const xvector &v)
		{
			x /= v.x;
			y /= v.y;
			z /= v.z;
			w /= v.w;
			return *this;
		}
		inline xvector& operator += (T scalar)
		{
			x += scalar;
			y += scalar;
			z += scalar;
			w += scalar;
			return *this;
		}
		inline xvector& operator -= (T scalar)
		{
			x -= scalar;
			y -= scalar;
			z -= scalar;
			w -= scalar;
			return *this;
		}
		inline xvector& operator *= (T scalar)
		{
			x *= scalar;
			y *= scalar;
			z *= scalar;
			w *= scalar;
			return *this;
		}
		inline xvector& operator /= (T scalar)
		{
			x /= scalar;
			y /= scalar;
			z /= scalar;
			w /= scalar;
			return *this;
		}
		inline xvector& reverse()
		{
			x = -x;
			y = -y;
			z = -z;
			w = -w;
			return *this;
		}
		inline xvector& reciprocal()
		{
			x = T(1) / x;
			y = T(1) / y;
			z = T(1) / z;
			w = T(1) / w;
			return *this;
		}
		inline xvector& reverse(const xvector &v)
		{
			x = -v.x;
			y = -v.y;
			z = -v.z;
			w = -v.w;
			return *this;
		}
		inline T dot(const xvector &v) const
		{
			return (x*v.x + y*v.y + z*v.z + w*v.w);
		}
		xvector cross(const xvector &v) const
		{
			xvector ret;
			ret.x = y*v.z - z*v.y;
			ret.y = z*v.x - x*v.z;
			ret.z = x*v.y - y*v.x;
			ret.w = 1;
			return ret;
		}
		inline T length2() const
		{
			return (x*x + y*y + z*z + w*w);
		}
		T length() const
		{
			T len = length2();
			if (len > std::numeric_limits<T>::min() * 2)
				return sqrt(len);
			T absx = abs(x);
			T absy = abs(y);
			T absz = abs(z);
			T absw = abs(w);
			len = absy < absx ? absx : absy;
			if (len < absz)
				len = absz;
			if (len < absw)
				len = absw;
			if (len == 0)
				return 0;
			absx /= len;
			absy /= len;
			absz /= len;
			absw /= len;
			return len * sqrt(absx*absx + absy*absy + absz*absz + absw*absw);
		}
		void normalize()
		{
			T len = length();
			if (len == 0)
				return;
			x /= len;
			y /= len;
			z /= len;
			w /= len;
		}
		bool operator == (const xvector& v) const
		{
			return x == v.x && y == v.y && z == v.z && w == v.w;
		}
		bool operator != (const xvector& v) const
		{
			return x != v.x || y != v.y || z != v.z || w != v.w;
		}
		inline bool operator < (const xvector& v) const
		{
			return x < v.x && y < v.y && z < v.z && w < v.w;
		}
		vector_operator_helper(xvector, T)
	};

	template <class T>
	inline T operator ^ (const xvector<T, 4> &v4, const xvector<T, 3> &v3)
	{
		return v4.x * v3.x + v4.y * v3.y + v4.z * v3.z + v4.w;
	}

	template <class T>
	inline T operator ^ (const xvector<T, 3> &v3, const xvector<T, 4> &v4)
	{
		return v4 ^ v3;
	}


} // endof namespace wyc

#endif // WYC_HEADER_VECTOR