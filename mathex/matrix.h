#ifndef WYC_HEADER_MATRIX
#define WYC_HEADER_MATRIX

namespace wyc
{

	template<class T, int R, int C>
	struct xmatrix
	{
		typedef T element_t;
		enum {
			ROW = R,
			COL = C,
		};

		T	_elem[R][C]; // row first

		inline xmatrix& operator = (const xmatrix &m)
		{
			memcpy(_elem[0], m._elem[0], sizeof(T)*R*C);
			return *this;
		}

		inline bool square() const
		{
			return (R == C);
		}
		void identity()
		{
			memset(&(_elem[0][0]), 0, sizeof(T)*R*C);
			assert(R == C);
			for (int i = 0; i<R; ++i)
				_elem[i][i] = 1;
		}
		inline T* data()
		{
			return _elem[0];
		}
		inline const T* data() const
		{
			return _elem[0];
		}
		inline void set(unsigned r, unsigned c, T val)
		{
			if (r<R && c<C)
				_elem[r][c] = val;
		}
		inline void get(unsigned r, unsigned c, T &val) const
		{
			if (r<R && c<C)
				val = _elem[r][c];
		}
		inline T operator () (unsigned r, unsigned c) const
		{
			return _elem[r][c];
		}
		inline T& operator () (unsigned r, unsigned c)
		{
			return _elem[r][c];
		}
		template<int D>
		inline void set_row(unsigned r, const xvector<T, D> &v)
		{
			assert(r<R);
			memcpy(_elem[r], v._elem, sizeof(T)*(C<D ? C : D));
		}
		template<int D>
		void set_col(unsigned c, const xvector<T, D> &v)
		{
			assert(c<C);
			for (int i = 0; i<(R<D ? R : D); ++i)
				_elem[i][c] = v._elem[i];
		}
		inline void get_row(unsigned r, xvector<T, C> &v) const
		{
			assert(r<R);
			memcpy(v._elem, _elem[r], sizeof(T)*C);
		}
		void get_col(unsigned c, xvector<T, R> &v) const
		{
			assert(c<C);
			for (int i = 0; i<R; ++i)
				v[i] = _elem[i][c];
		}

		inline xmatrix& operator += (const xmatrix &m)
		{
			return this->add(m);
		}
		inline xmatrix& operator -= (const xmatrix &m)
		{
			return this->sub(m);
		}
		inline xmatrix& operator *= (T val)
		{
			return this->scale(val);
		}
		inline xmatrix& operator /= (T val)
		{
			return this->div(val);
		}
		inline xmatrix& operator *= (const xmatrix<T, C, R> &m)
		{
			return this->mul(m);
		}

