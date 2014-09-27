#ifndef __HEADER_WYC_XVECMATH
#define __HEADER_WYC_XVECMATH

#include <cassert>
#include "mathex.h"

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable:4201) // anonymous struct/union
#endif // _MSC_VER

#define FAST_MATRIX4_INVERSE // use fast matrix inverse algorithm

#include "vector.h"
#include "matrix.h"

namespace wyc
{

	template<class T, int R, int C>
	void vec_mul_mat(const xvector<T, R> &v, const xmatrix<T, R, C> &m, xvector<T, C> &r)
	{
		T sum;
		for (int j = 0; j < C; ++j)
		{
			sum = 0;
			for (int i = 0; i < R; ++i)
				sum += v(i)*m(i, j);
			r(j) = sum;
		}
	}

	template<class T, int R, int C>
	void vec_mul_mat(const xmatrix<T, R, C> &m, const xvector<T, C> &v, xvector<T, C> &r)
	{
		T sum;
		for (int i = 0; i < R; ++i)
		{
			sum = 0;
			for (int j = 0; j < C; ++j)
				sum += v(j)*m(i, j);
			r(i) = sum;
		}
	}

	template<class T, int R, int C>
	inline xvector<T, C> operator * (const xvector<T, R> &v, const xmatrix<T, R, C> &m)
	{
		xvector<T, C> ret;
		vec_mul_mat(v, m, ret);
		return ret;
	}

	template<class T, int R, int C>
	inline xvector<T, C> operator * (const xmatrix<T, R, C> &m, const xvector<T, C> &v)
	{
		xvector<T, C> ret;
		vec_mul_mat(m, v, ret);
		return ret;
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
		r.x = v.x*m(0, 0) + v.y*m(1, 0) + v.z*m(2, 0);
		r.y = v.x*m(0, 1) + v.y*m(1, 1) + v.z*m(2, 1);
		r.z = v.x*m(0, 2) + v.y*m(1, 2) + v.z*m(2, 2);
		return r;
	}

	template<class T>
	xvector<T, 3> operator * (const xmatrix<T, 4, 4> &m, const xvector<T, 3> &v)
	{
		xvector<T, 3> r;
		r.x = v.x*m(0, 0) + v.y*m(0, 1) + v.z*m(0, 2);
		r.y = v.x*m(1, 0) + v.y*m(1, 1) + v.z*m(1, 2);
		r.z = v.x*m(2, 0) + v.y*m(2, 1) + v.z*m(2, 2);
		return r;
	}

	template<class T>
	void vec_mul_mat(const xvector<T, 4> &v, const xmatrix<T, 4, 4> &m, xvector<T, 4> &r)
	{
		r.x = v.x*m(0, 0) + v.y*m(1, 0) + v.z*m(2, 0) + v.w*m(3, 0);
		r.y = v.x*m(0, 1) + v.y*m(1, 1) + v.z*m(2, 1) + v.w*m(3, 1);
		r.z = v.x*m(0, 2) + v.y*m(1, 2) + v.z*m(2, 2) + v.w*m(3, 2);
		r.w = v.x*m(0, 3) + v.y*m(1, 3) + v.z*m(2, 3) + v.w*m(3, 3);
	}

	template<class T>
	void vec_mul_mat(const xmatrix<T, 4, 4> &m, const xvector<T, 4> &v, xvector<T, 4> &r)
	{
		r.x = v.x*m(0, 0) + v.y*m(0, 1) + v.z*m(0, 2) + v.w*m(0, 3);
		r.y = v.x*m(1, 0) + v.y*m(1, 1) + v.z*m(1, 2) + v.w*m(1, 3);
		r.z = v.x*m(2, 0) + v.y*m(2, 1) + v.z*m(2, 2) + v.w*m(2, 3);
		r.w = v.x*m(3, 0) + v.y*m(3, 1) + v.z*m(3, 2) + v.w*m(3, 3);
	}

