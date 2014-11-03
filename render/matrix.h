#ifndef WYC_HEADER_MATRIX
#define WYC_HEADER_MATRIX

#include <numeric>
#include "mathfwd.h"

namespace wyc
{

#define SWAP std::swap

	template<class T>
	inline bool almost_zero(T v)
	{
		return std::fabs(v) < std::numeric_limits<T>::epsilon() * 2;
	}

// implement arithmetic operators
// T: vector type
// S: vector element type
#define matrix_operator_helper(T, S) \
	friend inline T operator + (const T &lhs, const T &rhs)\
	{\
		T r; r.add(lhs, rhs); return r;\
	}\
	friend inline T operator - (const T &lhs, const T &rhs)\
	{\
		T r; r.sub(lhs, rhs); return r;\
	}\
	friend inline T operator * (const T &lhs, const T &rhs)\
	{\
		T r; r.mul(lhs, rhs); return r;\
	}\
	friend inline T operator * (const T &lhs, S scalar)\
	{\
		T r; r.scale(lhs, scalar); return r;\
	}\
	friend inline T operator / (const T &lhs, S scalar)\
	{\
		T r; r.div(lhs, scalar); return r;\
	}\
	friend inline T operator * (S scalar, const T &rhs)\
	{\
		T r; r.scale(rhs, scalar); return r;\
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
	}

	template<class T, int R, int C>
	struct xmatrix
	{
		typedef T scalar_t;
		enum {
			ROW = R,
			COL = C,
		};

		T	_m[R][C]; // row first

		xmatrix& operator = (std::initializer_list<T> li)
		{
			size_t cnt = li.size();
			if (cnt > R * C)
				cnt = R * C;
			const T *src = li.begin();
			T *dst = &_m[0][0];
			for (size_t i = 0; i < cnt; ++i)
				dst[i] = src[i];
			return *this;
		}

		inline bool square() const
		{
			return (R == C);
		}

		void identity()
		{
			if (!square())
				return;
			memset(&(_m[0][0]), 0, sizeof(T)*R*C);
			for (int i = 0; i<R; ++i)
				_m[i][i] = 1;
		}

		inline T* data()
		{
			return _m[0];
		}
		inline const T* data() const
		{
			return _m[0];
		}

		inline T* operator [] (unsigned r)
		{
			assert(r < R);
			return _m[r];
		}
		inline const T* operator [] (unsigned r) const
		{
			assert(r < R);
			return _m[r];
		}

		inline xvector<T, C> row(unsigned r) const
		{
			assert(r<R);
			xvector<T, C> v;
			memcpy(&v, _m[r], sizeof(v));
			return v;
		}
		xvector<T, R> col(unsigned c) const
		{
			assert(c<C);
			xvector<T, R> v;
			for (int i = 0; i<R; ++i)
				v[i] = _m[i][c];
			return v;
		}
		inline void set_row(unsigned r, const xvector<T, C> &v)
		{
			assert(r<R);
			memcpy(_m[r], &v, sizeof(v));
		}
		void set_col(unsigned c, const xvector<T, R> &v)
		{
			assert(c<C);
			for (int i = 0; i<R; ++i)
				_m[i][c] = v[i];
		}

		inline xmatrix& operator += (const xmatrix &m)
		{
			T *dst = _m[0];
			const T *src = m._m[0];
			for (int i = R*C; i>0; --i)
			{
				*dst += *src;
				++dst;
				++src;
			}
			return *this;
		}
		inline xmatrix& operator -= (const xmatrix &m)
		{
			T *dst = _m[0];
			const T *src = m._m[0];
			for (int i = R*C; i>0; --i)
			{
				*dst -= *src;
				++dst;
				++src;
			}
			return *this;
		}
		inline xmatrix& operator *= (const xmatrix<T, C, R> &m)
		{
			T tmp[C], *dst;
			int i, j, k;
			for (i = 0; i<R; ++i)
			{
				dst = _m[i];
				for (j = 0; j<C; ++j)
				{
					tmp[j] = 0;
					for (k = 0; k<C; ++k)
						tmp[j] += dst[k] * m._m[k][j];
				}
				memcpy(dst, tmp, sizeof(T)*C);
			}
			return *this;
		}

		inline xmatrix& operator += (T val)
		{
			T *dst = _m[0];
			for (int i = R*C; i>0; --i)
			{
				*dst += val;
				++dst;
			}
			return *this;
		}
		inline xmatrix& operator -= (T val)
		{
			T *dst = _m[0];
			for (int i = R*C; i>0; --i)
			{
				*dst -= val;
				++dst;
			}
			return *this;
		}
		inline xmatrix& operator *= (T val)
		{
			T *dst = _m[0];
			for (int i = R*C; i>0; --i)
			{
				*dst *= val;
				++dst;
			}
			return *this;
		}
		inline xmatrix& operator /= (T val)
		{
			T *dst = _m[0];
			for (int i = R*C; i>0; --i)
			{
				*dst /= val;
				++dst;
			}
			return *this;
		}