		xmatrix& add(const xmatrix &m)
		{
			T *dst = _elem[0];
			const T *src = m._elem[0];
			for (int i = R*C; i>0; --i)
			{
				*dst += *src;
				++dst;
				++src;
			}
			return *this;
		}
		xmatrix& add(const xmatrix &m1, const xmatrix &m2)
		{
			T *dst = _elem[0];
			const T *src1 = m1._elem[0], *src2 = m2._elem[0];
			for (int i = R*C; i>0; --i)
			{
				*dst = *src1 + *src2;
				++dst;
				++src1;
				++src2;
			}
			return *this;
		}
		xmatrix& sub(const xmatrix &m)
		{
			T *dst = _elem[0];
			const T *src = m._elem[0];
			for (int i = R*C; i>0; --i)
			{
				*dst -= *src;
				++dst;
				++src;
			}
			return *this;
		}
		xmatrix& sub(const xmatrix &m1, const xmatrix &m2)
		{
			T *dst = _elem[0];
			const T *src1 = m1._elem[0], *src2 = m2._elem[0];
			for (int i = R*C; i>0; --i)
			{
				*dst = *src1 - *src2;
				++dst;
				++src1;
				++src2;
			}
			return *this;
		}
		xmatrix& scale(T val)
		{
			T *dst = _elem[0];
			for (int i = R*C; i>0; --i)
			{
				*dst *= val;
				++dst;
			}
			return *this;
		}
		xmatrix& scale(const xmatrix &m, T val)
		{
			T *dst = _elem[0];
			const T *src = m._elem[0];
			for (int i = R*C; i>0; --i)
			{
				*dst = *src*val;
				++dst;
				++src;
			}
			return *this;
		}
		xmatrix& div(T val)
		{
			T *dst = _elem[0];
			for (int i = R*C; i>0; --i)
			{
				*dst /= val;
				++dst;
			}
			return *this;
		}
		xmatrix& div(const xmatrix &m, T val)
		{
			T *dst = _elem[0];
			const T *src = m._elem[0];
			for (int i = R*C; i>0; --i)
			{
				*dst = *src / val;
				++dst;
				++src;
			}
			return *this;
		}
		xmatrix& mul(const xmatrix<T, C, R> &m)
		{
			T tmp[C], *dst;
			int i, j, k;
			for (i = 0; i<R; ++i)
			{
				dst = _elem[i];
				for (j = 0; j<C; ++j)
				{
					tmp[j] = 0;
					for (k = 0; k<C; ++k)
						tmp[j] += dst[k] * m._elem[k][j];
				}
				memcpy(dst, tmp, sizeof(T)*C);
			}
			return *this;
		}
		template<int I>
		xmatrix& mul(const xmatrix<T, R, I> &m1, const xmatrix<T, I, C> &m2)
		{
			T sum, *dst;
			const T *src;
			for (int i = 0; i<R; ++i)
			{
				dst = _elem[i];
				src = m1._elem[i];
				for (int j = 0; j<C; ++j)
				{
					sum = 0;
					for (int k = 0; k<I; ++k)
						sum += src[k] * m2(k, j);
					dst[j] = sum;
				}
			}
			return *this;
		}
		xmatrix& transpose()
		{
			T *dst;
			for (int i = 0; i<R; ++i)
			{
				dst = _elem[i];
				for (int j = i + 1; j<C; ++j)
					SWAP(dst[j], _elem[j][i]);
			}
			return *this;
		}
		xmatrix& transpose(const xmatrix<T, C, R> &m)
		{
			T *dst;
			for (int i = 0; i<R; ++i)
			{
				dst = _elem[i];
				for (int j = 0; j<C; ++j)
					dst[j] = m._elem[j][i];
			}
			return *this;
		}
		// Laplace algorithm
		template<int D>
		T _recursive_det() const {
			xmatrix<T, D - 1, D - 1> sub;
			T v, r = 0;
			for (unsigned i = 0; i<D; ++i)
			{
				_get_sub_matrix(sub, 0, i);
				v = (i % 2) ? (-_elem[0][i]) : (_elem[0][i]);
				v *= sub.det();
				r += v;
			}
			return r;

		}
		template<> T _recursive_det<1>() const {
			return 	_elem[0][0];
		}
		template<> T _recursive_det<2>() const {
			return (_elem[0][0] * _elem[1][1] - _elem[0][1] * _elem[1][0]);
		}
		template<> T _recursive_det<3>() const {
			return (
				_elem[0][0] * _elem[1][1] * _elem[2][2] +
				_elem[0][1] * _elem[1][2] * _elem[2][0] +
				_elem[0][2] * _elem[1][0] * _elem[2][1] -
				_elem[2][0] * _elem[1][1] * _elem[0][2] -
				_elem[2][1] * _elem[1][2] * _elem[0][0] -
				_elem[2][2] * _elem[1][0] * _elem[0][1]
				);
		}
		T det() const
		{
			assert(square());
			return _recursive_det<R>();
		}
		// Gauss-Jordan消元法计算m的逆阵
		// 结果保存在this中，如果成功返回true，否则返回false
		// 只有方阵才能调用该函数
		bool inverse(const xmatrix &m)
		{
			assert(square());
			*this = m;
			int i, j, k;
			int row[R], col[C];
			T det = 1;
			T *src, *dst;
			for (k = 0; k<R; ++k)
			{
				T max = 0;
				// 寻找主元
				for (i = k; i<R; ++i)
				{
					src = _elem[i];
					for (j = k; j<C; ++j)
					{
						if (abs(src[j])>max)
						{
							max = src[j];
							row[k] = i;
							col[k] = j;
						}
					}
				}
				// 如果主元为0，逆阵不存在
				if (fequal(max, 0))
					return false;
				if (row[k] != k)
				{
					// 行交换
					det = -det;
					src = _elem[row[k]];
					dst = _elem[k];
					for (j = 0; j<C; ++j)
						SWAP(src[j], dst[j]);
				}
				if (col[k] != k)
				{
					// 列交换
					det = -det;
					for (j = col[k], i = 0; i<R; ++i)
						SWAP(_elem[i][j], _elem[i][k]);
				}
				T mkk = _elem[k][k];
				// 计算行列式
				det *= mkk;
				// 计算逆阵元素
				mkk = 1 / mkk;
				_elem[k][k] = mkk;
				src = _elem[k];
				for (j = 0; j<k; ++j)
					src[j] *= mkk;
				for (++j; j<C; ++j)
					src[j] *= mkk;
				for (i = 0; i<R; ++i)
				{
					if (i == k)
						continue;
					dst = _elem[i];
					for (j = 0; j<C; ++j)
						if (j != k)
							dst[j] -= src[j] * dst[k];
				}
				mkk = -mkk;
				for (i = 0; i<R; ++i)
					if (i != k)
						_elem[i][k] *= mkk;
			}
			// 如果之前执行了行/列交换，则需要执行逆交换
			// 交换次序相反，且行（列）交换用列（行）交换代替
			for (k = R - 1; k >= 0; --k)
			{
				if (col[k] != k)
				{
					src = _elem[col[k]];
					dst = _elem[k];
					for (j = 0; j<C; ++j)
						SWAP(src[j], dst[j]);
				}
				if (row[k] != k)
				{
					for (j = row[k], i = 0; i<R; ++i)
						SWAP(_elem[i][j], _elem[i][k]);
				}
			}
			return true;
		}

		bool operator == (const xmatrix &m) const
		{
			const T *iter1 = _elem[0], *iter2 = m._elem[0];
			for (int i = R*C; i>0; --i) {
				if (*iter1 != *iter2)
					return false;
				++iter1;
				++iter2;
			}
			return true;
		}
		bool operator != (const xmatrix &m) const
		{
			const T *iter1 = _elem[0], *iter2 = m._elem[0];
			for (int i = R*C; i>0; --i) {
				if (*iter1 != *iter2)
					return true;
				++iter1;
				++iter2;
			}
			return false;
		}

		template<int R2, int C2>
		xmatrix& copy(const xmatrix<T, R2, C2> &m)
		{
			int r = std::min(R2, R);
			int c = std::min(C2, C);
			for (int i = 0; i<r; ++i)
			{
				for (int j = 0; j<c; ++j)
					_elem[i][j] = m._elem[i][j];
			}
			return *this;
		}
		// 获取去除第rr行rc列后的子阵
		void _get_sub_matrix(xmatrix<T, R - 1, C - 1> &m, int rr, int rc) const
		{
			unsigned i, j, k, l;
			for (i = 0, k = 0; i<R; ++i)
			{
				if (i == rr)
					continue;
				for (j = 0, l = 0; j<C; ++j)
				{
					if (j == rc)
						continue;
					m._elem[k][l] = _elem[i][j];
					++l;
				}
				++k;
			}
		}
	};

	template<class T>
	struct xmatrix<T, 2, 2>
	{
		typedef T element_t;
		enum {
			ROW = 2,
			COL = 2,
		};

		union
		{
			T	_elem[2][2];
			struct
			{
				T
					m00, m01,
					m10, m11;
			};
		};

		xmatrix& operator = (const xmatrix &m)
		{
			m00 = m.m00; m01 = m.m01;
			m10 = m.m10; m11 = m.m11;
			return *this;
		}