	template<class T>
	inline xvector<T, 4> operator * (const xvector<T, 4> &v, const xmatrix<T, 4, 4> &m)
	{
		xvector<T, 4> r;
		vec_mul_mat(v, m, r);
		return r;
	}

	template<class T>
	inline xvector<T, 4> operator * (const xmatrix<T, 4, 4> &m, const xvector<T, 4> &v)
	{
		xvector<T, 4> r;
		vec_mul_mat(m, v, r);
		return r;
	}


	template<class T>
	class xquaternion
	{
	public:
		union
		{
			T	elem[4];
			struct
			{
				T	qa;
				xvector<T, 3> qv;
			};
			struct
			{
				T	w, x, y, z;
			};
		};

	public:
		xquaternion() {}

		~xquaternion() {}

		xquaternion(const xquaternion<T> &q)
		{
			*this = q;
		}
		xquaternion& operator = (const xquaternion<T> &q)
		{
			qa = q.qa;
			qv = q.qv;
			return *this;
		}

		// 根据旋转向量和旋转角构建四元数
		xquaternion(const xvector<T, 3> &v, T theta)
		{
			from_vec_ang(v, theta);
		}

		// 与3D向量间的转换
		xquaternion(const xvector<T, 3> &v) :
			qa(0), qv(v)
		{}
		inline xquaternion& operator = (const xvector<T, 3> &v)
		{
			qa = 0;
			qv = v;
			return *this;
		}

