#ifndef WYC_HEADER_VECTOR
#define WYC_HEADER_VECTOR

namespace wyc
{
	template<class T, int D>
	struct xvector
	{
	public:
		typedef T element_t;
		enum { DIMENSION = D };

		T elem[D];

		inline xvector& operator = (const xvector &v)
		{
			memcpy(elem, v.elem, sizeof(T)*D);
			return *this;
		}
		xvector& operator = (T val)
		{
			unsigned i = 0;
			while (i<D)
				elem[i++] = val;
			return *this;
		}
		template<int D2>
		inline xvector& operator = (const xvector<T, D2> &v)
		{
			assert(D != D2);
			memcpy(elem, v.elem, sizeof(T)*(D<D2 ? D : D2));
			return *this;
		}
		inline void zero()
		{
			memset(elem, 0, sizeof(T)*D);
		}
		inline T operator [] (unsigned i) const
		{
			assert(i<D);
			return elem[i];
		}
		inline T& operator [] (unsigned i)
		{
			assert(i<D);
			return elem[i];
		}
		inline xvector operator - () const {
			xvector ret;
			for (int i = 0; i < D; ++i)
				ret.elem[i] = -v.elem[i];
			return ret;
		}
		inline xvector& operator += (const xvector &v)
		{
			for (int i = 0; i<D; ++i)
				elem[i] += v.elem[i];
			return *this;
		}
		inline xvector& operator -= (const xvector &v)
		{
			for (int i = 0; i<D; ++i)
				elem[i] -= v.elem[i];
			return *this;
		}
		inline xvector& operator *= (T val)
		{
			for (int i = 0; i<D; ++i)
				elem[i] *= val;
			return *this;
		}

		xvector& reverse()
		{
			for (int i = 0; i<D; ++i)
				elem[i] = -elem[i];
			return *this;
		}
		xvector& reverse(const xvector &v)
		{
			for (int i = 0; i<D; ++i)
				elem[i] = -v.elem[i];
			return *this;
		}

		T dot(const xvector &v) const
		{
			T sum = 0;
			for (int i = 0; i<D; ++i)
				sum += elem[i] * v.elem[i];
			return sum;
		}

		T length2() const
		{
			T sum = 0;
			for (int i = 0; i<D; ++i)
				sum += elem[i] * elem[i];
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
				elem[i] *= len;
		}

		bool operator == (const xvector& v) const
		{
			for (int i = 0; i<D; ++i)
				if (elem[i] != v.elem[i])
					return false;
			return true;
		}
		bool operator != (const xvector& v) const
		{
			for (int i = 0; i<D; ++i)
				if (elem[i] != v.elem[i])
					return true;
			return false;
		}
	};


	template<class T>
	class xvector<T, 2>
	{
	public:
		typedef T element_t;
		enum { DIMENSION = 2 };
		union
		{
			T	elem[2];
			struct
			{
				T	x, y;
			};
		};