		inline bool square() const
		{
			return true;
		}
		inline void identity()
		{
			m00 = m11 = 1;
			m10 = m01 = 0;
		}
		inline T* data()
		{
			return _elem[0];
		}
		inline const T* data() const
		{
			return _elem[0];
		}
		inline void set(unsigned r, unsigned c, T val)
		{
			if (r<2 && c<2)
				_elem[r][c] = val;
		}
		inline void get(unsigned r, unsigned c, T &val) const
		{
			if (r<2 && c<2)
				val = _elem[r][c];
		}
		inline T operator () (unsigned r, unsigned c) const
		{
			assert(r<2 && c<2);
			return _elem[r][c];
		}
		inline T& operator () (unsigned r, unsigned c)
		{
			assert(r<2 && c<2);
			return _elem[r][c];
		}
		template<int D>
		inline void set_row(unsigned r, const xvector<T, D> &v)
		{
			assert(r<2);
			_elem[r][0] = v[0];
			_elem[r][1] = v[1];
		}
		template<int D>
		inline void set_col(unsigned c, const xvector<T, D> &v)
		{
			assert(c<2);
			_elem[0][c] = v[0];
			_elem[1][c] = v[1];
		}
		inline void get_row(unsigned r, xvector<T, 2> &v) const
		{
			assert(r<2);
			v.set(_elem[r][0], _elem[r][1]);
		}
		inline void get_col(unsigned c, xvector<T, 2> &v) const
		{
			assert(c<2);
			v.set(_elem[0][c], _elem[1][c]);
		}

		inline xmatrix& operator += (const xmatrix &m)
		{
			return this->add(m);
		}
		inline xmatrix& operator -= (const xmatrix &m)
		{
			return this->sub(m);
		}
		inline xmatrix& operator *= (T val)
		{
			return this->scale(val);
		}
		inline xmatrix& operator /= (T val)
		{
			return this->div(val);
		}
		inline xmatrix& operator *= (const xmatrix &m)
		{
			return this->mul(m);
		}
		xmatrix& add(const xmatrix &m)
		{
			m00 += m.m00; m01 += m.m01;
			m10 += m.m10; m11 += m.m11;
			return *this;
		}
		xmatrix& add(const xmatrix &m1, const xmatrix &m2)
		{
			m00 = m1.m00 + m2.m00; m01 = m1.m01 + m2.m01;
			m10 = m1.m10 + m2.m10; m11 = m1.m11 + m2.m11;
			return *this;
		}
		xmatrix& sub(const xmatrix &m)
		{
			m00 -= m.m00; m01 -= m.m01;
			m10 -= m.m10; m11 -= m.m11;
			return *this;
		}
		xmatrix& sub(const xmatrix &m1, const xmatrix &m2)
		{
			m00 = m1.m00 - m2.m00; m01 = m1.m01 - m2.m01;
			m10 = m1.m10 - m2.m10; m11 = m1.m11 - m2.m11;
			return *this;
		}
		xmatrix& scale(T val)
		{
			m00 *= val; m01 *= val;
			m10 *= val; m11 *= val;
			return *this;
		}
		xmatrix& scale(const xmatrix &m, T val)
		{
			m00 = m.m00*val; m01 = m.m01*val;
			m10 = m.m10*val; m11 = m.m11*val;
			return *this;
		}
		xmatrix& div(T val)
		{
			m00 /= val; m01 /= val;
			m10 /= val; m11 /= val;
			return *this;
		}
		xmatrix& div(const xmatrix &m, T val)
		{
			m00 = m.m00 / val; m01 = m.m01 / val;
			m10 = m.m10 / val; m11 = m.m11 / val;
			return *this;
		}
		xmatrix& mul(const xmatrix &m)
		{
			T tmp0, tmp1;
			tmp0 = m00*m.m00 + m01*m.m10;
			tmp1 = m00*m.m01 + m01*m.m11;
			m00 = tmp0;
			m01 = tmp1;
			tmp0 = m10*m.m00 + m11*m.m10;
			tmp1 = m10*m.m01 + m11*m.m11;
			m10 = tmp0;
			m11 = tmp1;
			return *this;
		}
		xmatrix& mul(const xmatrix &m1, const xmatrix &m2)
		{
			m00 = m1.m00*m2.m00 + m1.m01*m2.m10;
			m01 = m1.m00*m2.m01 + m1.m01*m2.m11;
			m10 = m1.m10*m2.m00 + m1.m11*m2.m10;
			m11 = m1.m10*m2.m01 + m1.m11*m2.m11;
			return *this;
		}
		inline xmatrix& transpose()
		{
			SWAP(m01, m10);
			return *this;
		}
		xmatrix& transpose(const xmatrix &m)
		{
			m00 = m.m00; m01 = m.m10;
			m10 = m.m01; m11 = m.m11;
			return *this;
		}
		inline T det() const
		{
			return (m00*m11 - m01*m10);
		}
		bool inverse(const xmatrix &m)
		{
			T d = m.det();
			if (fequal(d, 0))
				return false;
			T invdet = T(1) / d;
			m00 = m.m11*invdet; m01 = -m.m01*invdet;
			m10 = -m.m10*invdet; m11 = m.m00*invdet;
			return true;
		}
		bool operator == (const xmatrix &mat) const
		{
			return m00 == mat.m00 && m01 == mat.m01
				&& m10 == mat.m10 && m11 == mat.m11;
		}
		bool operator != (const xmatrix &mat) const
		{
			return m00 != mat.m00 || m01 != mat.m01
				|| m10 != mat.m10 || m11 != mat.m11;
		}
	};


	template<class T>
	struct xmatrix<T, 3, 3>
	{
		typedef T element_t;
		enum {
			ROW = 3,
			COL = 3,
		};

		union
		{
			T	_elem[3][3];
			struct
			{
				T
					m00, m01, m02,
					m10, m11, m12,
					m20, m21, m22;
			};
		};

		xmatrix& operator = (const xmatrix &m)
		{
			memcpy(_elem[0], m._elem[0], sizeof(T) * 9);
			return *this;
		}

		xmatrix& operator = (const xmatrix<T, 2, 2> &m)
		{
			for (int i = 0; i<2; ++i)
			{
				for (int j = 0; j<2; ++j)
					_elem[i][j] = m._elem[i][j];
			}
			_elem[0][2] = 0;
			_elem[1][2] = 0;
			T *row = _elem[2];
			row[0] = 0;
			row[1] = 0;
			row[2] = 1;
			return *this;
		}