	public:
		inline xquaternion& operator += (const xquaternion &q)
		{
			return add(q);
		}
		inline xquaternion& operator -= (const xquaternion &q)
		{
			return sub(q);
		}
		inline xquaternion& add(const xquaternion &q)
		{
			w += q.w;
			x += q.x;
			y += q.y;
			z += q.z;
			return *this;
		}
		inline xquaternion& add(const xquaternion &q1, const xquaternion &q2)
		{
			w = q1.w + q2.w;
			x = q1.x + q2.x;
			y = q1.y + q2.y;
			z = q1.z + q2.z;
			return *this;
		}
		inline xquaternion& sub(const xquaternion &q)
		{
			w -= q.w;
			x -= q.x;
			y -= q.y;
			z -= q.z;
			return *this;
		}
		inline xquaternion& sub(const xquaternion &q1, const xquaternion &q2)
		{
			w = q1.w - q2.w;
			x = q1.x - q2.x;
			y = q1.y - q2.y;
			z = q1.z - q2.z;
			return *this;
		}
		inline xquaternion& scale(T val)
		{
			w *= val;
			x *= val;
			y *= val;
			z *= val;
			return *this;
		}
		inline T dot(const xquaternion &q)
		{
			return (w*q.w + x*q.x + y*q.y + z*q.z);
		}
		// this=q1*q2
		xquaternion& mul(const xquaternion &q1, const xquaternion &q2)
		{
			//
			// 公式：[ w1*w2 - (v1 dot v2), w1*v2 + w2*v1 + (v1 cross v2) ]
			//
			// 直接计算，16次乘法，12次加法
			/*
			w = q1.w*q2.w - q1.x*q2.x - q1.y*q2.y - q1.z*q2.z;
			x = q1.w*q2.x + q1.x*q2.w + q1.y*q2.z - q1.z*q2.y;
			y = q1.w*q2.y - q1.x*q2.z + q1.y*q2.w + q1.z*q2.x;
			z = q1.w*q2.z + q1.x*q2.y - q1.y*q2.x + q1.z*q2.w;
			*/

			// 通过提取公共因子的改进算法，9次乘法，27次加法

			T a0 = (q1.z - q1.y) * (q2.y - q2.z);
			T a1 = (q1.w + q1.x) * (q2.w + q2.x);
			T a2 = (q1.w - q1.x) * (q2.y + q2.z);
			T a3 = (q1.y + q1.z) * (q2.w - q2.x);
			T a4 = (q1.z - q1.x) * (q2.x - q2.y);
			T a5 = (q1.z + q1.x) * (q2.x + q2.y);
			T a6 = (q1.w + q1.y) * (q2.w - q2.z);
			T a7 = (q1.w - q1.y) * (q2.w + q2.z);

			T a8 = a5 + a6 + a7;
			T a9 = T(0.5 * (a4 + a8));

			w = a0 + a9 - a5;
			x = a1 + a9 - a8;
			y = a2 + a9 - a7;
			z = a3 + a9 - a6;

			return *this;
		}
		// 求q1到q2的角度变换，结果保存在this中
		xquaternion& differ(const xquaternion &q1, const xquaternion &q2)
		{
			xquaternion tmp = q1;
			tmp.inverse();
			mul(tmp, q2);
			return *this;
		}
		inline T magnitude() const
		{
			return (sqrt(magnitude2()));
		}
		inline T magnitude2() const
		{
			return (w*w + x*x + y*y + z*z);
		}
		inline xquaternion& conjugate()
		{
			x = -x;
			y = -y;
			z = -z;
			return *this;
		}
		xquaternion& inverse()
		{
			T inv = magnitude();
			inv = 1 / inv;
			conjugate().scale(inv);
			return *this;
		}
		inline xquaternion& unit_inverse()
		{
			conjugate();
		}
		// log(this) logarithm
		xquaternion& log(const xquaternion &q)
		{
			T a = acos(q.w);
			w = 0;
			x = y = z = a;
			return *this;
		}
		// e^this exponent
		xquaternion& exp(const xquaternion &q)
		{
			T a = q.x;
			w = cos(a);
			x = y = z = sin(a);
			return *this;
		}
		// this^(t) power
		inline xquaternion& pow(T t)
		{
			return (log(*this).scale(t).exp(*this));
		}
		// 根据转轴和转角构造四元数
		// 参数normalized指出传入的向量是否单位向量
		void from_vec_ang(const xvector<T, 3> &v, T angle, bool normalized = true)
		{
			angle /= 2;
			T sina;
			if (!normalized)
			{
				// 如果不是单位向量，先要将其规整为单位向量
				T len = v.length();
				len = 1 / len;
				sina = sin(angle) * len;
			}
			else
			{
				sina = sin(angle);
			}
			w = cos(angle);
			x = v.x * sina;
			y = v.y * sina;
			z = v.z * sina;
		}
		// 从四元数中提取转轴和转角
		void to_vec_ang(xvector<T, 3> &v, T &angle) const
		{
			angle = acos(w);
			T inv_sina = sin(angle);
			if (fequal(inv_sina, T(0)))
			{
				v.x = v.y = v.z = 0;
			}
			else
			{
				inv_sina = 1 / inv_sina;
				v.x = x * inv_sina;
				v.y = y * inv_sina;
				v.z = z * inv_sina;
			}
			angle *= 2;
		}
	};

	template<class T>
	inline xquaternion<T> operator + (const xquaternion<T> &q1, const xquaternion<T> &q2)
	{
		xquaternion q;
		return q.add(q1, q2);
	}

	template<class T>
	inline xquaternion<T> operator - (const xquaternion<T> &q1, const xquaternion<T> &q2)
	{
		xquaternion q;
		return q.sub(q1, q2);
	}

	template<class T>
	inline xquaternion<T> operator * (const xquaternion<T> &q1, const xquaternion<T> &q2)
	{
		xquaternion q;
		return q.mul(q1, q2);
	}

	template<class T>
	inline xquaternion<T> operator / (const xquaternion<T> &q1, const xquaternion<T> &q2)
	{
		xquaternion q;
		return q.differ(q1, q2);
	}


	// 2D极坐标
	template<class T>
	struct xpolar
	{
		T	r;			// 半径
		T	theta;		// 角度
	};

