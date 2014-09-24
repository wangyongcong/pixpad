#ifndef WYC_HEADER_VECTOR
#define WYC_HEADER_VECTOR

#include <initializer_list>

namespace wyc
{
	template<class T, int D>
	struct xvector
	{
	public:
		typedef T element_t;
		enum { DIMENSION = D };

		T _elem[D];

		inline xvector& operator = (const xvector &v)
		{
			memcpy(_elem, v._elem, sizeof(T)*D);
			return *this;
		}
		xvector& operator = (T val)
		{
			unsigned i = 0;
			while (i<D)
				_elem[i++] = val;
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
		xvector& operator = (std::initializer_list<T> init_list) {
			int i = 0;
			for (auto iter = init_list.begin(), end = init_list.end(); iter != end && i < DIMENSION; ++iter, ++i)
				this->_elem[i] = *iter;
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
		inline xvector& operator *= (T val)
		{
			for (int i = 0; i<D; ++i)
				_elem[i] *= val;
			return *this;
		}
		inline xvector& operator *= (const xvector &v)
		{
			for (int i = 0; i<D; ++i)
				_elem[i] *= v._elem[i];
			return *this;
		}
		inline xvector& operator /= (T val)
		{
			for (int i = 0; i<D; ++i)
				_elem[i] /= val;
			return *this;
		}
		inline xvector& operator /= (const xvector &v)
		{
			for (int i = 0; i<D; ++i)
				_elem[i] /= v._elem[i];
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
		T length2() const
		{
			T sum = 0;
			for (int i = 0; i<D; ++i)
				sum += _elem[i] * _elem[i];
			return sum;
		}
		inline T length() const
		{
			return (sqrt(length2()));
		}
		void normalize()
		{
			T len = length();
			if (fequal(len, T(0)))
				return;
			len = T(1) / len;
			for (int i = 0; i<D; ++i)
				_elem[i] *= len;
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
	};


	template<class T>
	struct xvector<T, 2>
	{
	public:
		typedef T element_t;
		enum { DIMENSION = 2 };
		union
		{
			T	_elem[2];
			struct
			{
				T	x, y;
			};
		};
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
			return _elem[i];
		}
		inline T& operator [] (unsigned i)
		{
			assert(i<2);
			return _elem[i];
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
		inline xvector& operator *= (T val)
		{
			x *= val;
			y *= val;
			return *this;
		}
		inline xvector& operator *= (const xvector &v)
		{
			x *= v.x;
			y *= v.y;
			return *this;
		}
		inline xvector& operator /= (T val)
		{
			x /= val;
			y /= val;
			return *this;
		}
		inline xvector& operator /= (const xvector &v)
		{
			x /= v.x;
			y /= v.y;
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
		inline T length() const
		{
			return (sqrt(x*x + y*y));
		}
		void normalize()
		{
			T len = length();
			if (fequal(len, T(0)))
				return;
			len = T(1) / len;
			x *= len;
			y *= len;
		}
		bool operator == (const xvector& v) const
		{
			return x == v.x && y == v.y;
		}
		bool operator != (const xvector& v) const
		{
			return x != v.x || y != v.y;
		}
	};


	template<class T>
	class xvector<T, 3>
	{
	public:
		typedef T element_t;
		enum { DIMENSION = 3 };
		union
		{
			T	_elem[3];
			struct
			{
				T	x, y, z;
			};
		};
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
			for (int i = 0; i < cnt; ++i)
				this->_elem[i] = v._elem[i]
			return *this;
		}
		template<>
		xvector& operator = (const xvector<T, 2> &v)
		{
			x = v.x;
			y = v.y;
			z = 0;
		}
		template<>
		xvector& operator = (const xvector<T, 4> &v)
		{
			x = v.x;
			y = v.y;
			z = v.z;
		}
		inline void zero()
		{
			x = y = z = 0;
		}
		inline void set(T vx, T vy, T vz)
		{
			x = vx;
			y = vy;
			z = vz;
		}
		inline T operator [] (unsigned i) const
		{
			assert(i<3);
			return _elem[i];
		}
		inline T& operator [] (unsigned i)
		{
			assert(i<3);
			return _elem[i];
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
		inline xvector& operator *= (T val)
		{
			x *= val;
			y *= val;
			z *= val;
			return *this;
		}
		inline xvector& operator *= (const xvector &v)
		{
			x *= v.x;
			y *= v.y;
			z *= v.z;
			return *this;
		}
		inline xvector& operator /= (T val)
		{
			x /= val;
			y /= val;
			z /= val;
			return *this;
		}
		inline xvector& operator /= (const xvector &v)
		{
			x /= v.x;
			y /= v.y;
			z /= v.z;
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
		inline T length() const
		{
			return sqrt(x*x + y*y + z*z);
		}
		void normalize()
		{
			T len = length();
			if (fequal(len, T(0)))
				return;
			len = T(1) / len;
			x *= len;
			y *= len;
			z *= len;
		}
		bool operator == (const xvector& v) const
		{
			return x == v.x && y == v.y && z == v.z;
		}
		bool operator != (const xvector& v) const
		{
			return x != v.x || y != v.y || z != v.z;
		}
	};

	template<class T>
	class xvector<T, 4>
	{
	public:
		typedef T element_t;
		enum { DIMENSION = 4 };
		union
		{
			T	_elem[4];
			struct
			{
				T	x, y, z, w;
			};
		};

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
			for (int i = 0; i < cnt; ++i)
				this->_elem[i] = v._elem[i]
			return *this;
		}
		template<>
		xvector& operator = (const xvector<T, 2> &v)
		{
			x = v.x;
			y = v.y;
			z = 0;
			w = 1;
		}
		template<>
		xvector& operator = (const xvector<T, 3> &v)
		{
			x = v.x;
			y = v.y;
			z = v.z;
			w = 1;
		}
		inline void zero()
		{
			x = y = z = w = 0;
		}
		inline void set(T vx, T vy, T vz, T vw = 1)
		{
			x = vx;
			y = vy;
			z = vz;
			w = vw;
		}
		inline T operator [] (unsigned i) const
		{
			assert(i<4);
			return _elem[i];
		}
		inline T& operator [] (unsigned i)
		{
			assert(i<4);
			return _elem[i];
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
		inline xvector& operator *= (T val)
		{
			x *= val;
			y *= val;
			z *= val;
			w *= val;
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
		inline xvector& operator /= (T val)
		{
			x /= val;
			y /= val;
			z /= val;
			w /= val;
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
		inline T length2() const
		{
			return (x*x + y*y + z*z + w*w);
		}
		inline T length() const
		{
			return (sqrt(x*x + y*y + z*z + w*w));
		}
		void normalize()
		{
			T len = length();
			if (fequal(len, T(0)))
				return;
			len = T(1) / len;
			x *= len;
			y *= len;
			z *= len;
			w *= len;
		}
		bool operator == (const xvector& v) const
		{
			return x == v.x && y == v.y && z == v.z && w == v.w;
		}
		bool operator != (const xvector& v) const
		{
			return x != v.x || y != v.y || z != v.z || w != v.w;
		}
	};

	template<class T, int D>
	xvector<T, D> operator + (const xvector<T, D> &v1, const xvector<T, D> &v2)
	{
		xvector<T, D> r = v1;
		r += v2;
		return r;
	}

	template<class T, int D>
	xvector<T, D> operator - (const xvector<T, D> &v1, const xvector<T, D> &v2)
	{
		xvector<T, D> r = v1;
		r -= v2;
		return r;
	}

	template<class T, int D>
	xvector<T, D> operator * (const xvector<T, D> &v1, T val)
	{
		xvector<T, D> r = v1;
		r *= val;
		return r;
	}

	template<class T, int D>
	xvector<T, D> operator * (T val, const xvector<T, D> &v1)
	{
		xvector<T, D> r = v1;
		r *= val;
		return r;
	}

	template<class T, int D>
	xvector<T, D> operator * (const xvector<T, D> &v1, const xvector<T, D> &v2)
	{
		xvector<T, D> r = v1;
		r *= v2;
		return r;
	}

	template<class T, int D>
	xvector<T, D> operator / (const xvector<T, D> &v1, T val)
	{
		xvector<T, D> r = v1;
		r /= val;
		return r;
	}

	template<class T, int D>
	xvector<T, D> operator / (T val, const xvector<T, D> &v1)
	{
		xvector<T, D> r = v1;
		r.reciprocal()
		r *= val;
		return r;
	}

	template<class T, int D>
	xvector<T, D> operator / (const xvector<T, D> &v1, const xvector<T, D> &v2)
	{
		xvector<T, D> r = v1;
		r /= v2;
		return r;
	}

} // endof namespace wyc

#endif // WYC_HEADER_VECTOR