		xmatrix& operator = (const xmatrix<T, 4, 4> &m)
		{
			for (int i = 0; i<3; ++i)
			{
				for (int j = 0; j<3; ++j)
					_elem[i][j] = m._elem[i][j];
			}
			return *this;
		}

		inline bool square() const
		{
			return true;
		}
		inline void identity()
		{
			m00 = m11 = m22 = 1;
			m01 = m02 = m10 = m12 = m20 = m21 = 0;
		}
		inline T* data()
		{
			return _elem[0];
		}
		inline const T* data() const
		{
			return _elem[0];
		}
		inline void set(unsigned r, unsigned c, T val)
		{
			if (r<3 && c<3)
				_elem[r][c] = val;
		}
		inline void get(unsigned r, unsigned c, T &val) const
		{
			if (r<3 && c<3)
				val = _elem[r][c];
		}
		inline T operator () (unsigned r, unsigned c) const
		{
			assert(r<3 && c<3);
			return _elem[r][c];
		}
		inline T& operator () (unsigned r, unsigned c)
		{
			assert(r<3 && c<3);
			return _elem[r][c];
		}
		template<int D>
		inline void set_row(unsigned r, const xvector<T, D> &v)
		{
			assert(r<3);
			assert(D>2);
			T *row = _elem[r];
			row[0] = v[0];
			row[1] = v[1];
			row[2] = v[2];
		}
		template<int D>
		inline void set_col(unsigned c, const xvector<T, D> &v)
		{
			assert(c<3);
			assert(D>2);
			_elem[0][c] = v[0];
			_elem[1][c] = v[1];
			_elem[2][c] = v[2];
		}
		inline void set_row(unsigned r, const xvector<T, 2> &v)
		{
			assert(r<3);
			_elem[r][0] = v.x;
			_elem[r][1] = v.y;
		}
		inline void set_col(unsigned c, const xvector<T, 2> &v)
		{
			assert(c<3);
			_elem[0][c] = v.x;
			_elem[1][c] = v.y;
		}
		inline void get_row(unsigned r, xvector<T, 3> &v) const
		{
			assert(r<3);
			const T *row = _elem[r];
			v.set(row[0], row[1], row[2]);
		}
		inline void get_col(unsigned c, xvector<T, 3> &v) const
		{
			assert(c<3);
			v.set(_elem[0][c], _elem[1][c], _elem[2][c]);
		}