	// 3D柱面坐标
	template<class T>
	struct xcylin
	{
		T	r;			// 半径
		T	theta;		// 角度
		T	z;			// Z坐标
	};


	// 3D球坐标
	template<class T>
	struct xspherical
	{
		T	r;			// 半径
		union {
			// 经度（向量在X-Y面上的投影与正X轴间的夹角）
			T	theta;
			T	longitude;
		};
		union {
			// 纬度（向量与正Z轴的夹角）
			T	phi;
			T	latitude;
		};
	};

	// 2D笛卡尔坐标与极坐标之间的转换
	template<class T>
	inline void to_cartesian(const xpolar<T> &p, xvector<T, 2> &v)
	{
		v.x = p.r * cos(p.theta);
		v.y = p.r * sin(p.theta);
	}

	template<class T>
	inline void to_polar(const xvector<T, 2> &v, xpolar<T> &p)
	{
		p.r = v.length();
		p.theta = atan2(v.y, v.x);
	}

	// 3D笛卡尔坐标与柱面坐标之间的转换
	template<class T>
	inline void to_cartesian(const xcylin<T> &c, xvector<T, 3> &v)
	{
		v.x = c.r * cos(c.theta);
		v.y = c.r * sin(c.theta);
		v.z = c.z;
	}

	template<class T>
	inline void to_cylin(const xvector<T, 3> &v, xcylin<T> &c)
	{
		c.r = sqrt(v.x*v.x + v.y*v.y);
		c.theta = atan2(v.y, v.x);
		c.z = v.z;
	}

	// 3D笛卡尔坐标与球坐标之间的转换
	template<class T>
	void to_cartesian(const xspherical<T> &s, xvector<T, 3> &v)
	{
		T tmp = s.r * sin(s.phi);
		v.x = tmp * cos(s.theta);
		v.y = tmp * sin(s.theta);
		v.z = s.r * cos(s.phi);
	}

	template<class T>
	void to_spherical(const  xvector<T, 3> &v, xspherical<T> &s)
	{
		s.r = v.length();
		s.theta = atan2(v.y, v.x);
		s.phi = asin(v.x / (s.r*cos(s.theta)));
	}

	// 欧拉角，默认使用roll-pitch-yaw（Z-X-Y）的旋转顺序
	template<class T>
	struct xeuler
	{
		union
		{
			T elem[3];
			struct
			{
				T x, y, z;
			};
			struct
			{
				T pitch, yaw, roll;
			};
		};
	};

	// 欧拉角和旋转矩阵之间的转换
	template<class T, int D>
	void euler_to_matrix(const xeuler<T> &euler, xmatrix<T, D, D> &mat)
	{
		T sinr = sin(euler.roll);
		T sinp = sin(euler.pitch);
		T siny = sin(euler.yaw);

		T cosr = cos(euler.roll);
		T cosp = cos(euler.pitch);
		T cosy = cos(euler.yaw);

		T cosy_cosr = cosy * cosr;
		T cosy_sinr = cosy * sinr;
		T siny_cosr = siny * cosr;
		T siny_sinr = siny * sinr;

		mat(0, 0) = cosy_cosr + siny_sinr * sinp;
		mat(0, 1) = sinr * cosp;
		mat(0, 2) = cosy_sinr * sinp - siny_cosr;
		mat(1, 0) = siny_cosr * sinp - cosy_sinr;
		mat(1, 1) = cosr * cosp;
		mat(1, 2) = siny_sinr + cosy_cosr * sinp;
		mat(2, 0) = siny * cosp;
		mat(2, 1) = -sinp;
		mat(2, 2) = cosy * cosp;
	}