		xmatrix& add(const xmatrix &m1, const xmatrix &m2)
		{
			T *dst = _m[0];
			const T *src1 = m1._m[0], *src2 = m2._m[0];
			for (int i = R*C; i>0; --i)
			{
				*dst = *src1 + *src2;
				++dst;
				++src1;
				++src2;
			}
			return *this;
		}
		xmatrix& sub(const xmatrix &m1, const xmatrix &m2)
		{
			T *dst = _m[0];
			const T *src1 = m1._m[0], *src2 = m2._m[0];
			for (int i = R*C; i>0; --i)
			{
				*dst = *src1 - *src2;
				++dst;
				++src1;
				++src2;
			}
			return *this;
		}
		xmatrix& scale(const xmatrix &m, T val)
		{
			T *dst = _m[0];
			const T *src = m._m[0];
			for (int i = R*C; i>0; --i)
			{
				*dst = *src*val;
				++dst;
				++src;
			}
			return *this;
		}
		xmatrix& div(const xmatrix &m, T val)
		{
			T *dst = _m[0];
			const T *src = m._m[0];
			for (int i = R*C; i>0; --i)
			{
				*dst = *src / val;
				++dst;
				++src;
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
				dst = _m[i];
				src = m1._m[i];
				for (int j = 0; j<C; ++j)
				{
					sum = 0;
					for (int k = 0; k<I; ++k)
						sum += src[k] * m2[k][j];
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
				dst = _m[i];
				for (int j = i + 1; j<C; ++j)
					SWAP(dst[j], _m[j][i]);
			}
			return *this;
		}
		xmatrix& transpose_of(const xmatrix<T, C, R> &m)
		{
			if (&m == this)
				return this->transpose();
			T *dst;
			for (int i = 0; i<R; ++i)
			{
				dst = _m[i];
				for (int j = 0; j<C; ++j)
					dst[j] = m._m[j][i];
			}
			return *this;
		}

		// get minor(r,c) sub matrix
		void minor_matrix(xmatrix<T, R - 1, C - 1> &m, int r, int c) const
		{
			unsigned i, j, k, l;
			for (i = 0, k = 0; i<R; ++i)
			{
				if (i == r)
					continue;
				for (j = 0, l = 0; j<C; ++j)
				{
					if (j == c)
						continue;
					m._m[k][l] = _m[i][j];
					++l;
				}
				++k;
			}
		}
		// Laplace algorithm
		template<int D>
		T _recur_det() const {
			xmatrix<T, D - 1, D - 1> sub;
			T v, r = 0;
			for (unsigned i = 0; i<D; ++i)
			{
				minor_matrix(sub, 0, i);
				v = (i % 2) ? (-_m[0][i]) : (_m[0][i]);
				v *= sub.determinant();
				r += v;
			}
			return r;

		}
		template<> T _recur_det<1>() const {
			return 	_m[0][0];
		}
		template<> T _recur_det<2>() const {
			return (_m[0][0] * _m[1][1] - _m[0][1] * _m[1][0]);
		}
		template<> T _recur_det<3>() const {
			return (
				_m[0][0] * _m[1][1] * _m[2][2] +
				_m[0][1] * _m[1][2] * _m[2][0] +
				_m[0][2] * _m[1][0] * _m[2][1] -
				_m[2][0] * _m[1][1] * _m[0][2] -
				_m[2][1] * _m[1][2] * _m[0][0] -
				_m[2][2] * _m[1][0] * _m[0][1]
				);
		}
		T determinant() const
		{
			if(square())
				return _recur_det<R>();
			return 0;
		}
		inline bool inverse()
		{
			xmatrix tmp;
			if (!tmp.inverse_of(*this))
				return false;
			*this = tmp;
			return true;
		}
		// Gauss-Jordan elimination
		// return: true if success, else false
		bool inverse_of(const xmatrix &m)
		{
			if (!square())
				return false;
			*this = m;
			int i, j, k;
			int row[R], col[C];
			T det = 1;
			T *src, *dst;
			for (k = 0; k<R; ++k)
			{
				T max = 0;
				// find the pivot
				for (i = k; i<R; ++i)
				{
					src = _m[i];
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
				if (almost_zero(max))
					return false;
				if (row[k] != k)
				{
					// row switching
					det = -det;
					src = _m[row[k]];
					dst = _m[k];
					for (j = 0; j<C; ++j)
						SWAP(src[j], dst[j]);
				}
				if (col[k] != k)
				{
					// col switching
					det = -det;
					for (j = col[k], i = 0; i<R; ++i)
						SWAP(_m[i][j], _m[i][k]);
				}
				T mkk = _m[k][k];
				// here's the determinant
				det *= mkk;
				mkk = 1 / mkk;
				_m[k][k] = mkk;
				src = _m[k];
				for (j = 0; j<k; ++j)
					src[j] *= mkk;
				for (++j; j<C; ++j)
					src[j] *= mkk;
				for (i = 0; i<R; ++i)
				{
					if (i == k)
						continue;
					dst = _m[i];
					for (j = 0; j<C; ++j)
						if (j != k)
							dst[j] -= src[j] * dst[k];
				}
				mkk = -mkk;
				for (i = 0; i<R; ++i)
					if (i != k)
						_m[i][k] *= mkk;
			}
			// backward substitution
			for (k = R - 1; k >= 0; --k)
			{
				if (col[k] != k)
				{
					src = _m[col[k]];
					dst = _m[k];
					for (j = 0; j<C; ++j)
						SWAP(src[j], dst[j]);
				}
				if (row[k] != k)
				{
					for (j = row[k], i = 0; i<R; ++i)
						SWAP(_m[i][j], _m[i][k]);
				}
			}
			return true;
		}

		bool operator == (const xmatrix &m) const
		{
			const T *iter1 = _m[0], *iter2 = m._m[0];
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
			const T *iter1 = _m[0], *iter2 = m._m[0];
			for (int i = R*C; i>0; --i) {
				if (*iter1 != *iter2)
					return true;
				++iter1;
				++iter2;
			}
			return false;
		}
		bool operator < (const xmatrix &m) const
		{
			const T *iter1 = _m[0], *iter2 = m._m[0];
			for (int i = R*C; i>0; --i) {
				if (*iter1 >= *iter2)
					return false;
				++iter1;
				++iter2;
			}
			return true;
		}

		matrix_operator_helper(xmatrix, T)
	};

	template<class T>
	struct xmatrix<T, 2, 2>
	{
		typedef T scalar_t;
		enum {
			ROW = 2,
			COL = 2,
		};

		union
		{
			T	_m[2][2];
			struct
			{
				T
					m00, m01,
					m10, m11;
			};
		};

		xmatrix& operator = (std::initializer_list<T> li)
		{
			size_t cnt = li.size();
			if (cnt > 4)
				cnt = 4;
			const T *src = li.begin();
			T *dst = &this->m00;
			for (size_t i = 0; i < cnt; ++i)
				dst[i] = src[i];
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
			return _m[0];
		}
		inline const T* data() const
		{
			return _m[0];
		}

		inline T* operator [] (unsigned r)
		{
			assert(r < 2);
			return _m[r];
		}
		inline const T* operator [] (unsigned r) const
		{
			assert(r < 2);
			return _m[r];
		}

		inline xvector<T, 2> row(unsigned r) const
		{
			assert(r < 2);
			return xvector<T, 2>({ _m[r][0], _m[r][1] });
		}
		inline xvector<T, 2> col(unsigned c) const
		{
			assert(c < 2);
			return xvector<T, 2>({ _m[0][c], _m[1][c] });
		}
		inline void set_row(unsigned r, const xvector<T, 2> &v)
		{
			assert(r < 2);
			_m[r][0] = v[0];
			_m[r][1] = v[1];
		}
		inline void set_col(unsigned c, const xvector<T, 2> &v)
		{
			assert(c < 2);
			_m[0][c] = v[0];
			_m[1][c] = v[1];
		}

		inline xmatrix& operator += (const xmatrix &m)
		{
			m00 += m.m00; m01 += m.m01;
			m10 += m.m10; m11 += m.m11;
			return *this;
		}
		inline xmatrix& operator -= (const xmatrix &m)
		{
			m00 -= m.m00; m01 -= m.m01;
			m10 -= m.m10; m11 -= m.m11;
			return *this;
		}
		inline xmatrix& operator += (T val)
		{
			m00 += val; m01 += val;
			m10 += val; m11 += val;
			return *this;
		}
		inline xmatrix& operator -= (T val)
		{
			m00 -= val; m01 -= val;
			m10 -= val; m11 -= val;
			return *this;
		}
		inline xmatrix& operator *= (T val)
		{
			m00 *= val; m01 *= val;
			m10 *= val; m11 *= val;
			return *this;
		}
		inline xmatrix& operator /= (T val)
		{
			m00 /= val; m01 /= val;
			m10 /= val; m11 /= val;
			return *this;
		}
		inline xmatrix& operator *= (const xmatrix &m)
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

		xmatrix& add(const xmatrix &m1, const xmatrix &m2)
		{
			m00 = m1.m00 + m2.m00; m01 = m1.m01 + m2.m01;
			m10 = m1.m10 + m2.m10; m11 = m1.m11 + m2.m11;
			return *this;
		}
		xmatrix& sub(const xmatrix &m1, const xmatrix &m2)
		{
			m00 = m1.m00 - m2.m00; m01 = m1.m01 - m2.m01;
			m10 = m1.m10 - m2.m10; m11 = m1.m11 - m2.m11;
			return *this;
		}
		xmatrix& scale(const xmatrix &m, T val)
		{
			m00 = m.m00*val; m01 = m.m01*val;
			m10 = m.m10*val; m11 = m.m11*val;
			return *this;
		}
		xmatrix& div(const xmatrix &m, T val)
		{
			m00 = m.m00 / val; m01 = m.m01 / val;
			m10 = m.m10 / val; m11 = m.m11 / val;
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
		xmatrix& transpose_of(const xmatrix &m)
		{
			if (&m == this)
				return this->transpose();
			m00 = m.m00; m01 = m.m10;
			m10 = m.m01; m11 = m.m11;
			return *this;
		}

		inline T determinant() const
		{
			return (m00*m11 - m01*m10);
		}

		bool inverse()
		{
			xmatrix tmp;
			if (!tmp.inverse_of(*this))
				return false;
			*this = tmp;
			return true;
		}

		bool inverse_of(const xmatrix &m)
		{
			T d = m.determinant();
			if (almost_zero(d))
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
		bool operator < (const xmatrix &mat) const
		{
			return m00 < mat.m00 && m01 < mat.m01
				&& m10 < mat.m10 && m11 < mat.m11;
		}

		matrix_operator_helper(xmatrix, T)
	};


	template<class T>
	struct xmatrix<T, 3, 3>
	{
		typedef T scalar_t;
		enum {
			ROW = 3,
			COL = 3,
		};

		union
		{
			T	_m[3][3];
			struct
			{
				T
					m00, m01, m02,
					m10, m11, m12,
					m20, m21, m22;
			};
		};

		xmatrix& operator = (std::initializer_list<T> li)
		{
			size_t cnt = li.size();
			if (cnt > 9)
				cnt = 9;
			const T *src = li.begin();
			T *dst = &this->m00;
			for (size_t i = 0; i < cnt; ++i)
				dst[i] = src[i];
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
			return _m[0];
		}
		inline const T* data() const
		{
			return _m[0];
		}

		inline T* operator [] (unsigned r)
		{
			assert(r < 3);
			return _m[r];
		}
		inline const T* operator [] (unsigned r) const
		{
			assert(r < 3);
			return _m[r];
		}

		inline xvector<T, 3> row(unsigned r) const
		{
			assert(r<3);
			return xvector<T, 3>({ _m[r][0], _m[r][1], _m[r][2] });
		}
		inline xvector<T, 3> col(unsigned c) const
		{
			assert(c<3);
			return xvector<T, 3>({ _m[0][c], _m[1][c], _m[2][c] });
		}
		inline void set_row(unsigned r, const xvector<T, 3> &v)
		{
			assert(r<3);
			T *row = _m[r];
			row[0] = v[0];
			row[1] = v[1];
			row[2] = v[2];
		}
		inline void set_col(unsigned c, const xvector<T, 3> &v)
		{
			assert(c<3);
			_m[0][c] = v[0];
			_m[1][c] = v[1];
			_m[2][c] = v[2];
		}

		inline xmatrix& operator += (const xmatrix &m)
		{
			m00 += m.m00; m01 += m.m01; m02 += m.m02;
			m10 += m.m10; m11 += m.m11; m12 += m.m12;
			m20 += m.m20; m21 += m.m21; m22 += m.m22;
			return *this;
		}
		inline xmatrix& operator -= (const xmatrix &m)
		{
			m00 -= m.m00; m01 -= m.m01; m02 -= m.m02;
			m10 -= m.m10; m11 -= m.m11; m12 -= m.m12;
			m20 -= m.m20; m21 -= m.m21; m22 -= m.m22;
			return *this;
		}
		inline xmatrix& operator += (T val)
		{
			m00 += val; m01 += val; m02 += val;
			m10 += val; m11 += val; m12 += val;
			m20 += val; m21 += val; m22 += val;
			return *this;
		}
		inline xmatrix& operator -= (T val)
		{
			m00 -= val; m01 -= val; m02 -= val;
			m10 -= val; m11 -= val; m12 -= val;
			m20 -= val; m21 -= val; m22 -= val;
			return *this;
		}
		inline xmatrix& operator *= (T val)
		{
			m00 *= val; m01 *= val; m02 *= val;
			m10 *= val; m11 *= val; m12 *= val;
			m20 *= val; m21 *= val; m22 *= val;
			return *this;
		}
		inline xmatrix& operator /= (T val)
		{
			T inv = T(1) / val;
			return *this *= inv;
		}
		inline xmatrix& operator *= (const xmatrix &mat)
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

		xmatrix& add(const xmatrix &m1, const xmatrix &m2)
		{
			m00 = m1.m00 + m2.m00; m01 = m1.m01 + m2.m01; m02 = m1.m02 + m2.m02;
			m10 = m1.m10 + m2.m10; m11 = m1.m11 + m2.m11; m12 = m1.m12 + m2.m12;
			m20 = m1.m20 + m2.m20; m21 = m1.m21 + m2.m21; m22 = m1.m22 + m2.m22;
			return *this;
		}
		xmatrix& sub(const xmatrix &m1, const xmatrix &m2)
		{
			m00 = m1.m00 - m2.m00; m01 = m1.m01 - m2.m01; m02 = m1.m02 - m2.m02;
			m10 = m1.m10 - m2.m10; m11 = m1.m11 - m2.m11; m12 = m1.m12 - m2.m12;
			m20 = m1.m20 - m2.m20; m21 = m1.m21 - m2.m21; m22 = m1.m22 - m2.m22;
			return *this;
		}
		xmatrix& scale(const xmatrix &m, T val)
		{
			m00 = m.m00*val; m01 = m.m01*val; m02 = m.m02*val;
			m10 = m.m10*val; m11 = m.m11*val; m12 = m.m12*val;
			m20 = m.m20*val; m21 = m.m21*val; m22 = m.m22*val;
			return *this;
		}
		inline xmatrix& div(const xmatrix &m, T val)
		{
			T inv = T(1) / val;
			return scale(m, inv);
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
		xmatrix& transpose_of(const xmatrix &m)
		{
			if (&m == this)
				return this->transpose();
			m00 = m.m00; m01 = m.m10; m02 = m.m20;
			m10 = m.m01; m11 = m.m11; m12 = m.m21;
			m20 = m.m02; m21 = m.m12; m22 = m.m22;
			return *this;
		}

		T determinant() const
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
		bool inverse()
		{
			xmatrix tmp;
			if (!tmp.inverse_of(*this))
				return false;
			*this = tmp;
			return true;
		}
		bool inverse_of(const xmatrix &m)
		{
			T d = m.determinant();
			if (almost_zero(d))
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
			T *dst = _m[0];
			T invdet = T(1) / d;
			for (int i = 0; i<9; ++i)
				dst[i] = adj[i] * invdet;
			return true;
		}

		bool operator == (const xmatrix &m) const
		{
			const T *iter1 = _m[0], *iter2 = m._m[0];
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
			const T *iter1 = _m[0], *iter2 = m._m[0];
			for (int i = 9; i>0; --i) {
				if (*iter1 != *iter2)
					return true;
				++iter1;
				++iter2;
			}
			return false;
		}
		bool operator < (const xmatrix &m) const
		{
			const T *iter1 = _m[0], *iter2 = m._m[0];
			for (int i = 9; i>0; --i) {
				if (!(*iter1 < *iter2))
					return false;
				++iter1;
				++iter2;
			}
			return true;
		}

		matrix_operator_helper(xmatrix, T)
	};


	template<class T>
	struct xmatrix<T, 4, 4>
	{
		typedef T scalar_t;
		enum {
			ROW = 4,
			COL = 4,
		};

		union
		{
			T	_m[4][4];
			struct
			{
				T
					m00, m01, m02, m03,
					m10, m11, m12, m13,
					m20, m21, m22, m23,
					m30, m31, m32, m33;
			};
		};

		xmatrix& operator = (std::initializer_list<T> li)
		{
			size_t cnt = li.size();
			if (cnt > 16)
				cnt = 16;
			const T *src = li.begin();
			T *dst = &this->m00;
			for (size_t i = 0; i < cnt; ++i)
				dst[i] = src[i];
			return *this;
		}

		inline bool square() const
		{
			return true;
		}
		inline void identity()
		{
			memset(_m[0], 0, sizeof(_m));
			m00 = m11 = m22 = m33 = 1;
		}

		inline T* data()
		{
			return _m[0];
		}
		inline const T* data() const
		{
			return _m[0];
		}

		inline T* operator [] (unsigned r)
		{
			assert(r < 4);
			return _m[r];
		}
		inline const T* operator [] (unsigned r) const
		{
			assert(r < 4);
			return _m[r];
		}

		inline xvector<T, 4> row(unsigned r) const
		{
			assert(r<4);
			return xvector<T, 4>({ _m[r][0], _m[r][1], _m[r][2], _m[r][3] });
		}
		inline xvector<T, 4> col(unsigned c) const
		{
			assert(c<4);
			return xvector<T, 4>({ _m[0][c], _m[1][c], _m[2][c], _m[3][c] });
		}
		inline void set_row(unsigned r, const xvector<T, 4> &v)
		{
			assert(r<4);
			T *row = _m[r];
			row[0] = v[0];
			row[1] = v[1];
			row[2] = v[2];
			row[3] = v[3];
		}
		inline void set_col(unsigned c, const xvector<T, 4> &v)
		{
			assert(c<4);
			_m[0][c] = v[0];
			_m[1][c] = v[1];
			_m[2][c] = v[2];
			_m[3][c] = v[3];
		}

		inline xmatrix& operator += (const xmatrix &m)
		{
			m00 += m.m00; m01 += m.m01; m02 += m.m02; m03 += m.m03;
			m10 += m.m10; m11 += m.m11; m12 += m.m12; m13 += m.m13;
			m20 += m.m20; m21 += m.m21; m22 += m.m22; m23 += m.m23;
			m30 += m.m30; m31 += m.m31; m32 += m.m32; m33 += m.m33;
			return *this;
		}
		inline xmatrix& operator -= (const xmatrix &m)
		{
			m00 -= m.m00; m01 -= m.m01; m02 -= m.m02; m03 -= m.m03;
			m10 -= m.m10; m11 -= m.m11; m12 -= m.m12; m13 -= m.m13;
			m20 -= m.m20; m21 -= m.m21; m22 -= m.m22; m23 -= m.m23;
			m30 -= m.m30; m31 -= m.m31; m32 -= m.m32; m33 -= m.m33;
			return *this;
		}
		inline xmatrix& operator += (T val)
		{
			m00 += val; m01 += val; m02 += val; m03 += val;
			m10 += val; m11 += val; m12 += val; m13 += val;
			m20 += val; m21 += val; m22 += val; m23 += val;
			m30 += val; m31 += val; m32 += val; m33 += val;
			return *this;
		}
		inline xmatrix& operator -= (T val)
		{
			m00 -= val; m01 -= val; m02 -= val; m03 -= val;
			m10 -= val; m11 -= val; m12 -= val; m13 -= val;
			m20 -= val; m21 -= val; m22 -= val; m23 -= val;
			m30 -= val; m31 -= val; m32 -= val; m33 -= val;
			return *this;
		}
		inline xmatrix& operator *= (T val)
		{
			m00 *= val; m01 *= val; m02 *= val; m03 *= val;
			m10 *= val; m11 *= val; m12 *= val; m13 *= val;
			m20 *= val; m21 *= val; m22 *= val; m23 *= val;
			m30 *= val; m31 *= val; m32 *= val; m33 *= val;
			return *this;
		}
		inline xmatrix& operator /= (T val)
		{
			T inv = T(1) / val;
			*this *= inv;
			return *this;
		}
		inline xmatrix& operator *= (const xmatrix &mat)
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

		xmatrix& add(const xmatrix &m1, const xmatrix &m2)
		{
			m00 = m1.m00 + m2.m00; m01 = m1.m01 + m2.m01; m02 = m1.m02 + m2.m02; m03 = m1.m03 + m2.m03;
			m10 = m1.m10 + m2.m10; m11 = m1.m11 + m2.m11; m12 = m1.m12 + m2.m12; m13 = m1.m13 + m2.m13;
			m20 = m1.m20 + m2.m20; m21 = m1.m21 + m2.m21; m22 = m1.m22 + m2.m22; m23 = m1.m23 + m2.m23;
			m30 = m1.m30 + m2.m30; m31 = m1.m31 + m2.m31; m32 = m1.m32 + m2.m32; m33 = m1.m33 + m2.m33;
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
		xmatrix& scale(const xmatrix &m, T val)
		{
			m00 = m.m00*val; m01 = m.m01*val; m02 = m.m02*val; m03 = m.m03*val;
			m10 = m.m10*val; m11 = m.m11*val; m12 = m.m12*val; m13 = m.m13*val;
			m20 = m.m20*val; m21 = m.m21*val; m22 = m.m22*val; m23 = m.m23*val;
			m30 = m.m30*val; m31 = m.m31*val; m32 = m.m32*val; m33 = m.m33*val;
			return *this;
		}
		inline xmatrix& div(const xmatrix &m, T val)
		{
			T inv = T(1) / val;
			return scale(m, inv);
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
		xmatrix& transpose_of(const xmatrix &m)
		{
			if (&m == this)
				return this->transpose();
			m00 = m.m00; m01 = m.m10; m02 = m.m20; m03 = m.m30;
			m10 = m.m01; m11 = m.m11; m12 = m.m21; m13 = m.m31;
			m20 = m.m02; m21 = m.m12; m22 = m.m22; m23 = m.m32;
			m30 = m.m03; m31 = m.m13; m32 = m.m23; m33 = m.m33;
			return *this;
		}

		// get minor(0,c) sub matrix
		void minor_matrix(xmatrix<T, 3, 3> &m, int c) const
		{
			T *dst = m._m[0];
			const T *src = _m[1];;
			unsigned i, j;
			for (i = 1; i<4; ++i)
			{
				for (j = 0; j<4; ++j)
				{
					if (j != c)
						*dst++ = *src;
					++src;
				}
			}
		}
		// get minor(r,c) sub matrix
		void minor_matrix(xmatrix<T, 3, 3> &m, int r, int c) const
		{
			T *dst = m._m[0];
			const T *src;
			unsigned i, j;
			for (i = 0; i<4; ++i)
			{
				if (i == r)
					continue;
				src = _m[i];
				for (j = 0; j<4; ++j)
				{
					if (j == c)
						continue;
					*dst++ = src[j];
				}
			}
		}
		T determinant() const
		{
			xmatrix<T, 3, 3> sub;
			T v, r = 0;
			const T *row = _m[0];
			for (unsigned i = 0; i<4; ++i)
			{
				minor_matrix(sub, i);
				v = (i & 1) ? (-row[i]) : (row[i]);
				v *= sub.determinant();
				r += v;
			}
			return r;
		}
		inline bool inverse()
		{
			xmatrix tmp;
			if (!tmp.inverse_of(*this))
				return false;
			*this = tmp;
			return true;
		}
#ifdef FAST_MATRIX4_INVERSE
		// Strassen's Method
		// faster than GJ, but lower accuracy (because of the accumulated rounding error)
		bool inverse_of(const xmatrix &m)
		{
			// r0=inverse(a00)
			double d = m.m00*m.m11 - m.m01*m.m10;
			if (almost_zero(d))
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
			if (almost_zero(d))
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
		bool inverse_of(const xmatrix &m)
		{
			T adj[16], *iter;
			double d = 0;
			iter = adj;
			xmatrix<T, 3, 3> sub;
			const T *src = m._m[0];
			int i, j;
			// the determinant
			for (i = 0; i<4; ++i)
			{
				m.minor_matrix(sub, i);
				*iter = sub.determinant();
				if (i & 1)
					*iter = -*iter;
				d += src[i] * (*iter);
				++iter;
			}
			if (almost_zero(d))
				return false;
			// adjacent matrix
			for (i = 1; i<4; ++i) {
				for (j = 0; j<4; ++j) {
					m.minor_matrix(sub, i, j);
					*iter = sub.determinant();
					if ((i + j) & 1)
						*iter = -*iter;
					++iter;
				}
			}
			// the inverse matrix
			iter = adj;
			double invdet = 1 / d;
			for (i = 0; i<4; ++i) {
				for (j = 0; j<4; ++j)
					_m[j][i] = T((*iter++)*invdet);
			}
			return true;
		}
#endif // FAST_MATRIX4_INVERSE

		bool operator == (const xmatrix &m) const
		{
			const T *iter1 = _m[0], *iter2 = m._m[0];
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
			const T *iter1 = _m[0], *iter2 = m._m[0];
			for (int i = 16; i>0; --i) {
				if (*iter1 != *iter2)
					return true;
				++iter1;
				++iter2;
			}
			return false;
		}
		bool operator < (const xmatrix &m) const
		{
			const T *iter1 = _m[0], *iter2 = m._m[0];
			for (int i = 16; i>0; --i) {
				if (!(*iter1 < *iter2))
					return false;
				++iter1;
				++iter2;
			}
			return true;
		}

		matrix_operator_helper(xmatrix, T)
	};

	//-----------------------------------------------------------------------------------
	// matrix vector multiplication
	//-----------------------------------------------------------------------------------

	template<class T, int R, int C>
	inline xvector<T, C> operator * (const xvector<T, R> &v, const xmatrix<T, R, C> &m)
	{
		xvector<T, C> r;
		T sum;
		for (int j = 0; j < C; ++j)
		{
			sum = 0;
			for (int i = 0; i < R; ++i)
				sum += v[i]*m[i][j];
			r[j] = sum;
		}
		return r;
	}

	template<class T, int R, int C>
	inline xvector<T, C> operator * (const xmatrix<T, R, C> &m, const xvector<T, C> &v)
	{
		xvector<T, C> r;
		T sum;
		for (int i = 0; i < R; ++i)
		{
			sum = 0;
			for (int j = 0; j < C; ++j)
				sum += v[j]*m[i][j];
			r[i] = sum;
		}
		return r;
	}

	template<class T>
	xvector<T, 2> operator * (const xvector<T, 2> &v, const xmatrix<T, 2, 2> &m)
	{
		xvector<T, 2> r;
		r.x = v.x*m.m00 + v.y*m.m10;
		r.y = v.x*m.m01 + v.y*m.m11;
		return r;
	}

	template<class T>
	xvector<T, 2> operator * (const xmatrix<T, 2, 2> &m, const xvector<T, 2> &v)
	{
		xvector<T, 2> r;
		r.x = v.x*m.m00 + v.y*m.m01;
		r.y = v.x*m.m10 + v.y*m.m11;
		return r;
	}

	template<class T>
	xvector<T, 2> operator * (const xvector<T, 2> &v, const xmatrix<T, 3, 3> &m)
	{
		xvector<T, 2> r;
		r.x = v.x*m.m00 + v.y*m.m10;
		r.y = v.x*m.m01 + v.y*m.m11;
		return r;
	}

	template<class T>
	xvector<T, 2> operator * (const xmatrix<T, 3, 3> &m, const xvector<T, 2> &v)
	{
		xvector<T, 2> r;
		r.x = v.x*m.m00 + v.y*m.m01;
		r.y = v.x*m.m10 + v.y*m.m11;
		return r;
	}

	template<class T>
	xvector<T, 3> operator * (const xvector<T, 3> &v, const xmatrix<T, 3, 3> &m)
	{
		xvector<T, 3> r;
		r.x = v.x*m.m00 + v.y*m.m10 + v.z*m.m20;
		r.y = v.x*m.m01 + v.y*m.m11 + v.z*m.m21;
		r.z = v.x*m.m02 + v.y*m.m12 + v.z*m.m22;
		return r;
	}

	template<class T>
	xvector<T, 3> operator * (const xmatrix<T, 3, 3> &m, const xvector<T, 3> &v)
	{
		xvector<T, 3> r;
		r.x = v.x*m.m00 + v.y*m.m01 + v.z*m.m02;
		r.y = v.x*m.m10 + v.y*m.m11 + v.z*m.m12;
		r.z = v.x*m.m20 + v.y*m.m21 + v.z*m.m22;
		return r;
	}

	template<class T>
	xvector<T, 3> operator * (const xvector<T, 3> &v, const xmatrix<T, 4, 4> &m)
	{
		xvector<T, 3> r;
		r.x = v.x*m.m00 + v.y*m.m10 + v.z*m.m20;
		r.y = v.x*m.m01 + v.y*m.m11 + v.z*m.m21;
		r.z = v.x*m.m02 + v.y*m.m12 + v.z*m.m22;
		return r;
	}

	template<class T>
	xvector<T, 3> operator * (const xmatrix<T, 4, 4> &m, const xvector<T, 3> &v)
	{
		xvector<T, 3> r;
		r.x = v.x*m.m00 + v.y*m.m01 + v.z*m.m02;
		r.y = v.x*m.m10 + v.y*m.m11 + v.z*m.m12;
		r.z = v.x*m.m20 + v.y*m.m21 + v.z*m.m22;
		return r;
	}

	template<class T>
	inline xvector<T, 4> operator * (const xvector<T, 4> &v, const xmatrix<T, 4, 4> &m)
	{
		xvector<T, 4> r;
		r.x = v.x*m.m00 + v.y*m.m10 + v.z*m.m20 + v.w*m.m30;
		r.y = v.x*m.m01 + v.y*m.m11 + v.z*m.m21 + v.w*m.m31;
		r.z = v.x*m.m02 + v.y*m.m12 + v.z*m.m22 + v.w*m.m32;
		r.w = v.x*m.m03 + v.y*m.m13 + v.z*m.m23 + v.w*m.m33;
		return r;
	}

	template<class T>
	inline xvector<T, 4> operator * (const xmatrix<T, 4, 4> &m, const xvector<T, 4> &v)
	{
		xvector<T, 4> r;
		r.x = v.x*m.m00 + v.y*m.m01 + v.z*m.m02 + v.w*m.m03;
		r.y = v.x*m.m10 + v.y*m.m11 + v.z*m.m12 + v.w*m.m13;
		r.z = v.x*m.m20 + v.y*m.m21 + v.z*m.m22 + v.w*m.m23;
		r.w = v.x*m.m30 + v.y*m.m31 + v.z*m.m32 + v.w*m.m33;
		return r;
	}

} // endof namespace wyc

#endif // WYC_HEADER_MATRIX