		inline xmatrix& operator += (const xmatrix &m)
		{
			return this->add(m);
		}
		inline xmatrix& operator -= (const xmatrix &m)
		{
			return this->sub(m);
		}
		inline xmatrix& operator *= (T val)
		{
			return this->scale(val);
		}
		inline xmatrix& operator /= (T val)
		{
			return this->div(val);
		}
		inline xmatrix& operator *= (const xmatrix &m)
		{
			return this->mul(m);
		}
		xmatrix& add(const xmatrix &m)
		{
			m00 += m.m00; m01 += m.m01; m02 += m.m02;
			m10 += m.m10; m11 += m.m11; m12 += m.m12;
			m20 += m.m20; m21 += m.m21; m22 += m.m22;
			return *this;
		}
		xmatrix& add(const xmatrix &m1, const xmatrix &m2)
		{
			m00 = m1.m00 + m2.m00; m01 = m1.m01 + m2.m01; m02 = m1.m02 + m2.m02;
			m10 = m1.m10 + m2.m10; m11 = m1.m11 + m2.m11; m12 = m1.m12 + m2.m12;
			m20 = m1.m20 + m2.m20; m21 = m1.m21 + m2.m21; m22 = m1.m22 + m2.m22;
			return *this;
		}
		xmatrix& sub(const xmatrix &m)
		{
			m00 -= m.m00; m01 -= m.m01; m02 -= m.m02;
			m10 -= m.m10; m11 -= m.m11; m12 -= m.m12;
			m20 -= m.m20; m21 -= m.m21; m22 -= m.m22;
			return *this;
		}
		xmatrix& sub(const xmatrix &m1, const xmatrix &m2)
		{
			m00 = m1.m00 - m2.m00; m01 = m1.m01 - m2.m01; m02 = m1.m02 - m2.m02;
			m10 = m1.m10 - m2.m10; m11 = m1.m11 - m2.m11; m12 = m1.m12 - m2.m12;
			m20 = m1.m20 - m2.m20; m21 = m1.m21 - m2.m21; m22 = m1.m22 - m2.m22;
			return *this;
		}
		xmatrix& scale(T val)
		{
			m00 *= val; m01 *= val; m02 *= val;
			m10 *= val; m11 *= val; m12 *= val;
			m20 *= val; m21 *= val; m22 *= val;
			return *this;
		}
		xmatrix& scale(const xmatrix &m, T val)
		{
			m00 = m.m00*val; m01 = m.m01*val; m02 = m.m02*val;
			m10 = m.m10*val; m11 = m.m11*val; m12 = m.m12*val;
			m20 = m.m20*val; m21 = m.m21*val; m22 = m.m22*val;
			return *this;
		}
		inline xmatrix& div(T val)
		{
			T inv = T(1) / val;
			return scale(inv);
		}
		inline xmatrix& div(const xmatrix &m, T val)
		{
			T inv = T(1) / val;
			return scale(m, inv);
		}
		xmatrix& mul(const xmatrix &mat)
		{
			T temp[3];
			temp[0] = m00*mat.m00 + m01*mat.m10 + m02*mat.m20;
			temp[1] = m00*mat.m01 + m01*mat.m11 + m02*mat.m21;
			temp[2] = m00*mat.m02 + m01*mat.m12 + m02*mat.m22;
			m00 = temp[0];
			m01 = temp[1];
			m02 = temp[2];
			temp[0] = m10*mat.m00 + m11*mat.m10 + m12*mat.m20;
			temp[1] = m10*mat.m01 + m11*mat.m11 + m12*mat.m21;
			temp[2] = m10*mat.m02 + m11*mat.m12 + m12*mat.m22;
			m10 = temp[0];
			m11 = temp[1];
			m12 = temp[2];
			temp[0] = m20*mat.m00 + m21*mat.m10 + m22*mat.m20;
			temp[1] = m20*mat.m01 + m21*mat.m11 + m22*mat.m21;
			temp[2] = m20*mat.m02 + m21*mat.m12 + m22*mat.m22;
			m20 = temp[0];
			m21 = temp[1];
			m22 = temp[2];
			return *this;
		}
		xmatrix& mul(const xmatrix &m1, const xmatrix &m2)
		{
			m00 = m1.m00*m2.m00 + m1.m01*m2.m10 + m1.m02*m2.m20;
			m01 = m1.m00*m2.m01 + m1.m01*m2.m11 + m1.m02*m2.m21;
			m02 = m1.m00*m2.m02 + m1.m01*m2.m12 + m1.m02*m2.m22;
			m10 = m1.m10*m2.m00 + m1.m11*m2.m10 + m1.m12*m2.m20;
			m11 = m1.m10*m2.m01 + m1.m11*m2.m11 + m1.m12*m2.m21;
			m12 = m1.m10*m2.m02 + m1.m11*m2.m12 + m1.m12*m2.m22;
			m20 = m1.m20*m2.m00 + m1.m21*m2.m10 + m1.m22*m2.m20;
			m21 = m1.m20*m2.m01 + m1.m21*m2.m11 + m1.m22*m2.m21;
			m22 = m1.m20*m2.m02 + m1.m21*m2.m12 + m1.m22*m2.m22;
			return *this;
		}
		xmatrix& transpose()
		{
			SWAP(m01, m10);
			SWAP(m02, m20);
			SWAP(m12, m21);
			return *this;
		}
		xmatrix& transpose(const xmatrix &m)
		{
			m00 = m.m00; m01 = m.m10; m02 = m.m20;
			m10 = m.m01; m11 = m.m11; m12 = m.m21;
			m20 = m.m02; m21 = m.m12; m22 = m.m22;
			return *this;
		}
		T det() const
		{
			return (
				m00*m11*m22 +
				m01*m12*m20 +
				m02*m10*m21 -
				m20*m11*m02 -
				m21*m12*m00 -
				m22*m10*m01
				);
		}
		bool inverse(const xmatrix &m)
		{
			T d = det();
			if (fequal(d, 0))
				return false;
			// adjoint matrix
			T adj[9];
			adj[0] = m.m11*m.m22 - m.m12*m.m21;
			adj[3] = m.m12*m.m20 - m.m10*m.m22;
			adj[6] = m.m10*m.m21 - m.m11*m.m20;
			adj[1] = m.m02*m.m21 - m.m01*m.m22;
			adj[4] = m.m00*m.m22 - m.m02*m.m20;
			adj[7] = m.m01*m.m20 - m.m00*m.m21;
			adj[2] = m.m01*m.m12 - m.m02*m.m11;
			adj[5] = m.m02*m.m10 - m.m00*m.m12;
			adj[8] = m.m00*m.m11 - m.m01*m.m10;
			// the inverse matrix
			T *dst = _elem[0];
			T invdet = T(1) / d;
			for (int i = 0; i<9; ++i)
				dst[i] = adj[i] * invdet;
			return true;
		}
		bool operator == (const xmatrix &m) const
		{
			const T *iter1 = _elem[0], *iter2 = m._elem[0];
			for (int i = 9; i>0; --i) {
				if (*iter1 != *iter2)
					return false;
				++iter1;
				++iter2;
			}
			return true;
		}
		bool operator != (const xmatrix &m) const
		{
			const T *iter1 = _elem[0], *iter2 = m._elem[0];
			for (int i = 9; i>0; --i) {
				if (*iter1 != *iter2)
					return true;
				++iter1;
				++iter2;
			}
			return false;
		}

	};


	template<class T>
	struct xmatrix<T, 4, 4>
	{
		typedef T element_t;
		enum {
			ROW = 4,
			COL = 4,
		};

		union
		{
			T	_elem[4][4];
			struct
			{
				T
					m00, m01, m02, m03,
					m10, m11, m12, m13,
					m20, m21, m22, m23,
					m30, m31, m32, m33;
			};
		};

		xmatrix& operator = (const xmatrix &m)
		{
			memcpy(_elem[0], m._elem[0], sizeof(T) * 16);
			return *this;
		}
		inline bool square() const
		{
			return true;
		}
		inline void identity()
		{
			memset(_elem[0], 0, sizeof(_elem));
			m00 = m11 = m22 = m33 = 1;
		}
		inline T* data()
		{
			return _elem[0];
		}
		inline const T* data() const
		{
			return _elem[0];
		}
		inline void set(unsigned r, unsigned c, T val)
		{
			if (r<4 && c<4)
				_elem[r][c] = val;
		}
		inline void get(unsigned r, unsigned c, T &val) const
		{
			if (r<4 && c<4)
				val = _elem[r][c];
		}
		inline T operator () (unsigned r, unsigned c) const
		{
			assert(r<4 && c<4);
			return _elem[r][c];
		}
		inline T& operator () (unsigned r, unsigned c)
		{
			assert(r<4 && c<4);
			return _elem[r][c];
		}
		template<int D>
		inline void set_row(unsigned r, const xvector<T, D> &v)
		{
			assert(r<4);
			assert(D>3);
			T *row = _elem[r];
			row[0] = v[0];
			row[1] = v[1];
			row[2] = v[2];
			row[3] = v[3];
		}
		template<int D>
		inline void set_col(unsigned c, const xvector<T, D> &v)
		{
			assert(c<4);
			assert(D>3);
			_elem[0][c] = v[0];
			_elem[1][c] = v[1];
			_elem[2][c] = v[2];
			_elem[3][c] = v[3];
		}
		inline void set_row(unsigned r, const xvector<T, 2> &v)
		{
			assert(r<4);
			_elem[r][0] = v.x;
			_elem[r][1] = v.y;
		}
		inline void set_col(unsigned c, const xvector<T, 2> &v)
		{
			assert(c<4);
			_elem[0][c] = v.x;
			_elem[1][c] = v.y;
		}
		inline void set_row(unsigned r, const xvector<T, 3> &v)
		{
			assert(r<4);
			T *row = _elem[r];
			row[0] = v.x;
			row[1] = v.y;
			row[2] = v.z;
		}
		inline void set_col(unsigned c, const xvector<T, 3> &v)
		{
			assert(c<4);
			_elem[0][c] = v.x;
			_elem[1][c] = v.y;
			_elem[2][c] = v.z;
		}
		inline void get_row(unsigned r, xvector<T, 4> &v) const
		{
			assert(r<4);
			const T *row = _elem[r];
			v.set(row[0], row[1], row[2], row[3]);
		}
		inline void get_col(unsigned c, xvector<T, 4> &v) const
		{
			assert(c<4);
			v.set(_elem[0][c], _elem[1][c], _elem[2][c], _elem[3][c]);
		}