	template<class T, int D>
	void matrix_to_euler(const xmatrix<T, D, D> &mat, xeuler<T> &euler)
	{
		T M21 = mat(2, 1);
		if (M21 >= 1.0)
			euler.pitch = PI_DIV_2;
		else if (M21 <= -1.0)
			euler.pitch = -PI_DIV_2;
		else
			euler.pitch = asin(-mat(2, 1));

		if (M21 > 0.99999)
		{
			// gimbal lock
			euler.roll = 0;
			euler.yaw = asin(-mat(0, 2));
		}
		else
		{
			euler.roll = atan2(mat(0, 1), mat(1, 1));
			euler.yaw = atan2(mat(2, 0), mat(2, 2));
		}
	}

	// 四元数和旋转矩阵之间的转换
	template<class T, int D>
	void quat_to_matrix(const xquaternion<T> &quat, xmatrix<T, D, D> &mat)
	{
		// 使用临时变量来减少乘法
		T xx = quat.x * quat.x;
		T yy = quat.y * quat.y;
		T zz = quat.z * quat.z;
		T ww = quat.w * quat.w;

		T xy = quat.x * quat.y;
		T xz = quat.x * quat.z;
		T xw = quat.x * quat.w;
		T yz = quat.y * quat.z;
		T yw = quat.y * quat.w;
		T zw = quat.z * quat.w;

		mat(0, 0) = 1 - 2 * (yy + zz);
		mat(1, 1) = 1 - 2 * (xx + zz);
		mat(2, 2) = 1 - 2 * (xx + yy);

		mat(0, 1) = 2 * (xy + zw);
		mat(0, 2) = 2 * (xz - yw);
		mat(1, 0) = 2 * (xy - zw);
		mat(1, 2) = 2 * (yz + xw);
		mat(2, 0) = 2 * (yw + xz);
		mat(2, 1) = 2 * (yz - xw);
	}

	template<class T, int D>
	void matrix_to_quat(const xmatrix<T, D, D> &mat, xquaternion<T> &quat)
	{
		// 确定最大的分量
		T max_w = mat(0, 0) + mat(1, 1) + mat(2, 2);
		T max_x = mat(0, 0) - mat(1, 1) - mat(2, 2);
		T max_y = mat(1, 1) - mat(2, 2) - mat(0, 0);
		T max_z = mat(2, 2) - mat(0, 0) - mat(1, 1);
		int id = 0;
		T max = max_w;
		if (max_x > max)
		{
			max = max_x;
			id = 1;
		}
		if (max_y > max)
		{
			max = max_y;
			id = 2;
		}
		if (max_z > max)
		{
			max = max_z;
			id = 3;
		}
		// 计算四元数的值
		max = T(sqrt(max + 1)*0.5);
		T mult = T(0.25 / max);
		switch (id)
		{
		case 0:
			quat.w = max;
			quat.x = (mat(1, 2) - mat(2, 1)) * mult;
			quat.y = (mat(2, 0) - mat(0, 2)) * mult;
			quat.z = (mat(0, 1) - mat(1, 0)) * mult;
			break;
		case 1:
			quat.x = max;
			quat.w = (mat(1, 2) - mat(2, 1)) * mult;
			quat.y = (mat(0, 1) + mat(1, 0)) * mult;
			quat.z = (mat(2, 0) + mat(0, 2)) * mult;
			break;
		case 2:
			quat.y = max;
			quat.w = (mat(2, 0) - mat(0, 2)) * mult;
			quat.x = (mat(0, 1) + mat(1, 0)) * mult;
			quat.z = (mat(1, 2) + mat(2, 1)) * mult;
			break;
		case 3:
			quat.z = max;
			quat.w = (mat(0, 1) - mat(1, 0)) * mult;
			quat.x = (mat(2, 0) + mat(0, 2)) * mult;
			quat.y = (mat(1, 2) + mat(2, 1)) * mult;
			break;
		}
	}