		inline xvector& operator = (const xvector &v)
		{
			x = v.x;
			y = v.y;
			return *this;
		}
		inline xvector& operator = (T val)
		{
			x = y = val;
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
		inline void set(T vx, T vy)
		{
			x = vx;
			y = vy;
		}
		inline T operator () (unsigned i) const
		{
			assert(i<2);
			return elem[i];
		}
		inline T& operator () (unsigned i)
		{
			assert(i<2);
			return elem[i];
		}
		inline T operator [] (unsigned i) const
		{
			assert(i<2);
			return elem[i];
		}
		inline T& operator [] (unsigned i)
		{
			assert(i<2);
			return elem[i];
		}
		inline xvector& operator += (const xvector &v)
		{
			return add(v);
		}
		inline xvector& operator -= (const xvector &v)
		{
			return sub(v);
		}
		inline xvector& operator *= (T val)
		{
			return scale(val);
		}
		inline xvector& reverse()
		{
			x = -x;
			y = -y;
			return *this;
		}
		inline xvector& reverse(const xvector &v)
		{
			x = -v.x;
			y = -v.y;
			return *this;
		}
		inline xvector& add(const xvector &v)
		{
			x += v.x;
			y += v.y;
			return *this;
		}
		inline xvector& add(const xvector &v1, const xvector &v2)
		{
			x = v1.x + v2.x;
			y = v1.y + v2.y;
			return *this;
		}
		inline xvector& sub(const xvector &v)
		{
			x -= v.x;
			y -= v.y;
			return *this;
		}
		inline xvector& sub(const xvector &v1, const xvector &v2)
		{
			x = v1.x - v2.x;
			y = v1.y - v2.y;
			return *this;
		}
		inline xvector& scale(T val)
		{
			x *= val;
			y *= val;
			return *this;
		}
		inline xvector& scale(const xvector &v, T val)
		{
			x = v.x*val;
			y = v.y*val;
			return *this;
		}
		inline T dot(const xvector &v) const
		{
			return (x*v.x + y*v.y);
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
			len = 1 / len;
			x *= len;
			y *= len;
		}
		T cross(const xvector &v1) const
		{
			return x*v1.y - y*v1.x;
		}

		inline T cos_of_angle(const xvector &v) const
		{
			return (this->dot(v) / (length()*v.length()));
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
			T	elem[3];
			struct
			{
				T	x, y, z;
			};
		};

		inline xvector& operator = (const xvector &v)
		{
			x = v.x;
			y = v.y;
			z = v.z;
			return *this;
		}
		inline xvector& operator = (T val)
		{
			x = y = z = val;
			return *this;
		}
		template<int D2>
		xvector& operator = (const xvector<T, D2> &v)
		{
			assert(3 != D2);
			memcpy(elem, v.elem, sizeof(T)*(3<D2 ? 3 : D2));
			return *this;
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

		inline T operator () (unsigned i) const
		{
			assert(i<3);
			return elem[i];
		}
		inline T& operator () (unsigned i)
		{
			assert(i<3);
			return elem[i];
		}
		inline T operator [] (unsigned i) const
		{
			assert(i<3);
			return elem[i];
		}
		inline T& operator [] (unsigned i)
		{
			assert(i<3);
			return elem[i];
		}

		inline xvector& operator += (const xvector &v)
		{
			return add(v);
		}
		inline xvector& operator -= (const xvector &v)
		{
			return sub(v);
		}
		inline xvector& operator *= (T val)
		{
			return scale(val);
		}
		inline xvector& reverse()
		{
			x = -x;
			y = -y;
			z = -z;
			return *this;
		}
		inline xvector& reverse(const xvector &v)
		{
			x = -v.x;
			y = -v.y;
			z = -v.z;
			return *this;
		}
		inline xvector& add(const xvector &v)
		{
			x += v.x;
			y += v.y;
			z += v.z;
			return *this;
		}
		inline xvector& add(const xvector &v1, const xvector &v2)
		{
			x = v1.x + v2.x;
			y = v1.y + v2.y;
			z = v1.z + v2.z;
			return *this;
		}
		inline xvector& sub(const xvector &v)
		{
			x -= v.x;
			y -= v.y;
			z -= v.z;
			return *this;
		}
		inline xvector& sub(const xvector &v1, const xvector &v2)
		{
			x = v1.x - v2.x;
			y = v1.y - v2.y;
			z = v1.z - v2.z;
			return *this;
		}
		inline xvector& scale(T val)
		{
			x *= val;
			y *= val;
			z *= val;
			return *this;
		}
		inline xvector& scale(const xvector &v, T val)
		{
			x = v.x*val;
			y = v.y*val;
			z = v.z*val;
			return *this;
		}
		inline T dot(const xvector &v) const
		{
			return (x*v.x + y*v.y + z*v.z);
		}
		inline T length2() const
		{
			return (x*x + y*y + z*z);
		}
		inline T length() const
		{
			return (sqrt(x*x + y*y + z*z));
		}
		void normalize()
		{
			T len = length();
			if (fequal(len, T(0)))
				return;
			len = 1 / len;
			x *= len;
			y *= len;
			z *= len;
		}
		xvector& cross(const xvector &v1, const xvector &v2)
		{
			x = v1.y*v2.z - v1.z*v2.y;
			y = v1.z*v2.x - v1.x*v2.z;
			z = v1.x*v2.y - v1.y*v2.x;
			return *this;
		}
		inline T cos_of_angle(const xvector &v) const
		{
			return (this->dot(v) / (length()*v.length()));
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


	// homogeneous vector
	template<class T>
	class xvector<T, 4>
	{
	public:
		typedef T element_t;
		enum { DIMENSION = 4 };
		union
		{
			T	elem[4];
			struct
			{
				T	x, y, z, w;
			};
		};

		inline xvector& operator = (const xvector &v)
		{
			x = v.x;
			y = v.y;
			z = v.z;
			w = v.w;
			return *this;
		}
		inline xvector& operator = (T val)
		{
			x = y = z = w = val;
			return *this;
		}
		template<int D2>
		xvector& operator = (const xvector<T, D2> &v)
		{
			assert(4 != D2);
			memcpy(elem, v.elem, sizeof(T)*(4<D2 ? 4 : D2));
			return *this;
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
		inline T operator () (unsigned i) const
		{
			assert(i<4);
			return elem[i];
		}
		inline T& operator () (unsigned i)
		{
			assert(i<4);
			return elem[i];
		}
		inline T operator [] (unsigned i) const
		{
			assert(i<4);
			return elem[i];
		}
		inline T& operator [] (unsigned i)
		{
			assert(i<4);
			return elem[i];
		}
		inline xvector& operator += (const xvector &v)
		{
			return add(v);
		}
		inline xvector& operator -= (const xvector &v)
		{
			return sub(v);
		}
		inline xvector& operator *= (T val)
		{
			return scale(val);
		}
		inline xvector& reverse()
		{
			x = -x;
			y = -y;
			z = -z;
			w = -w;
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
		inline xvector& add(const xvector &v)
		{
			x += v.x;
			y += v.y;
			z += v.z;
			w += v.w;
			return *this;
		}
		inline xvector& add(const xvector &v1, const xvector &v2)
		{
			x = v1.x + v2.x;
			y = v1.y + v2.y;
			z = v1.z + v2.z;
			w = v1.w + v2.w;
			return *this;
		}
		inline xvector& sub(const xvector &v)
		{
			x -= v.x;
			y -= v.y;
			z -= v.z;
			w -= v.w;
			return *this;
		}
		inline xvector& sub(const xvector &v1, const xvector &v2)
		{
			x = v1.x - v2.x;
			y = v1.y - v2.y;
			z = v1.z - v2.z;
			w = v1.w - v2.w;
			return *this;
		}
		inline xvector& scale(T val)
		{
			x *= val;
			y *= val;
			z *= val;
			w *= val;
			return *this;
		}
		inline xvector& scale(const xvector &v, T val)
		{
			x = v.x*val;
			y = v.y*val;
			z = v.z*val;
			w = v.w*val;
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
			len = 1 / len;
			x *= len;
			y *= len;
			z *= len;
			w *= len;
		}
		void homogenize()
		{
			if (fequal(w, T(0)) || fequal(w, T(1)))
				return;
			w = 1 / w;
			x *= w;
			y *= w;
			z *= w;
			w = 1;
		}
		xvector& cross(const xvector &v1, const xvector &v2)
		{
			x = v1.y*v2.z - v1.z*v2.y;
			y = v1.z*v2.x - v1.x*v2.z;
			z = v1.x*v2.y - v1.y*v2.x;
			return *this;
		}
		inline T cos_of_angle(const xvector &v) const
		{
			return (this->dot(v) / (length()*v.length()));
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
	xvector<T, D> operator - (const xvector<T, D> &v1)
	{
		xvector<T, D> r;
		r.reverse(v1);
		return r;
	}

	template<class T, int D>
	xvector<T, D> operator + (const xvector<T, D> &v1, const xvector<T, D> &v2)
	{
		xvector<T, D> r;
		r.add(v1, v2);
		return r;
	}

	template<class T, int D>
	xvector<T, D> operator - (const xvector<T, D> &v1, const xvector<T, D> &v2)
	{
		xvector<T, D> r;
		r.sub(v1, v2);
		return r;
	}

	template<class T, int D>
	xvector<T, D> operator * (const xvector<T, D> &v1, T val)
	{
		xvector<T, D> r;
		r.scale(v1, val);
		return r;
	}

	template<class T, int D>
	xvector<T, D> operator * (T val, const xvector<T, D> &v1)
	{
		xvector<T, D> r;
		r.scale(v1, val);
		return r;
	}

	template<class T, int D>
	T operator * (const xvector<T, D> &v1, const xvector<T, D> &v2)
	{
		return v1.dot(v2);
	}


} // endof namespace wyc

#endif // WYC_HEADER_VECTOR