		inline xmatrix& operator += (const xmatrix &m)
		{
			return this->add(m);
		}
		inline xmatrix& operator -= (const xmatrix &m)
		{
			return this->sub(m);
		}
		inline xmatrix& operator *= (T val)
		{
			return this->scale(val);
		}
		inline xmatrix& operator /= (T val)
		{
			return this->div(val);
		}
		inline xmatrix& operator *= (const xmatrix &m)
		{
			return this->mul(m);
		}
		xmatrix& add(const xmatrix &m)
		{
			m00 += m.m00; m01 += m.m01; m02 += m.m02; m03 += m.m03;
			m10 += m.m10; m11 += m.m11; m12 += m.m12; m13 += m.m13;
			m20 += m.m20; m21 += m.m21; m22 += m.m22; m23 += m.m23;
			m30 += m.m30; m31 += m.m31; m32 += m.m32; m33 += m.m33;
			return *this;
		}
		xmatrix& add(const xmatrix &m1, const xmatrix &m2)
		{
			m00 = m1.m00 + m2.m00; m01 = m1.m01 + m2.m01; m02 = m1.m02 + m2.m02; m03 = m1.m03 + m2.m03;
			m10 = m1.m10 + m2.m10; m11 = m1.m11 + m2.m11; m12 = m1.m12 + m2.m12; m13 = m1.m13 + m2.m13;
			m20 = m1.m20 + m2.m20; m21 = m1.m21 + m2.m21; m22 = m1.m22 + m2.m22; m23 = m1.m23 + m2.m23;
			m30 = m1.m30 + m2.m30; m31 = m1.m31 + m2.m31; m32 = m1.m32 + m2.m32; m33 = m1.m33 + m2.m33;
			return *this;
		}
		xmatrix& sub(const xmatrix &m)
		{
			m00 -= m.m00; m01 -= m.m01; m02 -= m.m02; m03 -= m.m03;
			m10 -= m.m10; m11 -= m.m11; m12 -= m.m12; m13 -= m.m13;
			m20 -= m.m20; m21 -= m.m21; m22 -= m.m22; m23 -= m.m23;
			m30 -= m.m30; m31 -= m.m31; m32 -= m.m32; m33 -= m.m33;
			return *this;
		}
		xmatrix& sub(const xmatrix &m1, const xmatrix &m2)
		{
			m00 = m1.m00 - m2.m00; m01 = m1.m01 - m2.m01; m02 = m1.m02 - m2.m02; m03 = m1.m03 - m2.m03;
			m10 = m1.m10 - m2.m10; m11 = m1.m11 - m2.m11; m12 = m1.m12 - m2.m12; m13 = m1.m13 - m2.m13;
			m20 = m1.m20 - m2.m20; m21 = m1.m21 - m2.m21; m22 = m1.m22 - m2.m22; m23 = m1.m23 - m2.m23;
			m30 = m1.m30 - m2.m30; m31 = m1.m31 - m2.m31; m32 = m1.m32 - m2.m32; m33 = m1.m33 - m2.m33;
			return *this;
		}
		xmatrix& scale(T val)
		{
			m00 *= val; m01 *= val; m02 *= val; m03 *= val;
			m10 *= val; m11 *= val; m12 *= val; m13 *= val;
			m20 *= val; m21 *= val; m22 *= val; m23 *= val;
			m30 *= val; m31 *= val; m32 *= val; m33 *= val;
			return *this;
		}
		xmatrix& scale(const xmatrix &m, T val)
		{
			m00 = m.m00*val; m01 = m.m01*val; m02 = m.m02*val; m03 = m.m03*val;
			m10 = m.m10*val; m11 = m.m11*val; m12 = m.m12*val; m13 = m.m13*val;
			m20 = m.m20*val; m21 = m.m21*val; m22 = m.m22*val; m23 = m.m23*val;
			m30 = m.m30*val; m31 = m.m31*val; m32 = m.m32*val; m33 = m.m33*val;
			return *this;
		}
		inline xmatrix& div(T val)
		{
			T inv = T(1) / val;
			return scale(inv);
		}
		inline xmatrix& div(const xmatrix &m, T val)
		{
			T inv = T(1) / val;
			return scale(m, inv);
		}
		xmatrix& mul(const xmatrix &mat)
		{
			T temp[4];
			temp[0] = m00*mat.m00 + m01*mat.m10 + m02*mat.m20 + m03*mat.m30;
			temp[1] = m00*mat.m01 + m01*mat.m11 + m02*mat.m21 + m03*mat.m31;
			temp[2] = m00*mat.m02 + m01*mat.m12 + m02*mat.m22 + m03*mat.m32;
			temp[3] = m00*mat.m03 + m01*mat.m13 + m02*mat.m23 + m03*mat.m33;
			m00 = temp[0];
			m01 = temp[1];
			m02 = temp[2];
			m03 = temp[3];
			temp[0] = m10*mat.m00 + m11*mat.m10 + m12*mat.m20 + m13*mat.m30;
			temp[1] = m10*mat.m01 + m11*mat.m11 + m12*mat.m21 + m13*mat.m31;
			temp[2] = m10*mat.m02 + m11*mat.m12 + m12*mat.m22 + m13*mat.m32;
			temp[3] = m10*mat.m03 + m11*mat.m13 + m12*mat.m23 + m13*mat.m33;
			m10 = temp[0];
			m11 = temp[1];
			m12 = temp[2];
			m13 = temp[3];
			temp[0] = m20*mat.m00 + m21*mat.m10 + m22*mat.m20 + m23*mat.m30;
			temp[1] = m20*mat.m01 + m21*mat.m11 + m22*mat.m21 + m23*mat.m31;
			temp[2] = m20*mat.m02 + m21*mat.m12 + m22*mat.m22 + m23*mat.m32;
			temp[3] = m20*mat.m03 + m21*mat.m13 + m22*mat.m23 + m23*mat.m33;
			m20 = temp[0];
			m21 = temp[1];
			m22 = temp[2];
			m23 = temp[3];
			temp[0] = m30*mat.m00 + m31*mat.m10 + m32*mat.m20 + m33*mat.m30;
			temp[1] = m30*mat.m01 + m31*mat.m11 + m32*mat.m21 + m33*mat.m31;
			temp[2] = m30*mat.m02 + m31*mat.m12 + m32*mat.m22 + m33*mat.m32;
			temp[3] = m30*mat.m03 + m31*mat.m13 + m32*mat.m23 + m33*mat.m33;
			m30 = temp[0];
			m31 = temp[1];
			m32 = temp[2];
			m33 = temp[3];
			return *this;
		}
		xmatrix& mul(const xmatrix &m1, const xmatrix &m2)
		{
			m00 = m1.m00*m2.m00 + m1.m01*m2.m10 + m1.m02*m2.m20 + m1.m03*m2.m30;
			m01 = m1.m00*m2.m01 + m1.m01*m2.m11 + m1.m02*m2.m21 + m1.m03*m2.m31;
			m02 = m1.m00*m2.m02 + m1.m01*m2.m12 + m1.m02*m2.m22 + m1.m03*m2.m32;
			m03 = m1.m00*m2.m03 + m1.m01*m2.m13 + m1.m02*m2.m23 + m1.m03*m2.m33;
			m10 = m1.m10*m2.m00 + m1.m11*m2.m10 + m1.m12*m2.m20 + m1.m13*m2.m30;
			m11 = m1.m10*m2.m01 + m1.m11*m2.m11 + m1.m12*m2.m21 + m1.m13*m2.m31;
			m12 = m1.m10*m2.m02 + m1.m11*m2.m12 + m1.m12*m2.m22 + m1.m13*m2.m32;
			m13 = m1.m10*m2.m03 + m1.m11*m2.m13 + m1.m12*m2.m23 + m1.m13*m2.m33;
			m20 = m1.m20*m2.m00 + m1.m21*m2.m10 + m1.m22*m2.m20 + m1.m23*m2.m30;
			m21 = m1.m20*m2.m01 + m1.m21*m2.m11 + m1.m22*m2.m21 + m1.m23*m2.m31;
			m22 = m1.m20*m2.m02 + m1.m21*m2.m12 + m1.m22*m2.m22 + m1.m23*m2.m32;
			m23 = m1.m20*m2.m03 + m1.m21*m2.m13 + m1.m22*m2.m23 + m1.m23*m2.m33;
			m30 = m1.m30*m2.m00 + m1.m31*m2.m10 + m1.m32*m2.m20 + m1.m33*m2.m30;
			m31 = m1.m30*m2.m01 + m1.m31*m2.m11 + m1.m32*m2.m21 + m1.m33*m2.m31;
			m32 = m1.m30*m2.m02 + m1.m31*m2.m12 + m1.m32*m2.m22 + m1.m33*m2.m32;
			m33 = m1.m30*m2.m03 + m1.m31*m2.m13 + m1.m32*m2.m23 + m1.m33*m2.m33;
			return *this;
		}
		xmatrix& transpose()
		{
			SWAP(m01, m10);
			SWAP(m02, m20);
			SWAP(m03, m30);
			SWAP(m12, m21);
			SWAP(m13, m31);
			SWAP(m23, m32);
			return *this;
		}
		xmatrix& transpose(const xmatrix &m)
		{
			m00 = m.m00; m01 = m.m10; m02 = m.m20; m03 = m.m30;
			m10 = m.m01; m11 = m.m11; m12 = m.m21; m13 = m.m31;
			m20 = m.m02; m21 = m.m12; m22 = m.m22; m23 = m.m32;
			m30 = m.m03; m31 = m.m13; m32 = m.m23; m33 = m.m33;
			return *this;
		}
		T det() const
		{
			xmatrix<T, 3, 3> sub;
			T v, r = 0;
			const T *row = _elem[0];
			for (unsigned i = 0; i<4; ++i)
			{
				_get_sub_matrix(sub, i);
				v = (i & 1) ? (-row[i]) : (row[i]);
				v *= sub.det();
				r += v;
			}
			return r;
		}
#ifdef FAST_MATRIX4_INVERSE
		// Strassen's Method
		// 速度很快，但舍入误差会被累积，导致结果精度降低
		bool inverse(const xmatrix &m)
		{
			// r0=inverse(a00)
			double d = m.m00*m.m11 - m.m01*m.m10;
			if (fequal(d, 0))
				return false;
			d = double(1) / d;
			double r0[4] = {
				d*m.m11, -d*m.m01,
				-d*m.m10, d*m.m00,
			};
			// r1=a10*r0
			double r1[4] = {
				m.m20*r0[0] + m.m21*r0[2], m.m20*r0[1] + m.m21*r0[3],
				m.m30*r0[0] + m.m31*r0[2], m.m30*r0[1] + m.m31*r0[3],
			};
			// r2=r0*a01
			double r2[4] = {
				m.m02*r0[0] + m.m12*r0[1], m.m03*r0[0] + m.m13*r0[1],
				m.m02*r0[2] + m.m12*r0[3], m.m03*r0[2] + m.m13*r0[3],
			};
			// r3=a10*r2
			double r3[4] = {
				m.m20*r2[0] + m.m21*r2[2], m.m20*r2[1] + m.m21*r2[3],
				m.m30*r2[0] + m.m31*r2[2], m.m30*r2[1] + m.m31*r2[3],
			};
			// r4=r3-a11
			r3[0] -= m.m22, r3[1] -= m.m23,
				r3[2] -= m.m32, r3[3] -= m.m33,
				// r5=inverse(r4)
				d = r3[0] * r3[3] - r3[1] * r3[2];
			if (fequal(d, 0))
				return false;
			d = 1 / d;
			double tmp = r3[0];
			r3[0] = r3[3] * d;
			r3[1] *= -d;
			r3[2] *= -d;
			r3[3] = tmp*d;
			// c01=r2*r5
			m02 = T(r2[0] * r3[0] + r2[1] * r3[2]); m03 = T(r2[0] * r3[1] + r2[1] * r3[3]);
			m12 = T(r2[2] * r3[0] + r2[3] * r3[2]); m13 = T(r2[2] * r3[1] + r2[3] * r3[3]);
			// c10=r5*r1
			m20 = T(r3[0] * r1[0] + r3[1] * r1[2]); m21 = T(r3[0] * r1[1] + r3[1] * r1[3]);
			m30 = T(r3[2] * r1[0] + r3[3] * r1[2]); m31 = T(r3[2] * r1[1] + r3[3] * r1[3]);
			// c11=-r5
			m22 = T(-r3[0]); m23 = T(-r3[1]);
			m32 = T(-r3[2]); m33 = T(-r3[3]);
			// r6=r2*c10
			r3[0] = r2[0] * m20 + r2[1] * m30; r3[1] = r2[0] * m21 + r2[1] * m31;
			r3[2] = r2[2] * m20 + r2[3] * m30; r3[3] = r2[2] * m21 + r2[3] * m31;
			// c00=r0-r6
			m00 = T(r0[0] - r3[0]); m01 = T(r0[1] - r3[1]);
			m10 = T(r0[2] - r3[2]); m11 = T(r0[3] - r3[3]);
			return true;
		}
#else
		bool inverse(const xmatrix &m)
		{
			T adj[16], *iter;
			double d = 0;
			iter = adj;
			xmatrix<T, 3, 3> sub;
			const T *src = m._elem[0];
			int i, j;
			// 计算行列式
			for (i = 0; i<4; ++i)
			{
				m._get_sub_matrix(sub, i);
				*iter = sub.det();
				if (i & 1)
					*iter = -*iter;
				d += src[i] * (*iter);
				++iter;
			}
			if (fequal(d, 0))
				return false;
			// 计算伴随矩阵
			for (i = 1; i<4; ++i) {
				for (j = 0; j<4; ++j) {
					m._get_sub_matrix(sub, i, j);
					*iter = sub.det();
					if ((i + j) & 1)
						*iter = -*iter;
					++iter;
				}
			}
			// 计算逆阵
			iter = adj;
			double invdet = 1 / d;
			for (i = 0; i<4; ++i) {
				for (j = 0; j<4; ++j)
					_elem[j][i] = T((*iter++)*invdet);
			}
			return true;
		}
#endif // FAST_MATRIX4_INVERSE
		bool operator == (const xmatrix &m) const
		{
			const T *iter1 = _elem[0], *iter2 = m._elem[0];
			for (int i = 16; i>0; --i) {
				if (*iter1 != *iter2)
					return false;
				++iter1;
				++iter2;
			}
			return true;
		}
		bool operator != (const xmatrix &m) const
		{
			const T *iter1 = _elem[0], *iter2 = m._elem[0];
			for (int i = 16; i>0; --i) {
				if (*iter1 != *iter2)
					return true;
				++iter1;
				++iter2;
			}
			return false;
		}
		// 获取去除0行rc列后的子阵
		void _get_sub_matrix(xmatrix<T, 3, 3> &m, int rc) const
		{
			T *dst = m._elem[0];
			const T *src = _elem[1];;
			unsigned i, j;
			for (i = 1; i<4; ++i)
			{
				for (j = 0; j<4; ++j)
				{
					if (j != rc)
						*dst++ = *src;
					++src;
				}
			}
		}
		// 获取去除第rr行rc列后的子阵
		void _get_sub_matrix(xmatrix<T, 3, 3> &m, int rr, int rc) const
		{
			T *dst = m._elem[0];
			const T *src;
			unsigned i, j;
			for (i = 0; i<4; ++i)
			{
				if (i == rr)
					continue;
				src = _elem[i];
				for (j = 0; j<4; ++j)
				{
					if (j == rc)
						continue;
					*dst++ = src[j];
				}
			}
		}
	};