	// 欧拉角和四元数之间的转换
	template<class T>
	void euler_to_quat(const xeuler<T> &euler, xquaternion<T> &quat)
	{
		T r = T(euler.roll * 0.5);
		T p = T(euler.pitch * 0.5);
		T y = T(euler.yaw * 0.5);

		float cosr = cos(r);
		float cosp = cos(p);
		float cosy = cos(y);

		float sinr = sin(r);
		float sinp = sin(p);
		float siny = sin(y);

		quat.w = cosy*cosp*cosr + siny*sinp*sinr;
		quat.x = cosy*sinp*cosr + siny*cosp*sinr;
		quat.y = siny*cosp*cosr - cosy*sinp*sinr;
		quat.z = cosy*cosp*cosr - siny*sinp*cosr;
	}

	template<class T>
	void quat_to_euler(const xquaternion<T> &quat, xeuler<T> &euler)
	{
		T tmp = -2 * (quat.y*quat.z - quat.w*quat.x);
		if (abs(tmp) > 0.99999f)
		{
			// gimbal lock
			euler.pitch = PI_DIV_2*tmp;
			euler.yaw = atan2(T(-quat.x*quat.z + quat.w*quat.y), T(-quat.y*quat.y - quat.z*quat.z));
			euler.roll = 0;
		}
		else
		{
			T xx = quat.x * quat.x;
			euler.pitch = asin(tmp);
			euler.yaw = atan2(T(quat.x*quat.z + quat.w*quat.y), T(0.5 - xx - quat.y*quat.y));
			euler.roll = atan2(T(quat.x*quat.y + quat.w*quat.z), T(0.5 - xx - quat.z*quat.z));
		}
	}

	// 2D平移矩阵
	template<class T, int D>
	inline void matrix_translate2d(xmatrix<T, D, D> &matrix, T dx, T dy)
	{
		matrix.identity();
		matrix(2, 0) = dx;
		matrix(2, 1) = dy;
	}

	// 2D缩放矩阵
	template<class T, int D>
	inline void matrix_scale2d(xmatrix<T, D, D> &matrix, T sx, T sy)
	{
		matrix.identity();
		matrix(0, 0) = sx;
		matrix(1, 1) = sy;
	}

	// 2D旋转矩阵,逆时针旋转radian弧度
	template<class T, int D>
	void matrix_rotate2d(xmatrix<T, D, D> &matrix, T radian)
	{
		matrix.identity();
		matrix(0, 0) = cos(radian);  matrix(0, 1) = sin(radian);
		matrix(1, 0) = -sin(radian); matrix(1, 1) = cos(radian);
	}

	// 3D平移矩阵
	template<class T, int D>
	inline void matrix_translate3d(xmatrix<T, D, D> &matrix, T dx, T dy, T dz)
	{
		matrix.identity();
		matrix(3, 0) = dx; matrix(3, 1) = dy; matrix(3, 2) = dz;
	}

	// 3D缩放矩阵
	template<class T, int D>
	inline void matrix_scale3d(xmatrix<T, D, D> &matrix, T sx, T sy, T sz)
	{
		matrix.identity();
		matrix(0, 0) = sx; matrix(1, 1) = sy; matrix(2, 2) = sz;
	}

	// 3D旋转矩阵,绕轴n(单位向量)逆时针旋转radian弧度
	template<class T, int D>
	void matrix_rotate3d(xmatrix<T, D, D> &matrix, const xvector<T, 3> &n, T radian)
	{
		matrix.identity();
		T sina = sin(radian);
		T cosa = cos(radian);
		T t = 1 - cosa;
		matrix(0, 0) = n.x*n.x*t + cosa; matrix(0, 1) = n.x*n.y*t + n.z*sina; matrix(0, 2) = n.x*n.z*t - n.y*sina;
		matrix(1, 0) = n.x*n.y*t - n.z*sina; matrix(1, 1) = n.y*n.y*t + cosa; matrix(1, 2) = n.z*n.y*t + n.x*sina;
		matrix(2, 0) = n.x*n.z*t + n.y*sina; matrix(2, 1) = n.y*n.z*t - n.x*sina; matrix(2, 2) = n.z*n.z*t + cosa;
	}

