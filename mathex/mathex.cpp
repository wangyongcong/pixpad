#include "mathex.h"

namespace wyc
{

// sin/cos查找表
static float g_xmath_sintab[361];
static float g_xmath_costab[361];

// 构造正/余弦函数值查找表
void build_sincos_table()
{
	// 从0度到360度
	float tmp;
	for(int ang=0;ang<=360;++ang)
	{
		// 转化为弧度
		tmp=DEG_TO_RAD(ang);
		// 使用C数学函数填充查找表
		g_xmath_sintab[ang]=sin(tmp);
		g_xmath_costab[ang]=cos(tmp);
	}
}

// 使用线性插值快速计算sin/cos，传入的参数为角度
float fast_sin(float ang)
{
	ang=fmodf(ang,360);
	if(ang<0)
		ang+=360;
	int alpha=(int)ang;
	float beta=ang-alpha;
	return (g_xmath_sintab[alpha]+beta*(g_xmath_sintab[alpha+1]-g_xmath_sintab[alpha]));
}

float fast_cos(float ang)
{
	ang=fmodf(ang,360);
	if(ang<0)
		ang+=360;
	int alpha=(int)ang;
	float beta=ang-alpha;
	return (g_xmath_costab[alpha]+beta*(g_xmath_costab[alpha+1]-g_xmath_costab[alpha]));
}

// 定位到[0,1]边界
void clamp(float32_t &f)
{
	FLOATBITS tmp;
	tmp.fval=f;
	tmp.ival&=~(tmp.ival>>31);
	tmp.fval-=1.0f;
	tmp.ival&=tmp.ival>>31;
	f=tmp.fval+1.0f;
}

// mod到正整数
float32_t mod(float32_t a, float32_t b)
{
	*(int32_t*)(&b)&=0x7FFFFFFF;
	int32_t n=int32_t(a/b);
	a-=n*b;
	if(a<0)
		a+=b;
	return a;
}

// 快速计算x的平方根的倒数，相对误差约为0.177585%
float32_t fast_invsqrt(float32_t x)
{
    float32_t xhalf=0.5f*x;
    int32_t i=*(int*)&x;
    i=0x5f37642f-(i>>1);	// 估计初值
    x=*(float32_t*)&i;
    x=x*(1.5f-xhalf*x*x);	// NR过程
    return x;
}

// 快速计算x的平方根，相对误差约为0.177585%
float32_t fast_sqrt(float32_t x)
{
	FLOATBITS convertor, convertor2;
	convertor.fval = x;
	convertor2.fval = x;
	convertor.ival = 0x1FBCF800 + (convertor.ival >> 1);
	convertor2.ival = 0x5f37642f - (convertor2.ival >> 1);
	return 0.5f*(convertor.fval + (x * convertor2.fval));
}

} // namespace wyc