	template<typename T, int R, int C>
	inline xmatrix<T, R, C> operator + (const xmatrix<T, R, C> &m1, const xmatrix<T, R, C> &m2)
	{
		xmatrix<T, R, C> ret;
		ret.add(m1, m2);
		return ret;
	}

	template<typename T, int R, int C>
	inline xmatrix<T, R, C> operator - (const xmatrix<T, R, C> &m1, const xmatrix<T, R, C> &m2)
	{
		xmatrix<T, R, C> ret;
		ret.sub(m1, m2);
		return ret;
	}

	template<typename T, int R, int C>
	inline xmatrix<T, R, C> operator * (const xmatrix<T, R, C> &m1, const xmatrix<T, R, C> &m2)
	{
		xmatrix<T, R, C> ret;
		ret.mul(m1, m2);
		return ret;
	}

	template<typename T, int R, int C>
	inline xmatrix<T, R, C> operator * (const xmatrix<T, R, C> &m1, T val)
	{
		xmatrix<T, R, C> ret;
		ret.scale(m1, val);
		return ret;
	}

	template<typename T, int R, int C>
	inline xmatrix<T, R, C> operator * (T val, const xmatrix<T, R, C> &m1)
	{
		xmatrix<T, R, C> ret;
		ret.scale(m1, val);
		return ret;
	}

	template<typename T, int R, int C>
	inline xmatrix<T, R, C> operator / (const xmatrix<T, R, C> &m1, T val)
	{
		xmatrix<T, R, C> ret;
		ret.div(m1, val);
		return ret;
	}

	template<typename T>
	void matrix_test()
	{
		typedef typename T::element_t elem_t;
		T mat;
		mat.identity();
	}

} // endof namespace wyc

#endif // WYC_HEADER_MATRIX