	// 3D旋转矩阵,绕x轴逆时针旋转radian弧度
	template<class T, int D>
	void matrix_xrotate3d(xmatrix<T, D, D> &matrix, T radian)
	{
		matrix.identity();
		T sina = sin(radian);
		T cosa = cos(radian);
		matrix(1, 1) = cosa; matrix(1, 2) = sina;
		matrix(2, 1) = -sina; matrix(2, 2) = cosa;
	}

	// 3D旋转矩阵,绕y轴逆时针旋转radian弧度
	template<class T, int D>
	void matrix_yrotate3d(xmatrix<T, D, D> &matrix, T radian)
	{
		matrix.identity();
		T sina = sin(radian);
		T cosa = cos(radian);
		matrix(0, 0) = cosa; matrix(0, 2) = -sina;
		matrix(2, 0) = sina; matrix(2, 2) = cosa;
	}

	// 3D旋转矩阵,绕z轴逆时针旋转radian弧度
	template<class T, int D>
	void matrix_zrotate3d(xmatrix<T, D, D> &matrix, T radian)
	{
		matrix.identity();
		T sina = sin(radian);
		T cosa = cos(radian);
		matrix(0, 0) = cosa; matrix(0, 1) = sina;
		matrix(1, 0) = -sina; matrix(1, 1) = cosa;
	}


	typedef xvector<int, 2> xpt2i_t;
	typedef xvector<float, 2> xpt2f_t;
	typedef xvector<double, 2> xpt2d_t;
	typedef xvector<int, 3> xpt3i_t;
	typedef xvector<float, 3> xpt3f_t;
	typedef xvector<double, 3> xpt3d_t;
	typedef xvector<int, 4> xpt4i_t;
	typedef xvector<float, 4> xpt4f_t;
	typedef xvector<double, 4> xpt4d_t;

	typedef xvector<int, 2> xvec2i_t;
	typedef xvector<float, 2> xvec2f_t;
	typedef xvector<double, 2> xvec2d_t;
	typedef xvector<int, 3> xvec3i_t;
	typedef xvector<float, 3> xvec3f_t;
	typedef xvector<double, 3> xvec3d_t;
	typedef xvector<int, 4> xvec4i_t;
	typedef xvector<float, 4> xvec4f_t;
	typedef xvector<double, 4> xvec4d_t;

	typedef xmatrix<int, 2, 2> xmat2i_t;
	typedef xmatrix<float, 2, 2> xmat2f_t;
	typedef xmatrix<double, 2, 2>	xmat2d_t;
	typedef xmatrix<int, 3, 3> xmat3i_t;
	typedef xmatrix<float, 3, 3> xmat3f_t;
	typedef xmatrix<double, 3, 3>	xmat3d_t;
	typedef xmatrix<int, 4, 4> xmat4i_t;
	typedef xmatrix<float, 4, 4> xmat4f_t;
	typedef xmatrix<double, 4, 4>	xmat4d_t;

	typedef xquaternion<int> xquati_t;
	typedef xquaternion<float> xquatf_t;
	typedef xquaternion<double>	xquatd_t;

	typedef xeuler<int> xeuleri_t;
	typedef xeuler<float> xeulerf_t;
	typedef xeuler<double> xeulerd_t;

	// OpenGL orthograph matrix
	void set_orthograph(xmat4f_t &mat, float xmin, float ymin, float zmin, float xmax, float ymax, float zmax);

	// OpenGL perspective matrix
	// aspect is width/height
	void set_perspective(xmat4f_t &proj, float yfov, float aspect, float fnear, float ffar);

	// orthograph matrix for GUI
	void set_ui_projection(xmat4f_t &proj, float screen_width, float screen_height, float z_range);


} // namespace wyc

#ifdef _MSC_VER
#pragma warning (pop)
#endif // _MSC_VER

#endif // end of __HEADER_WYC_XVECMATH
